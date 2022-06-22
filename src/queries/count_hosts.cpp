#include "count_hosts.hpp"

#include <cstring>
#include <iostream>

#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/tnode_types/tnode_location.hpp"
#include "../common/logger.hpp"

namespace query_ns {

TreeDirectionToGo CountHostsQuery::first_enter_into_node(const std::string &target_location_prefix, Tnode *, const BaseSortedTreap *elem_unique, const ds::DB *db) {
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
        std::string host_type;
        // Consider the static field `is_human_host` to reduce the number of DB requests, because those are much slower than an 'elem' access.
        // So, in the case of 'elem->is_human_host' true, we will set the host_type to 'Human' without any DB access.
        if (elem->is_human_host) {
            host_type = "Human";
        } else {
            host_type = db->get_element(elem->database_id).covv_data[common::SEQ_FIELDS_TO_ID.at("covv_host")];
        } 
        total_host_occurrences[snapshot_idx][host_type] ++;
    }
    
    return LeftChild;
}

TreeDirectionToGo CountHostsQuery::second_enter_into_node(const std::string &target_location_prefix, Tnode *, const BaseSortedTreap *elem_unique, const ds::DB *) {
    if (elem_unique->key > target_location_prefix + "~") {
        return NoSubtree;
    }

    return RightChild;
}

CountHostsQuery::CountHostsQuery(const uint64_t num_total_snapshots) {
    total_host_occurrences.resize(num_total_snapshots);
    reset();
}

std::string CountHostsQuery::get_treap_name() {
    return "location_treap";
}

void CountHostsQuery::print_results() {
    std::cout << "Total host occurrences:\t";

    for (uint64_t i = 0; i < total_host_occurrences.size(); ++i) {
        std::cout << "label=" << i << " ";
        for (const auto &it : total_host_occurrences[i]) {
            std::cout << it.first << ":" << it.second << "\t";
        }
        std::cout << std::endl;
    }
}

void CountHostsQuery::reset() {
    for (uint32_t i = 0; i < total_host_occurrences.size(); ++i) {
        total_host_occurrences[i].clear();
    } 
}

void CountHostsQuery::set_deletion_mode(const bool is_deletion_mode) {
    lca_tnode = NULL;
    before_lca_tnode = true;
    deletions_mode = is_deletion_mode;
}

} // namespace query_ns