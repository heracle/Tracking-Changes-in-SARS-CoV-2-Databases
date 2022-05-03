# pragma once

#include "base_query.hpp"
#include "../cli/config.hpp"

#include "../ds/ctc.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/static_types/static_base.hpp"

namespace query_ns {

class CountIndelsQuery : public BaseQuery {
  private:
    Tnode *lca_tnode = NULL;
    bool found_first = false;
    bool before_lca_tnode = true;

  public:
    std::vector<uint32_t> total_inserted_sequences;
    std::vector<uint32_t> total_modified_sequences;
    std::vector<uint32_t> total_deleted_sequences;
    
    CountIndelsQuery(const uint32_t num_total_snapshots);
    std::string get_treap_name();
    TreeDirectionToGo first_enter_into_node(const std::string &target_location_prefix, Tnode *, const BaseSortedTreap *, const ds::DB *);
    TreeDirectionToGo second_enter_into_node(const std::string &target_location_prefix, Tnode *, const BaseSortedTreap *, const ds::DB *);
    void print_results();
    void reset();
    void set_deletion_mode(const bool is_deletion_mode);
};

} // namespace query_ns
