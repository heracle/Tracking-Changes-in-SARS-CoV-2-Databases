#include "count_indels.hpp"

#include <cstring>
#include <iostream>

#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/tnode_types/tnode_location.hpp"
#include "../common/logger.hpp"

namespace query_ns {

CountIndelsQuery::CountIndelsQuery() {
    // filepath_to_read = req_filepath;
}

std::string CountIndelsQuery::get_treap_name() {
    // todo make a field in constants.hpp.
    return "location_treap";
}

TreeDirectionToGo CountIndelsQuery::first_enter_into_node(const std::string &target_location_prefix, Tnode *tnode, const BaseSortedTreap *elem_unique, const ds::DB *db) {
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

    if (lca_tnode == NULL && lcp == target_location_prefix.size()) {
        lca_tnode = tnode;
    } else if (before_lca_tnode && lcp == target_location_prefix.size()) {
        // take the entire subtree of tnode->right
        if (tnode->r != NULL) {
            const LocationTnode *right_tnode = static_cast<const LocationTnode*>(tnode->r);
            num_sequences += right_tnode->total_nodes_in_subtree;
            sum_versions += right_tnode->total_versions_in_subtree;
        }
    } else if (before_lca_tnode == false && lcp == target_location_prefix.size()) {
        if (tnode->l != NULL) {
            const LocationTnode *left_tnode = static_cast<const LocationTnode*>(tnode->l);
            num_sequences += left_tnode->total_nodes_in_subtree;
            sum_versions += left_tnode->total_versions_in_subtree;
        }

        num_sequences++;
        sum_versions += elem->num_sequence_versions;
        return RightChild;
    }
    return LeftChild;
}

TreeDirectionToGo CountIndelsQuery::second_enter_into_node(const std::string &target_location_prefix, Tnode *tnode, const BaseSortedTreap *elem_unique, const ds::DB *) {
    if (elem_unique->key > target_location_prefix + "~") {
        return NoSubtree;
    }
    const LocationSorted* elem = static_cast<const LocationSorted*>(elem_unique);
    // find lcp between elem->key and config->location
    uint32_t lcp = 0;
    for (lcp = 0; lcp < target_location_prefix.size() && lcp < elem->key.size(); ++lcp) {
        if (target_location_prefix[lcp] != elem->key[lcp]) {
            break;
        }
    }

    if (tnode == lca_tnode) {
        before_lca_tnode = false;
    }

    if (lcp == target_location_prefix.size()) {
        num_sequences++;
        sum_versions += elem->num_sequence_versions;
    }

    if (before_lca_tnode && lcp == target_location_prefix.size()) {
        return NoSubtree;
    }

    return RightChild;
}

void CountIndelsQuery::print_results(const std::string &query_location) {
    std::cout << "Location prefix: '" << query_location << "', total number of sequences: " << num_sequences << " with total number of versions: " << sum_versions << std::endl;
}

void CountIndelsQuery::reset() {
    num_sequences = 0;
    sum_versions = 0;
    lca_tnode = NULL;
    before_lca_tnode = true;
}

} // namespace query_ns
