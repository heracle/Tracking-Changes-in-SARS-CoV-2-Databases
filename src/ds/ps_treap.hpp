#pragma once

#include <vector>
#include <functional>

#include "../external_libraries/hopscotch-map/include/tsl/hopscotch_map.h"
#include "../external_libraries/hopscotch-map/include/tsl/hopscotch_set.h"
#include "../common/json_helper.hpp"
#include "../queries/base_query.hpp"

#include "tnode_types/tnode_base.hpp"
#include "static_types/static_base.hpp"
using namespace treap_types;

namespace ds {

class PS_Treap {
  //todo fix this private field
  public:
      std::vector<std::unique_ptr<BaseSortedTreap>> static_data;
      std::vector<uint32_t> prv_static_data;
  private:
  // Pieces of code taken from: https://cp-algorithms.com/data_structures/treap.html
    /* The 'data' field stores all the sequences which have been previously inserted to the treap.
     * * This data is not stored inside the treap node structure (Tnode*) for time efficiency reasons.
     * * 
     */  
  public: //todo fix in private
    Tnode *root = NULL;
  private:
    tsl::hopscotch_map<std::string, uint32_t> date_to_root_idx;
    std::vector<Tnode*> root_history;

    // make this a static/normal method inside Tnode.
    std::function<void(Tnode *, const BaseSortedTreap*)> recompute_tnode_statistics;
    std::function<Tnode*(const uint32_t)> create_new_specialized_tnode;
    std::function<Tnode*(const Tnode*)> copy_specialized_tnode;
    // todo replace int with uint32_t and long long with uint64_t.

    friend class Tnode;

    // Less or equal will be in &left. Greater in &right.
    void split(Tnode *&tnode, const BaseSortedTreap &key, Tnode *&left, Tnode *&right);
    void merge(Tnode *&tnode, Tnode *left, Tnode *right);
    void insert_tnode(Tnode *&tnode, Tnode *to_add);
    void erase_node(Tnode *&node, const BaseSortedTreap &to_delete, int &deleted_key_index);
    void run_tnode_callback(const Tnode *tnode, const std::function<void(const BaseSortedTreap &)> &callback) const;
    void run_treap_query_callback_subtree(Tnode *tnode, const ds::DB *db, query_ns::BaseQuery *query) const;
    void delete_subtree(Tnode *&node);
    void save_tnodes_in_subtree(tsl::hopscotch_set<Tnode*> &tnodes_to_save, Tnode *node);
    static void get_next_stack_tnode(std::vector<Tnode*> &stack);
  public:
    // todo use unique pointer get and send "BaseSortedTreap*".
    PS_Treap(const std::function<void(Tnode *, const BaseSortedTreap*)> &recompute_tnode_statistics,
             const std::function<Tnode*(const uint32_t)> &req_create_new_specialized_tnode,
             const std::function<Tnode*(const Tnode*)> &req_copy_specialized_tnode);
    PS_Treap(const H5::Group &treap_group,
             std::vector<std::unique_ptr<BaseSortedTreap>> &treap_data,
             const std::function<void(Tnode *, const BaseSortedTreap*)> &recompute_tnode_statistics,
             const std::function<Tnode*(const uint32_t)> &req_create_new_specialized_tnode,
             const std::function<Tnode*(const Tnode*)> &req_copy_specialized_tnode,
             const std::function<Tnode*(const uint32_t, const uint64_t, const uint32_t)> &get_h5_tnode);
    PS_Treap(PS_Treap *source);

    ~PS_Treap();
    // we need to delete the old nodes, so it can't be a 'const'.
    /*
      insert method: only .second is used from 'seq_elems_with_prv'.
    */
    void insert(std::vector<std::unique_ptr<BaseSortedTreap>> &nodes, const std::vector<std::pair<common::SeqElem, uint32_t> > &seq_elems_with_prv = std::vector<std::pair<common::SeqElem, uint32_t> >());
    void erase(const std::vector<uint32_t> &nodes_indices); // delete list of indeces returned by 'get_differences()' method.
    void save_snapshot(const std::string &name);

    std::vector<std::string> get_snapshots_names();
    uint32_t get_data_size();

    Tnode* find(const BaseSortedTreap &target);
    // void upper_bound(const T& target);

    void export_to_hdf5(H5::Group &treap_group, 
                        const std::function<void(const std::vector<std::unique_ptr<BaseSortedTreap>> &, H5::Group &)> &write_static_data_to_h5,
                        const std::function<void(Tnode *)> &append_tnode_data); 

    // todo add unit tests
    // looks that 'get_data_from_iterator' is not used
    // T get_data_from_iterator(Tnode *it);

    // Run the callback function against each tnode in the mid order traversal (sorted order).
    void iterate_ordered(const std::function<void(const BaseSortedTreap &)> &callback, const std::string &snapshot = "");

    // delete iterate_ordered and replace with 'query_callback_subtree'
    void query_callback_subtree(query_ns::BaseQuery *query,
                                const ds::DB *db = NULL,
                                const std::string &snapshot = "") const;

    static void get_differences(const PS_Treap *first_treap,
                                const PS_Treap *second_treap,
                                std::vector<uint32_t> &insertions_db_ids,
                                std::vector<uint32_t> &deletions_db_ids,
                                std::vector<std::pair<uint32_t, uint32_t>> &updates_db_ids);
};

} // namespace ds