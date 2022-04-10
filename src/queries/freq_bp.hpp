# pragma once

#include "base_query.hpp"

#include "../ds/ctc.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/static_types/static_base.hpp"

namespace query_ns {

class FreqBpQuery : public BaseQuery {
  private:
    std::vector<std::pair<uint32_t, uint32_t>> altered_bp;
    std::string target_location_prefix;
    Tnode *lca_tnode = NULL;
    bool found_first = false;
    bool before_lca_tnode = true;
    tsl::hopscotch_map<std::string, uint32_t> owner_edit_cnt;
    tsl::hopscotch_map<std::string, uint32_t> owner_total_cnt;

    std::vector<std::pair<uint32_t, uint32_t>> add_alters(const std::vector<std::pair<uint32_t, uint32_t>> &main_altered_bp, const std::vector<uint32_t> &secondary) const;

  public:
    FreqBpQuery(const std::vector<std::string> &params);
    std::string get_treap_name();
    TreeDirectionToGo first_enter_into_node(Tnode *, const BaseSortedTreap *, const ds::DB *);
    TreeDirectionToGo second_enter_into_node(Tnode *, const BaseSortedTreap *, const ds::DB *);
    void print_results();
};

} // namespace query_ns