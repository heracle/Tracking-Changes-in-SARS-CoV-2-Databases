#include "freq_bp.hpp"

#include <iostream>
#include <cstring>

#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/tnode_types/tnode_location.hpp"
#include "../common/logger.hpp"

namespace query_ns {

bool is_string_included(const std::string &small, const std::string &large) {
    if (small.size() > large.size()) {
        return false;
    }

    for (char x : small) {
        bool is_in_large = false;
        for (char y : large) {
            if (y == x) {
                is_in_large = true;
                break;
            }
        }
        if (is_in_large == false) {
            return false;
        }
    }
    return true;
}

std::string get_bp_code(char x) {
    if (x == 'A') {
        return "A";
    } else if (x == 'G') {
        return "G";
    } else if (x == 'C') {
        return "C";
    } else if (x == 'T') {
        return "T";
    } else if (x == 'Y') {
        return "CT";
    } else if (x == 'R') {
        return "AG";
    } else if (x == 'W') {
        return "AT";
    } else if (x == 'S') {
        return "GC";
    } else if (x == 'K') {
        return "TG";
    } else if (x == 'M') {
        return "CA";
    } else if (x == 'D') {
        return "AGT";
    } else if (x == 'V') {
        return "ACG";
    } else if (x == 'H') {
        return "ACT";
    } else if (x == 'B') {
        return "CGT";
    } else if (x == 'N') {
        return "AGCT";
    } 
    Logger::error("not recognised char");
}

void FreqBpQuery::eval_alteration_type(char prv, char act) {
    if (prv == act) {
        Logger::error("eval alteration type for equal chars");
    }

    if (prv == 'X' || prv == '-') {
        prv = 'N';
    }
    if (act == 'X' || act == '-') {
        act = 'N';
    }

    if (prv == 'N' && act == 'N') {
        num_alter_n_dash[snapshot_idx]++;
        return;
    }

    std::string prv_s = get_bp_code(prv);
    std::string act_s = get_bp_code(act);

    if (is_string_included(prv_s, act_s)) {
        num_alter_less_precise[snapshot_idx]++;
    } else if (is_string_included(act_s, prv_s)) {
        num_alter_more_precise[snapshot_idx]++;
    } else {
        num_problem_alter_precise[snapshot_idx]++;
    }
}

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

