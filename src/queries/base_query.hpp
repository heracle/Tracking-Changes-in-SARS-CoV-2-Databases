#pragma once

#include <string>

#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/static_types/static_base.hpp"
#include "../ds/database.hpp"

namespace query_ns {

enum TreeDirectionToGo {
    NoSubtree = 0,
    LeftChild = 1,
    RightChild = 2,
};

class BaseQuery {
  public:
    // init does some initialisation for the specific query and returns the name of the treap to run.
    virtual std::string get_treap_name() = 0;
    virtual TreeDirectionToGo first_enter_into_node(treap_types::Tnode *, const treap_types::BaseSortedTreap *, const ds::DB *db) = 0;
    virtual TreeDirectionToGo second_enter_into_node(treap_types::Tnode *, const treap_types::BaseSortedTreap *, const ds::DB *db) = 0;
    virtual void print_results() = 0;
    virtual ~BaseQuery(){};
};

} // namespace query_ns