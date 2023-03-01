#include "count_indels.hpp"

#include <cstring>
#include <iostream>

#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/tnode_types/tnode_location.hpp"
#include "../common/logger.hpp"

namespace query_ns {

CountIndelsQuery::CountIndelsQuery(const uint64_t num_total_snapshots) {
    reset();
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
    uint64_t lcp = 0;
    for (lcp = 0; lcp < target_location_prefix.size() && lcp < elem->key.size(); ++lcp) {
        if (target_location_prefix[lcp] != elem->key[lcp]) {
            break;
        }
    }
    bool go_directly_to_right = false;
    auto &saved = saved_data(snapshot_current_name);

    if (lca_tnode == NULL && lcp == target_location_prefix.size()) {
        lca_tnode = tnode;
    } else if (before_lca_tnode && lcp == target_location_prefix.size()) {
        // take the entire subtree of tnode->right
        if (tnode->r != NULL) {
            const LocationTnode *right_tnode = static_cast<const LocationTnode*>(tnode->r);
            if (deletions_mode) {
                saved("deleted").SetValInt(
                    saved("deleted").GetValInt() + right_tnode->total_nodes_in_subtree
                );
            } else {
                saved("inserted").SetValInt(
                    saved("inserted").GetValInt() + right_tnode->total_nodes_in_subtree
                );
            }
            saved("modified").SetValInt(
                saved("modified").GetValInt() + right_tnode->total_versions_in_subtree
            );
        }
    } else if (before_lca_tnode == false && lcp == target_location_prefix.size()) {
        if (tnode->l != NULL) {
            const LocationTnode *left_tnode = static_cast<const LocationTnode*>(tnode->l);
            if (deletions_mode) {
                saved("deleted").SetValInt(
                    saved("deleted").GetValInt() + left_tnode->total_nodes_in_subtree
                );
            } else {
                saved("inserted").SetValInt(
                    saved("inserted").GetValInt() + left_tnode->total_nodes_in_subtree
                );
            }
            saved("modified").SetValInt(
                saved("modified").GetValInt() + left_tnode->total_versions_in_subtree
            );
        }
        go_directly_to_right = true;
    }
    if (lcp == target_location_prefix.size()) {
        if (deletions_mode) {
            saved("deleted").SetValInt(saved("deleted").GetValInt() + 1);
        } else {
            saved("inserted").SetValInt(saved("inserted").GetValInt() + 1);
        }
        saved("modified").SetValInt(
            saved("modified").GetValInt() + elem->num_sequence_versions
        );
    }

    if (go_directly_to_right) {
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
    uint64_t lcp = 0;
    for (lcp = 0; lcp < target_location_prefix.size() && lcp < elem->key.size(); ++lcp) {
        if (target_location_prefix[lcp] != elem->key[lcp]) {
            break;
        }
    }

    if (tnode == lca_tnode) {
        before_lca_tnode = false;
    }

    if (before_lca_tnode && lcp == target_location_prefix.size()) {
        return NoSubtree;
    }

    return RightChild;
}

void CountIndelsQuery::reset() {
    saved_data.ClearData();
}

void CountIndelsQuery::set_deletion_mode(const bool is_deletion_mode) {
    lca_tnode = NULL;
    before_lca_tnode = true;
    deletions_mode = is_deletion_mode;
}

} // namespace query_ns