        eval_alteration_type(prv_char, act_char);

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

FreqBpQuery::FreqBpQuery(const bool req_compute_total_owner_cnt, const uint64_t req_num_to_print, const uint64_t num_total_snapshots) {
    this->compute_total_owner_cnt = req_compute_total_owner_cnt;
    this->num_to_print = req_num_to_print;

    num_alter_more_precise.resize(num_total_snapshots);
    num_alter_less_precise.resize(num_total_snapshots);
    num_problem_alter_precise.resize(num_total_snapshots);
    num_alter_n_dash.resize(num_total_snapshots);

    num_total_sequences.resize(num_total_snapshots);
    num_stable_sequences.resize(num_total_snapshots);
    num_seq_modified_after_X_weeks.resize(num_total_snapshots);
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
        if (processed_database_ids.count(elem->database_id) == 0) {
            processed_database_ids.insert(elem->database_id);

            std::string acc_id = db->get_element(elem_unique->database_id).covv_data[common::SEQ_FIELDS_TO_ID.at("covv_accession_id")];
            num_total_sequences[snapshot_idx]++;
            num_stable_sequences[snapshot_idx]++;

            if (acc_id_to_last_label.count(acc_id) == 0) {
                acc_id_to_last_label[acc_id] = snapshot_idx;
                return LeftChild;
            }

            uint64_t prv_snap = acc_id_to_last_label.at(acc_id);
            acc_id_to_last_label[acc_id] = snapshot_idx;

            num_seq_modified_after_X_weeks[snapshot_idx - prv_snap]++;
            num_stable_sequences[prv_snap]--;
            
            for (const uint64_t alteration : elem->bp_alterations) {
                uint64_t altered_bp = (alteration >> common::BITS_FOR_STEPS_BACK);
                uint64_t num_versions_back = alteration - (altered_bp << common::BITS_FOR_STEPS_BACK);
                if (num_versions_back != 0) {
                    continue;
                }
                num_seq_modified_after_X_weeks_per_bp[altered_bp][snapshot_idx - prv_snap]++;
            }
        }
        // std::string owner = "";
        // if (db != NULL && (this->compute_total_owner_cnt || elem->bp_alterations.size())) {
        //     owner = db->get_element(elem->database_id).covv_data[common::SEQ_FIELDS_TO_ID.at("owner")]; 
        // }
        // if (elem->bp_alterations.size()) {
        //     add_alters(elem->bp_alterations, elem->database_id, owner, db);
        //     owner_edit_cnt[owner]++;
        // }
        // if (this->compute_total_owner_cnt) {
        //     owner_total_cnt[owner]++;
        // }
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
    std::cout << "Num stable sequences:\n";
    for (uint64_t i = 0; i < num_stable_sequences.size(); ++i) {
        std::cout << "week=" << i + 1 << " " << num_stable_sequences[i] << " out of " << num_total_sequences[i] << "\t" << ((double) num_stable_sequences[i]) / num_total_sequences[i] << "\n";
    }
    std::cout << "Percent of stable sequences\n"; 
    for (uint64_t i = 0; i < num_stable_sequences.size(); ++i) {
        std::cout << "(" << i + 1 << ", " << (int((1 - ((double) num_stable_sequences[i]) / num_total_sequences[i]) * 100000)) / 1000.0 << ")\n";
    }

    std::cout << "Seq modified after X weeks:\n";
    double total_edits = 0;
    for (uint64_t i = 0; i < num_stable_sequences.size(); ++i) {
        std::cout << "week=" << i + 1 << " " << num_seq_modified_after_X_weeks[i] << "\n";
        total_edits += num_seq_modified_after_X_weeks[i];
    }
    std::cout << "Percent of edits after X weeks since the first submission:\n";
    for (uint64_t i = 0; i < num_stable_sequences.size(); ++i) {
        std::cout << "(" << i + 1 << ", " << (int((((double)num_seq_modified_after_X_weeks[i]) / total_edits) * 100000)) / 1000.0 << ")\n";
    }

    std::vector<std::pair<std::pair<uint64_t, uint64_t>, double>> updates_bp_clusters;

    std::cout << "Number of changes per clusters of bp ";
    uint64_t clusters = 200;
    std::cout << "cluster=" << clusters << "\n";
    for (uint64_t bp = 0; bp < common::ALIGNED_SEQ_SIZE; bp += clusters) {
        double total_changes = 0;
        for (uint64_t i = bp; i < bp + clusters; ++i) {
            for (uint64_t week = 0; week < 40; ++week) {
                total_changes += ((double)num_seq_modified_after_X_weeks_per_bp[i][week]) / processed_database_ids.size();
            }
        }
        total_changes /= clusters;
        total_changes *= 100; //to make a percent
        std::cout << "(" << bp + clusters << ", " << total_changes << ")\n";
    }

    std::cout << "Avg. number of weeks until bp cluster is stable ";
    std::cout << "cluster=" << clusters << "\n";
    for (uint64_t bp = 0; bp < common::ALIGNED_SEQ_SIZE; bp += clusters) {
        double total_changes = 0;
        double weighted_changes = 0;
        for (uint64_t i = bp; i < bp + clusters; ++i) {
            for (uint64_t week = 0; week < 40; ++week) {
                total_changes += num_seq_modified_after_X_weeks_per_bp[i][week];
                weighted_changes += num_seq_modified_after_X_weeks_per_bp[i][week] * week;
            }
        }
        double ratio = 1;
        if (total_changes != 0) {
            ratio = weighted_changes / total_changes;
        }
        std::cout << "(" << bp + clusters << ", " << ratio << ")\n";
        updates_bp_clusters.push_back(std::make_pair(std::make_pair(bp, bp+clusters), ratio));
    }

    // std::sort(updates_bp_clusters.begin(), updates_bp_clusters.end(), [](std::pair<std::pair<uint64_t, uint64_t>, double> &a, std::pair<std::pair<uint64_t, uint64_t>, double> &b) {
    //     return a.second > b.second;
    // });

    // std::cout << "\n\ntop " << num_to_print << " bp clusters:\n";
    // for (uint64_t i = 0; i < updates_bp_clusters.size(); ++i) {
    //     std::cout << updates_bp_clusters[i].first.first << "-" << updates_bp_clusters[i].first.second << "\t" << updates_bp_clusters[i].second << "\n";
    // }


    return;
    std::cout << "More precise alterations:\n";
    for (uint64_t i = 0; i < num_alter_more_precise.size(); ++i) {
        std::cout << "(" << i+1 << ", " << num_alter_more_precise[i] << ")\n";
    }
    std::cout << "Less precise alterations:\n";
    for (uint64_t i = 0; i < num_alter_less_precise.size(); ++i) {
        std::cout << "(" << i+1 << ", " << num_alter_less_precise[i] << ")\n";
    }
    std::cout << "Problematic alterations:\n";
    for (uint64_t i = 0; i < num_problem_alter_precise.size(); ++i) {
        std::cout << "(" << i+1 << ", " << num_problem_alter_precise[i] << ")\n";
    }

    std::cout << "Null alterations:\n";
    for (uint64_t i = 0; i < num_alter_n_dash.size(); ++i) {
        std::cout << "(" << i+1 << ", " << num_alter_n_dash[i] << ")\n";
    }

    return;

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
