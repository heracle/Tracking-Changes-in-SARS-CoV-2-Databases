#include "freq_bp.hpp"

#include <iostream>


#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/tnode_types/tnode_location.hpp"
#include "../common/logger.hpp"

namespace query_ns {

std::vector<std::pair<uint32_t, uint32_t>> FreqBpQuery::add_alters(const std::vector<std::pair<uint32_t, uint32_t>> &first, const std::vector<uint32_t> &second) const {
    std::vector<std::pair<uint32_t, uint32_t>> answer;
    uint32_t index_f = 0;
    uint32_t index_s = 0;

    while (index_f < first.size() && index_s < second.size()) {
        if (first[index_f].first < second[index_s]) {
            answer.push_back(first[index_f]);
            ++index_f;
        } else if (second[index_s] < first[index_f].first) {
            answer.push_back(std::make_pair(second[index_s], 1));
            ++index_s;
        } else {
            answer.push_back(std::make_pair(first[index_f].first, first[index_f].second + 1));
            ++index_f;
            ++index_s;
        }
    }
    while (index_f < first.size()) {
        answer.push_back(first[index_f]);
        ++index_f;
    }
    while (index_s < second.size()) {
        answer.push_back(std::make_pair(second[index_s], 1));
        ++index_s;
    }

    return answer;
}

FreqBpQuery::FreqBpQuery(const std::vector<std::string> &params) {
    if (params.size() != 1) {
        Logger::error("freq_bp query must receive exactly one parameter, the prefix of the location string to query");
    }
    target_location_prefix = params[0];
}

std::string FreqBpQuery::get_treap_name() {
    // todo make a field in constants.hpp.
    return "location_treap";
}

TreeDirectionToGo FreqBpQuery::first_enter_into_node(Tnode *tnode, const BaseSortedTreap *elem_unique, const ds::DB *db) {
    if (elem_unique->key < target_location_prefix) {
        return RightChild;
    }

    const LocationSorted* elem = static_cast<const LocationSorted*>(elem_unique);

    // find lcp between elem->key and config->location
    uint32_t lcp = 0;
    for (lcp = 0; lcp < target_location_prefix.size() && lcp < elem->key.size(); ++lcp) {
        if (target_location_prefix[lcp] != elem->key[lcp]) {
            break;
        }
    }

    if (lcp == target_location_prefix.size()) {
        altered_bp = add_alters(altered_bp, elem->bp_alterations);
        std::string owner = "";
        if (db != NULL) {
            owner = db->get_element(elem->database_id).covv_data[common::SEQ_FIELDS_TO_ID.at("owner")]; 
        }
        if (elem->bp_alterations.size()) {
            owner_edit_cnt[owner]++;
        }
        owner_total_cnt[owner]++;
    }
    
    return LeftChild;
}
    
TreeDirectionToGo FreqBpQuery::second_enter_into_node(Tnode *tnode, const BaseSortedTreap *elem_unique, const ds::DB *db) {
    if (elem_unique->key > target_location_prefix + "~") {
        return NoSubtree;
    }

    return RightChild;
}

void FreqBpQuery::print_results() {
    std::cout << "\n\nanswer_altered_bp size=" << altered_bp.size() << std::endl;
    for (const auto &pr : altered_bp) {
        std::cout << pr.first << "\t" << pr.second << std::endl;
    }

    std::sort(altered_bp.begin(), altered_bp.end(), [](const std::pair<uint32_t, uint32_t> &a, const std::pair<uint32_t, uint32_t> &b){
        if (a.second != b.second) {
            return a.second > b.second;
        }
        return a.first > b.first;
    });

    std::cout << "\n\ntop 50 bp:\n";
    for (uint32_t i = 0; i < altered_bp.size() && i < 50; ++i) {
        std::cout << altered_bp[i].first << "\t" << altered_bp[i].second << std::endl;
    }

    struct OwnerDetails {
        uint32_t edits;
        uint32_t uploads;
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
        return a.uploads > b.uploads;
    });

    std::cout << "\n\ntop 50 owners:\n";
    for (uint32_t i = 0; i < owners.size() && i < 50; ++i) {
        std::cout << owners[i].edits << "\t" << owners[i].uploads << "\t" << owners[i].name << std::endl;
    }
}

} // namespace query_ns
