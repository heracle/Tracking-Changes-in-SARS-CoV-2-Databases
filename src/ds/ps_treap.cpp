#include "ps_treap.hpp"

#include <cassert>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "../common/logger.hpp"
#include "../common/constants.hpp"
#include "../common/h5_helper.hpp"

#include "database.hpp"

#include "static_types/static_acc_id.hpp"

using namespace treap_types;

namespace ds {

void PS_Treap::split(Tnode *&tnode, const BaseSortedTreap &key, Tnode *&left, Tnode *&right) {
    if (tnode == NULL) {
        left = right = NULL;
        return;
    }
    Tnode *old = tnode;
    // create a copy of node for persistency purpose.
    // tnode = new Tnode(*tnode);
    tnode = copy_specialized_tnode(tnode);
    old->try_to_suicide();

    if (key < *static_data[tnode->data_id]) {
        Tnode *new_left = NULL;
        split(tnode->l, key, left, new_left);
        tnode->l = new_left;
        right = tnode;
        if (tnode->l != NULL) {
            //maybe not needed
            recompute_tnode_statistics(tnode->l, static_data[tnode->l->data_id].get());
        }
        if (right != NULL) {
            recompute_tnode_statistics(right, static_data[right->data_id].get());
        }
    } else {
        Tnode *new_right = NULL;
        split(tnode->r, key, new_right, right);
        tnode->r = new_right;
        left = tnode;
        if (tnode->r != NULL) {
            //maybe not needed
            recompute_tnode_statistics(tnode->r, static_data[tnode->r->data_id].get());
        }
        if (left != NULL) {
            recompute_tnode_statistics(left, static_data[left->data_id].get());
        }
    }
}

void PS_Treap::merge(Tnode *&tnode, Tnode *left, Tnode *right) {
    if (left == NULL || right == NULL) {
        tnode = (left != NULL ? left : right);
        return;
    }
    Tnode *old;
    if (left->prio > right->prio) {
        Tnode *new_node = NULL;
        merge(new_node, left->r, right);
        old = left;
        // tnode = new Tnode(*left);
        tnode = copy_specialized_tnode(left);
        tnode->r = new_node;
    } else {
        Tnode *new_node = NULL;
        merge(new_node, left, right->l);
        old = right;
        // tnode = new Tnode(*right);
        tnode = copy_specialized_tnode(right);
        tnode->l = new_node;
    }
    old->try_to_suicide();
    recompute_tnode_statistics(tnode, static_data[tnode->data_id].get());
}

void PS_Treap::insert(std::vector<std::unique_ptr<BaseSortedTreap>> &nodes, const std::vector<std::pair<common::SeqElem, uint64_t> > &seq_elems_with_prv) {
    if (seq_elems_with_prv.size() && nodes.size() != seq_elems_with_prv.size()) {
        Logger::error("internal error while trying to insert new nodes in the treap (not proper size for seq_prv array)");
    }

    // todo add this i inside the for.
    uint64_t i = 0;
    for (std::unique_ptr<BaseSortedTreap> &unique_node : nodes) {
        // Tnode *new_tnode = new Tnode(static_data.size());
        Tnode *new_tnode = create_new_specialized_tnode(static_data.size());
        static_data.push_back(std::move(unique_node));
        if (seq_elems_with_prv.size()) {
            prv_static_data.push_back(seq_elems_with_prv[i].second);
        } else {
            prv_static_data.push_back(ULLONG_MAX);
        }
        
        insert_tnode(root, new_tnode);
        assert(root != NULL);
        ++i;
    }
    // clear nodes, because everything was already destructed.
    nodes.clear();
}

void PS_Treap::insert_tnode(Tnode *&tnode, Tnode *to_add) {
    if (tnode == NULL) {
        tnode = to_add;
        recompute_tnode_statistics(tnode, static_data[tnode->data_id].get());
        return;
    }
    if (tnode->prio < to_add->prio) {
        split(tnode, *static_data[to_add->data_id], to_add->l, to_add->r);
        tnode = to_add;
    } else {
        // create a copy of node for persistency purpose.
        Tnode *old = tnode;
        // tnode = new Tnode(*tnode);
        tnode = copy_specialized_tnode(tnode);
        old->try_to_suicide();
        insert_tnode(*static_data[to_add->data_id] < *static_data[tnode->data_id] ? tnode->l : tnode->r, to_add);
    }
    assert(tnode != NULL);
    recompute_tnode_statistics(tnode, static_data[tnode->data_id].get());
}

Tnode* PS_Treap::find(const BaseSortedTreap &target) {
    Tnode *curr = root;
    while(curr != NULL) {
        if (*static_data[curr->data_id] < target) {
            curr = curr->r;
        } else if (target < *static_data[curr->data_id]) {
            curr = curr->l;
        } else {
            return curr;
        }
    }
    return NULL;
}

void PS_Treap::erase_node(Tnode *&tnode, const BaseSortedTreap &to_delete, int64_t &deleted_data_id) {
    if (tnode == NULL) {
        // The requested tnode does not exist in the treap.
        return;
    }

    Tnode *old;
    if (*static_data[tnode->data_id] < to_delete) {
        old = tnode;
        // tnode = new Tnode(*tnode);
        tnode = copy_specialized_tnode(tnode);
        erase_node(tnode->r, to_delete, deleted_data_id);
    } else if (to_delete < *static_data[tnode->data_id]) {
        old = tnode;
        // tnode = new Tnode(*tnode);
        tnode = copy_specialized_tnode(tnode);
        erase_node(tnode->l, to_delete, deleted_data_id);
    } else {
        old = tnode;
        deleted_data_id = tnode->data_id;
        Tnode *new_node = NULL;
        merge(new_node, tnode->l, tnode->r);
        tnode = new_node;
    }
    old->try_to_suicide();
    if (tnode != NULL) {
        recompute_tnode_statistics(tnode, static_data[tnode->data_id].get());
    }
}

void PS_Treap::erase(const std::vector<uint64_t> &nodes_indices) {
    for (const uint64_t &node_id : nodes_indices) {
        int64_t deleted_data_id = -1;
        erase_node(root, *static_data[node_id], deleted_data_id);
        
        if (deleted_data_id == -1) {
            throw std::runtime_error("ERROR -> cannot delete a value that doesn't exist in the treap");
        }
    }
}

void PS_Treap::run_tnode_callback(const Tnode *tnode, const std::function<void(const BaseSortedTreap &)> &callback) const{
    if (tnode == NULL) {
        return;
    }
    run_tnode_callback(tnode->l, callback);
    callback(*static_data[tnode->data_id]);
    run_tnode_callback(tnode->r, callback);
}

void PS_Treap::iterate_ordered(const std::function<void(const BaseSortedTreap &)> &callback, const std::string &snapshot) {
    if (snapshot == "") {
        run_tnode_callback(root, callback);
        return;
    } 
    auto it = date_to_root_idx.find(snapshot);
    if (it == date_to_root_idx.end()) {
        // todo test this line.
        throw std::runtime_error("ERROR -> query requested on a snapshot name that was not previously saved '" + snapshot + "'.");
    }
    run_tnode_callback(root_history[it->second], callback);
}

void PS_Treap::run_treap_query_callback_subtree(const std::string &target_key,Tnode *tnode, const ds::DB *db, query_ns::BaseQuery *query) const {
    if (tnode == NULL) {
        return;
    }
    query_ns::TreeDirectionToGo direction = query->first_enter_into_node(target_key, tnode, static_data[tnode->data_id].get(), db);
    
    if (direction == query_ns::NoSubtree) {
        return;
    }

    if (direction == query_ns::LeftChild) {
        run_treap_query_callback_subtree(target_key, tnode->l, db, query);
        direction = query->second_enter_into_node(target_key, tnode, static_data[tnode->data_id].get(), db); 
    }

    if (direction == query_ns::RightChild) {
        run_treap_query_callback_subtree(target_key, tnode->r, db, query);
    }
}

void PS_Treap::query_callback_subtree(query_ns::BaseQuery *query,
                                      const std::string &target_key,
                                      const ds::DB *db,
                                      const std::string &snapshot) const {
    if (snapshot == "") {
        run_treap_query_callback_subtree(target_key, root, db, query);
        return;
    }
    auto it = date_to_root_idx.find(snapshot);
    if (it == date_to_root_idx.end()) {
        // todo test this line.
        throw std::runtime_error("ERROR -> query requested on a snapshot name that was not previously saved '" + snapshot + "'.");
    }
    run_treap_query_callback_subtree(target_key, root_history[it->second], db, query);
}

void PS_Treap::delete_subtree(Tnode *&node) {
    if (node == NULL) {
        return;
    }
    delete_subtree(node->l);
    delete_subtree(node->r);
    delete node;
}

PS_Treap::~PS_Treap() {
    delete_subtree(root);
    static_data.clear(); // are the unique ptr released?
}

PS_Treap::PS_Treap(const std::function<void(Tnode *, const BaseSortedTreap*)> &req_recompute_tnode_statistics,
                   const std::function<Tnode*(const uint64_t)> &req_create_new_specialized_tnode,
                   const std::function<Tnode*(const Tnode*)> &req_copy_specialized_tnode){
    this->recompute_tnode_statistics = req_recompute_tnode_statistics;
    this->create_new_specialized_tnode = req_create_new_specialized_tnode;
    this->copy_specialized_tnode = req_copy_specialized_tnode;
}

// todo delete this 'treap_data' field.
PS_Treap::PS_Treap(const H5::Group &treap_group, 
                   std::vector<std::unique_ptr<BaseSortedTreap>> &treap_data,
                   const std::function<void(Tnode *, const BaseSortedTreap*)> &req_recompute_tnode_statistics,
                   const std::function<Tnode*(const uint64_t)> &req_create_new_specialized_tnode,
                   const std::function<Tnode*(const Tnode*)> &req_copy_specialized_tnode,
                   const std::function<Tnode*(const uint64_t, const uint64_t, const uint64_t)> &get_h5_tnode) {
    this->recompute_tnode_statistics = req_recompute_tnode_statistics;
    this->create_new_specialized_tnode = req_create_new_specialized_tnode;
    this->copy_specialized_tnode = req_copy_specialized_tnode;

    // deserialize static_data.
    for (std::unique_ptr<BaseSortedTreap> &unique_data : treap_data) {
        static_data.push_back(std::move(unique_data));
    }
    // clear treap_data, because everything was already destructed.
    treap_data.clear();

    // deserialize tnodes_to_save
    // todo use vector instead of hopscotch_map.
    tsl::hopscotch_map<uint64_t, Tnode*> h5id_to_tnodes;

    std::vector<uint64_t> tnode_data_id = H5Helper::read_h5_int_to_dataset<uint64_t>(treap_group, "data_id");
    std::vector<uint64_t> tnode_prio = H5Helper::read_h5_int_to_dataset<uint64_t>(treap_group, "prio");
    std::vector<int64_t> tnode_l = H5Helper::read_h5_int_to_dataset<int64_t>(treap_group, "saved_l");
    std::vector<int64_t> tnode_r = H5Helper::read_h5_int_to_dataset<int64_t>(treap_group, "saved_r");
    std::vector<uint64_t> tnode_index = H5Helper::read_h5_int_to_dataset<uint64_t>(treap_group, "index_tnode");

    assert(tnode_data_id.size() == tnode_prio.size());
    assert(tnode_data_id.size() == tnode_l.size());
    assert(tnode_data_id.size() == tnode_r.size());

    for (uint64_t i = 0; i < tnode_data_id.size(); ++i) {
        h5id_to_tnodes[i] = get_h5_tnode(tnode_data_id[i], tnode_prio[i], tnode_index[i]);
    }

    // add 'l' and 'r' fields;
    for (uint64_t i = 0; i < tnode_data_id.size(); ++i) {
        Tnode *t = h5id_to_tnodes.at(i);
        if (tnode_l[i] != -1) {
            auto it = h5id_to_tnodes.find(tnode_l[i]);
            if (it == h5id_to_tnodes.end()) {
                Logger::error("Broken hdf5 file, could not find a specific linked tnode.");
            }
            t->l = it->second;
        }
        if (tnode_r[i] != -1) {
            auto it = h5id_to_tnodes.find(tnode_r[i]);
            if (it == h5id_to_tnodes.end()) {
                Logger::error("Broken hdf5 file, could not find a specific linked tnode.");
            }
            t->r = it->second;
        }
    }
    // todo recompute all the tnodes in the tree.

    std::vector<uint64_t> saved_root_history = H5Helper::read_h5_int_to_dataset<uint64_t>(treap_group, "root_history");

    // deserialize root_history
    for (uint64_t i = 0; i < saved_root_history.size(); ++i) {
        if (saved_root_history[i] == ULLONG_MAX) {
            this->root_history.push_back(NULL);
            continue;
        }
        auto it = h5id_to_tnodes.find(saved_root_history[i]);
        if (it == h5id_to_tnodes.end()) {
            Logger::error("Broken hdf5 file, could not find a specific linked tnode.");
        }
        this->root_history.push_back(it->second);
    }

    std::vector<std::string> root_idx_key = H5Helper::read_h5_dataset(treap_group, "root_idx_key");
    std::vector<uint64_t> root_idx_value = H5Helper::read_h5_int_to_dataset<uint64_t>(treap_group, "root_idx_value");
    assert(root_idx_key.size() == root_idx_value.size());
    
    for (uint64_t i = 0; i < root_idx_value.size(); ++i) {
        date_to_root_idx[root_idx_key[i]] = root_idx_value[i];
    }

    // set root
    if (root_history.size()) {
        this->root = root_history.back();
    } else {
        this->root = NULL;
    }
    
}

PS_Treap::PS_Treap(PS_Treap *source) {
    this->recompute_tnode_statistics = source->recompute_tnode_statistics;
    this->create_new_specialized_tnode = source->create_new_specialized_tnode;
    this->copy_specialized_tnode = source->copy_specialized_tnode;

    for (uint64_t i = 0; i < source->static_data.size(); ++i) {
        this->static_data.push_back(std::move(source->static_data[i]));
    }

    this->root = source->root;
    this->date_to_root_idx = source->date_to_root_idx;
    this->root_history = source->root_history;

    // delete pointers from source after moving them to this.
    source->root = NULL;
    source->root_history.clear();
}

void PS_Treap::save_snapshot(const std::string &name) {
    Tnode::first_notsaved_index_tnode = Tnode::next_index_tnode;
    date_to_root_idx[name] = root_history.size();
    root_history.push_back(root);
}

void PS_Treap::export_to_hdf5(H5::Group &treap_group, 
                              const std::function<void(const std::vector<std::unique_ptr<BaseSortedTreap>> &, H5::Group &)> &write_static_data_to_h5,
                              const std::function<void(Tnode*)> &append_tnode_data) {
    if (root_history.size() && root != root_history.back()) {
        Logger::warn("The last root version was not saved");
    }
    tsl::hopscotch_set<Tnode*> tnodes_to_save;

    for (uint64_t i = 0; i < root_history.size(); ++i) {
        if (root_history[i] == NULL) {
            continue;
        }
        save_tnodes_in_subtree(tnodes_to_save, root_history[i]);
    }

    // serialize tnodes_to_save
    tsl::hopscotch_map<Tnode*, uint64_t> tnodes_to_h5id;

    uint64_t counter_saved_size = 0;
    for (Tnode* tnode : tnodes_to_save) {
        tnodes_to_h5id[tnode] = counter_saved_size++;
    }

    std::vector<uint64_t> saved_data_id;
    std::vector<uint64_t> saved_prio;
    std::vector<int64_t> saved_l;
    std::vector<int64_t> saved_r;
    std::vector<uint64_t> saved_tnode_index;

    for (Tnode* tnode : tnodes_to_save) {
        saved_data_id.push_back(tnode->data_id);
        saved_prio.push_back(tnode->prio);
        saved_tnode_index.push_back(tnode->index_tnode);

        append_tnode_data(tnode);

        // Set l and r fields.
        int64_t l_to_set = -1;
        if (tnode->l) {
            auto tnode_l_it = tnodes_to_h5id.find(tnode->l);
            assert(tnode_l_it != tnodes_to_h5id.end());
            l_to_set = tnode_l_it->second;
        } 
        int64_t r_to_set = -1;
        if (tnode->r) {
            auto tnode_r_it = tnodes_to_h5id.find(tnode->r);
            assert(tnode_r_it != tnodes_to_h5id.end());
            r_to_set = tnode_r_it->second;
        } 

        saved_l.push_back(l_to_set);
        saved_r.push_back(r_to_set);
    }

    H5Helper::write_h5_int_to_dataset(saved_data_id, &treap_group, "data_id");
    H5Helper::write_h5_int_to_dataset(saved_prio, &treap_group, "prio");
    H5Helper::write_h5_int_to_dataset(saved_l, &treap_group, "saved_l");
    H5Helper::write_h5_int_to_dataset(saved_r, &treap_group, "saved_r");
    H5Helper::write_h5_int_to_dataset(saved_tnode_index, &treap_group, "index_tnode");

    // serialize data
    write_static_data_to_h5(this->static_data, treap_group);

    std::vector<uint64_t> saved_root_history;

    // serialize root_history
    for (uint64_t i = 0; i < root_history.size(); ++i) {
        if (root_history[i] == NULL) {
            saved_root_history.push_back(ULLONG_MAX);
        } else {
            saved_root_history.push_back(tnodes_to_h5id[root_history[i]]);
        }
    }
    H5Helper::write_h5_int_to_dataset(saved_root_history, &treap_group, "root_history");

    std::vector<std::string> saved_root_idx_key;
    std::vector<uint64_t> saved_root_idx_values;
    for (const std::pair<std::string, uint64_t> &date_to_root_el : this->date_to_root_idx) {
        saved_root_idx_key.push_back(date_to_root_el.first);
        saved_root_idx_values.push_back(date_to_root_el.second);
    }
    H5Helper::write_h5_dataset(saved_root_idx_key, &treap_group, "root_idx_key");
    H5Helper::write_h5_int_to_dataset(saved_root_idx_values, &treap_group, "root_idx_value");
}

void PS_Treap::save_tnodes_in_subtree(tsl::hopscotch_set<Tnode*> &tnodes_to_save, Tnode *node) {
    if (node == NULL) {
        return;
    }

    auto it = tnodes_to_save.find(node);
    if (it != tnodes_to_save.end()) {
        // Current tnode was already saved, just return;
        return;
    }

    tnodes_to_save.insert(node);
    save_tnodes_in_subtree(tnodes_to_save, node->l);
    save_tnodes_in_subtree(tnodes_to_save, node->r);
}

std::vector<std::string> PS_Treap::get_snapshots_names() {
    std::vector<std::pair<std::string, uint64_t> > names_pair;
    for (const std::pair<std::string, uint64_t> &it : this->date_to_root_idx) {
        names_pair.push_back(it);
    }
    std::sort(names_pair.begin(), names_pair.end(), 
        [](const std::pair<std::string, uint64_t> &a, 
           const std::pair<std::string, uint64_t> &b) {
            return a.second < b.second;
        }
    );
    std::vector<std::string> names;
    for (const std::pair<std::string, uint64_t> &p : names_pair) {
        names.push_back(p.first);
    }
    return names;
}

uint64_t PS_Treap::get_data_size() {
    return this->static_data.size();
}

void PS_Treap::get_next_stack_tnode(std::vector<Tnode*> &stack) {
    assert(stack.size());
    assert(stack.back() != NULL);
    // Case 1 check if children on the right.
    if (stack.back()->r) {
        stack.push_back(NULL); // widlcard for going right.
        stack.push_back(stack[stack.size() - 2]->r);

        while (stack.back()->l != NULL) {
            stack.push_back(stack.back()->l);
        }
        return;
    }
    // Case 2, no children on the right.
    stack.pop_back();

    while (stack.size() && stack.back() == NULL) {
        // the last edge was a right child.
        stack.pop_back();
        assert(stack.size());
        stack.pop_back();
    }
    // if stack.empty -> we have finished with all the nodes.
    // if not empty -> we are returning an intermediary node after a left child.
}

void PS_Treap::get_differences(const PS_Treap *first_treap,
                     const PS_Treap *second_treap,
                     std::vector<uint64_t> &insertions_db_ids,
                     std::vector<uint64_t> &deletions_db_ids,
                     std::vector<std::pair<uint64_t, uint64_t>> &updates_db_ids) {
    std::vector<Tnode*> first_tnode_stack;
    std::vector<Tnode*> second_tnode_stack;
    
    if (first_treap->root == NULL) {
        Logger::error("requested differences for an empty treap");
    }

    first_tnode_stack.push_back(first_treap->root);

    while(first_tnode_stack.back()->l != NULL) {
        first_tnode_stack.push_back(first_tnode_stack.back()->l);
    }

    if (second_treap->root == NULL) {
        Logger::error("requested differences for an empty second treap");
    }

    second_tnode_stack.push_back(second_treap->root);
    while(second_tnode_stack.back()->l != NULL) {
        second_tnode_stack.push_back(second_tnode_stack.back()->l);
    }

    // Run mid-order traversal on both treaps simultaneously.
    while(first_tnode_stack.size() && second_tnode_stack.size()) {
        if (*first_treap->static_data[first_tnode_stack.back()->data_id] < *second_treap->static_data[second_tnode_stack.back()->data_id]) {
            deletions_db_ids.push_back(first_tnode_stack.back()->data_id);
            get_next_stack_tnode(first_tnode_stack);
        } else if (*second_treap->static_data[second_tnode_stack.back()->data_id] < *first_treap->static_data[first_tnode_stack.back()->data_id]) {
            insertions_db_ids.push_back(second_tnode_stack.back()->data_id);
            get_next_stack_tnode(second_tnode_stack);
        } else {
            // equal 
            AccessionIdSorted* first_elem = static_cast<AccessionIdSorted*>(first_treap->static_data[first_tnode_stack.back()->data_id].get());
            AccessionIdSorted* second_elem = static_cast<AccessionIdSorted*>(second_treap->static_data[second_tnode_stack.back()->data_id].get());

            if (first_elem->seq_hash != second_elem->seq_hash || first_elem->metadata_hash != second_elem->metadata_hash) {
                updates_db_ids.push_back({first_tnode_stack.back()->data_id, second_tnode_stack.back()->data_id});
            }
            get_next_stack_tnode(first_tnode_stack);
            get_next_stack_tnode(second_tnode_stack);
        }
    }
    while(first_tnode_stack.size()) {
        deletions_db_ids.push_back(first_tnode_stack.back()->data_id);
        get_next_stack_tnode(first_tnode_stack);
    }
    while(second_tnode_stack.size()) {
        insertions_db_ids.push_back(second_tnode_stack.back()->data_id);
        get_next_stack_tnode(second_tnode_stack);
    }
}

} //namespace ds;
