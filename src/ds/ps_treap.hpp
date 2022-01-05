#pragma once

#include <vector>
#include <functional>

#include "../external_libraries/hopscotch-map/include/tsl/hopscotch_map.h"
#include "../external_libraries/hopscotch-map/include/tsl/hopscotch_set.h"
#include "../common/utils.hpp"

#include "treap_representation/static_base.hpp"
using namespace treap_types;

namespace ds {

class PS_Treap {
  private:
  // Pieces of code taken from: https://cp-algorithms.com/data_structures/treap.html
    /* The 'data' field stores all the sequences which have been previously inserted to the treap.
     * * This data is not stored inside the treap node structure (Tnode*) for time efficiency reasons.
     * * 
     */  
    std::vector<std::unique_ptr<BaseSortedTreap>> static_data;

    common::Tnode *root = NULL;
    tsl::hopscotch_map<std::string, uint32_t> date_to_root_idx;
    std::vector<common::Tnode*> root_history;

    // make this a static/normal method inside Tnode.
    std::function<void(common::Tnode *, const std::unique_ptr<BaseSortedTreap>&)> recompute_tnode_statistics;

    friend class common::Tnode;

    // Less or equal will be in &left. Greater in &right.
    void split(common::Tnode *&tnode, const BaseSortedTreap &key, common::Tnode *&left, common::Tnode *&right);
    void merge(common::Tnode *&tnode, common::Tnode *left, common::Tnode *right);
    void insert_tnode(common::Tnode *&tnode, common::Tnode *to_add);
    void erase_node(common::Tnode *&node, const BaseSortedTreap &to_delete, int &deleted_key_index);
    void run_tnode_callback(const common::Tnode *tnode, const std::function<void(const BaseSortedTreap &)> &callback);
    void run_treap_query_callback_subtree(common::Tnode *tnode, 
                                          const std::function<int(common::Tnode *, const BaseSortedTreap *)> &callback_fst,
                                          const std::function<int(common::Tnode *, const BaseSortedTreap *)> &callback_scd);
    void delete_subtree(common::Tnode *&node);
    void save_tnodes_in_subtree(tsl::hopscotch_set<common::Tnode*> &tnodes_to_save, common::Tnode *node);
    static void get_next_stack_tnode(std::vector<common::Tnode*> &stack);
  public:
    // todo use unique pointer get and send "BaseSortedTreap*".
    PS_Treap(const std::function<void(common::Tnode *, const std::unique_ptr<BaseSortedTreap>&)> &recompute_tnode_statistics);
    PS_Treap(const H5::Group &treap_group,
             std::vector<std::unique_ptr<BaseSortedTreap>> &treap_data,
             const std::function<void(common::Tnode *, const std::unique_ptr<BaseSortedTreap>&)> &recompute_tnode_statistics);

    ~PS_Treap();
    // we need to delete the old nodes, so it can't be a 'const'.
    void insert(std::vector<std::unique_ptr<BaseSortedTreap>> &nodes);
    void erase(const std::vector<uint32_t> &nodes_indices); // delete list of indeces returned by 'get_differences()' method.
    void save_snapshot(const std::string &name);

    std::vector<std::string> get_snapshots_names();
    uint32_t get_data_size();

    common::Tnode* find(const BaseSortedTreap &target);
    // void upper_bound(const T& target);

    void export_to_hdf5(H5::Group &treap_group, const std::function<void(const std::unique_ptr<BaseSortedTreap> &, H5::Group &)> &h5_append_elem); 

    // todo add unit tests
    // looks that 'get_data_from_iterator' is not used
    // T get_data_from_iterator(common::Tnode *it);

    // Run the callback function against each tnode in the mid order traversal (sorted order).
    void iterate_ordered(const std::function<void(const BaseSortedTreap &)> &callback, const std::string &snapshot = "");

    // delete iterate_ordered and replace with 'query_callback_subtree'
    void query_callback_subtree(const std::function<int(common::Tnode *, const BaseSortedTreap *)> &callback_fst,
                                const std::function<int(common::Tnode *, const BaseSortedTreap *)> &callback_scd,
                                const std::string &snapshot = "");

    static void get_differences(const PS_Treap *first_treap,
                                const PS_Treap *second_treap,
                                std::vector<uint32_t> &insertions_db_ids,
                                std::vector<uint32_t> &deletions_db_ids,
                                std::vector<std::pair<uint32_t, uint32_t>> &updates_db_ids);
};

} // namespace ds