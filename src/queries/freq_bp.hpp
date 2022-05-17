# pragma once

#include "base_query.hpp"
#include "../cli/config.hpp"

#include "../ds/ctc.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/static_types/static_base.hpp"

namespace query_ns {

class FreqBpQuery : public BaseQuery {
  private:
    uint64_t alterations_per_bp[common::ALIGNED_SEQ_SIZE];
    std::string target_location_prefix;
    Tnode *lca_tnode = NULL;
    bool found_first = false;
    bool before_lca_tnode = true;
    bool compute_total_owner_cnt;
    uint64_t num_to_print;
    tsl::hopscotch_map<std::string, uint64_t> owner_edit_cnt;
    tsl::hopscotch_map<std::string, uint64_t> owner_total_cnt;

    tsl::hopscotch_map<std::string, uint64_t> owner_distrib_per_bp[common::ALIGNED_SEQ_SIZE];
    tsl::hopscotch_map<std::string, tsl::hopscotch_map<std::string, uint64_t>> char_to_char_distrib_per_bp_per_owner[common::ALIGNED_SEQ_SIZE];
    tsl::hopscotch_map<std::string, tsl::hopscotch_map<std::string, uint64_t>> char_to_char_final_result_per_bp_per_owner[common::ALIGNED_SEQ_SIZE];
    tsl::hopscotch_map<uint64_t, std::pair<std::string, std::string>> prv_sequences;

    void add_alters(const std::vector<uint64_t> bp_alterations, uint64_t database_id, const std::string &owner, const ds::DB *db);

  public:
    FreqBpQuery(const bool req_compute_total_owner_cnt, const uint64_t req_num_to_print);
    void reset();
    void set_deletion_mode(const bool is_deletion_mode);
    std::string get_treap_name();
    TreeDirectionToGo first_enter_into_node(const std::string &, Tnode *, const BaseSortedTreap *, const ds::DB *);
    TreeDirectionToGo second_enter_into_node(const std::string &, Tnode *, const BaseSortedTreap *, const ds::DB *);
    void print_results();
};

} // namespace query_ns