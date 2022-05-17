#include "freq_bp.hpp"

#include <iostream>
#include <cstring>

#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/tnode_types/tnode_location.hpp"
#include "../common/logger.hpp"

namespace query_ns {

void FreqBpQuery::add_alters(const std::vector<uint64_t> bp_alterations, uint64_t database_id, const std::string &owner, const ds::DB *db) {
    uint64_t last_version_id = 0;
    for (const uint64_t alteration : bp_alterations) {
        uint64_t altered_bp = (alteration >> common::BITS_FOR_STEPS_BACK);
        uint64_t num_versions_back = alteration - (altered_bp << common::BITS_FOR_STEPS_BACK);
        if (num_versions_back > last_version_id) {
            last_version_id = num_versions_back;
        }
    }

    for (const uint64_t alteration : bp_alterations) {
        uint64_t altered_bp = (alteration >> common::BITS_FOR_STEPS_BACK);
        uint64_t num_versions_back = alteration - (altered_bp << common::BITS_FOR_STEPS_BACK);
        alterations_per_bp[altered_bp]++;
        owner_distrib_per_bp[altered_bp][owner]++;
        if (db == NULL) {
            continue;
        }

        uint64_t curr_mask = (((uint64_t)database_id)<<common::BITS_FOR_STEPS_BACK) + num_versions_back;
        if (prv_sequences.count(curr_mask) == 0) {
            uint64_t target_database_idx = database_id;
            while (num_versions_back) {
                target_database_idx = db->get_element(target_database_idx).prv_db_id;
                assert(target_database_idx != ULLONG_MAX);
                --num_versions_back;
            }
            uint64_t prev_database_idx = db->get_element(target_database_idx).prv_db_id;
            assert(prev_database_idx != ULLONG_MAX);

            prv_sequences[curr_mask] = std::make_pair(db->get_element(prev_database_idx).covv_data[common::SEQ_FIELDS_TO_ID.at("sequence")], 
                                                      db->get_element(target_database_idx).covv_data[common::SEQ_FIELDS_TO_ID.at("sequence")]);
        }
        char prv_char = prv_sequences[curr_mask].first[altered_bp];
        char act_char = prv_sequences[curr_mask].second[altered_bp];

        std::string char_to_char;
        char_to_char.push_back(prv_char);
        char_to_char = char_to_char + ">";
        char_to_char.push_back(act_char);

        char_to_char_distrib_per_bp_per_owner[altered_bp][owner][char_to_char]++;

        std::string final_char;
        final_char.push_back(act_char);
        if (alteration - (altered_bp << common::BITS_FOR_STEPS_BACK) == last_version_id) {
            // if the latest version, increment the map for the final state of that bp.
            char_to_char_final_result_per_bp_per_owner[altered_bp][owner][final_char]++;
        }
    }
}

FreqBpQuery::FreqBpQuery(const bool req_compute_total_owner_cnt, const uint64_t req_num_to_print) {
    this->compute_total_owner_cnt = req_compute_total_owner_cnt;
    this->num_to_print = req_num_to_print;
    reset();
}

void FreqBpQuery::reset() {
    memset(alterations_per_bp, 0, sizeof alterations_per_bp);    
}

void FreqBpQuery::set_deletion_mode(const bool is_deletion_mode) {
    found_first = false;
    before_lca_tnode = true;
    deletions_mode = is_deletion_mode;
}

std::string FreqBpQuery::get_treap_name() {
    // todo make a field in constants.hpp.
    return "location_treap";
}

TreeDirectionToGo FreqBpQuery::first_enter_into_node(const std::string &target_location_prefix, Tnode *, const BaseSortedTreap *elem_unique, const ds::DB *db) {
    if (elem_unique->key < target_location_prefix) {
        return RightChild;
    }

    const LocationSorted* elem = static_cast<const LocationSorted*>(elem_unique);

    // find lcp between elem->key and config->location
    uint64_t lcp = 0;
    for (lcp = 0; lcp < target_location_prefix.size() && lcp < elem->key.size(); ++lcp) {
        if (target_location_prefix[lcp] != elem->key[lcp]) {
            break;
        }
    }

    if (lcp == target_location_prefix.size()) {
        std::string owner = "";
        if (db != NULL && (this->compute_total_owner_cnt || elem->bp_alterations.size())) {
            owner = db->get_element(elem->database_id).covv_data[common::SEQ_FIELDS_TO_ID.at("owner")]; 
        }
        if (elem->bp_alterations.size()) {
            add_alters(elem->bp_alterations, elem->database_id, owner, db);
            owner_edit_cnt[owner]++;
        }
        if (this->compute_total_owner_cnt) {
            owner_total_cnt[owner]++;
        }
    }
    
    return LeftChild;
}
    
TreeDirectionToGo FreqBpQuery::second_enter_into_node(const std::string &target_location_prefix, Tnode *, const BaseSortedTreap *elem_unique, const ds::DB *) {
    if (elem_unique->key > target_location_prefix + "~") {
        return NoSubtree;
    }

    return RightChild;
}

void FreqBpQuery::print_results() {
    std::vector<std::pair<uint64_t, uint64_t>> top_bp_idx;

    for (uint64_t i = 0; i < common::ALIGNED_SEQ_SIZE; ++i) {
        top_bp_idx.push_back({i, alterations_per_bp[i]});
    }

    std::sort(top_bp_idx.begin(), top_bp_idx.end(), [](const std::pair<uint64_t, uint64_t> &a, const std::pair<uint64_t, uint64_t> &b){
        if (a.second != b.second) {
            return a.second > b.second;
        }
        return a.first < b.first;
    });

    std::cout << "\n\ntop " << num_to_print << " bp:\n";
    for (uint64_t i = 0; i < num_to_print; ++i) {
        if (top_bp_idx[i].second == 0) {
            break;
        }
        uint64_t bp_idx = top_bp_idx[i].first;
        std::cout << "basepair index = " << bp_idx << "\t total number of edits = " << top_bp_idx[i].second << std::endl;
        std::vector<std::pair<uint64_t, std::string>> owners_list;
        uint64_t num_owners = 0, total_edits = 0;
        for (const auto x : owner_distrib_per_bp[bp_idx]) {
            owners_list.push_back({x.second, x.first});
            ++num_owners;
            total_edits += x.second;
        }
        std::sort(owners_list.begin(), owners_list.end());
        std::cout << "owner distribution per bp --> #owners=" << num_owners << " #edits=" << total_edits <<":\n";
        for (int64_t i = owners_list.size() - 1; i >= 0; --i) {
            std::cout << "\t" << 100.0 * owners_list[i].first / total_edits << "% (" << owners_list[i].first << ") \t owner=" << owners_list[i].second << "\n";

            std::vector<std::pair<uint64_t, std::string>> bp_distrib;

            uint64_t num_char_to_char_types = 0;
            uint64_t total_owner_edits = 0;
            for (const auto x : char_to_char_distrib_per_bp_per_owner[bp_idx][owners_list[i].second]) {
                bp_distrib.push_back({x.second, x.first});
                ++num_char_to_char_types;
                total_owner_edits += x.second;
            }
            std::sort(bp_distrib.begin(), bp_distrib.end());
            // std::cout << "char to char distribution per bp --> #char_to_char_types=" << num_char_to_char_types << " #edits=" << total_edits << ":\n";
            for (int64_t i = bp_distrib.size() - 1; i >= 0; --i) {
                std::cout << "\t\t" << bp_distrib[i].second << " " << 100.0 * bp_distrib[i].first / total_owner_edits << "% (" << bp_distrib[i].first << ") " << "\n";
            }

            bp_distrib.clear();
            total_owner_edits = 0;
            for (const auto x : char_to_char_final_result_per_bp_per_owner[bp_idx][owners_list[i].second]) {
                bp_distrib.push_back({x.second, x.first});
                total_owner_edits += x.second;
            }
            std::sort(bp_distrib.begin(), bp_distrib.end());
            std::cout << "\t\t" << "Final base pair state\n";
            for (int64_t i = bp_distrib.size() - 1; i >= 0; --i) {
                std::cout << "\t\t" << bp_distrib[i].second << " " << 100.0 * bp_distrib[i].first / total_owner_edits << "% (" << bp_distrib[i].first << ") " << "\n";
            }

        }
        std::cout << "\n" << std::endl;
    }

    struct OwnerDetails {
        uint64_t edits;
        uint64_t uploads;
        std::string name;
    };

    std::vector<OwnerDetails> owners;
    for (const auto &it : owner_edit_cnt) {
        owners.push_back({it.second, owner_total_cnt[it.first], it.first});
    }

    std::sort(owners.begin(), owners.end(), 
    [](const OwnerDetails &a, const OwnerDetails &b) {
        if (a.edits != b.edits) {
            return a.edits > b.edits;
        }
        return a.name < b.name;
    });

    std::cout << "\n\ntop " << num_to_print << " owners:\n";
    for (uint64_t i = 0; i < owners.size() && i < num_to_print; ++i) {
        std::cout << owners[i].edits << "\t";
        if (this->compute_total_owner_cnt) {
            std::cout << owners[i].uploads << "\t";
        }
        std::cout << owners[i].name << std::endl;
    }
}

} // namespace query_ns
