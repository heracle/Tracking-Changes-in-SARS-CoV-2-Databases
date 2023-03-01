# pragma once

#include "base_query.hpp"
#include "../cli/config.hpp"

#include "../ds/ctc.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/static_types/static_base.hpp"

namespace query_ns {

/*
    A query is composed of QS query strings and SN snapshots. 
    Thus, the Treap will be iterated QS x SN times.

    At each iteration, the nodes will be called in their lexicographic order:
    <*>     'first_enter_into_node' is called before iterating the node's subtree
    <*>     'second_enter_into_node' is called after iterating the node's left subtree

    For avoiding a full tree iteration, both 'first_enter_into_node' and 'second_enter_into_node' return a TreeDirectionToGo object. 
    
    This TreeDirectionToGo is an enum with 3 values:
    <*>     NoSubtree=0  -> skip any subtree iteration
    <*>     LeftChild=1  -> continue the iteration with the left child
    <*>     RightChild=2 -> continue the iteration with the right child
*/
class CountHostsQuery : public BaseQuery {
  private:
    Tnode *lca_tnode = NULL;
    bool found_first = false;
    bool before_lca_tnode = true;

  public:    
    CountHostsQuery(const uint64_t num_total_snapshots);
    std::string get_treap_name();
    TreeDirectionToGo first_enter_into_node(const std::string &target_location_prefix, Tnode *, const BaseSortedTreap *, const ds::DB *);
    TreeDirectionToGo second_enter_into_node(const std::string &target_location_prefix, Tnode *, const BaseSortedTreap *, const ds::DB *);
    /*
        'reset()' has to be called after printing the results for one query string.
    */
    void reset();
    /*
        'set_deletion_mode(true)' has to be called before a deletion treap iteration.
        'set_deletion_mode(false)' has to be called before a standard treap iteration. 
    */
    void set_deletion_mode(const bool is_deletion_mode);
};

} // namespace query_ns
