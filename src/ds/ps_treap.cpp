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

#include "../ds/treap_representation/static_acc_id.hpp"

using namespace treap_types;

namespace ds {

void PS_Treap::split(common::Tnode *&tnode, const BaseSortedTreap &key, common::Tnode *&left, common::Tnode *&right) {
    if (tnode == NULL) {
        left = right = NULL;
        return;
    }
    // create a copy of node for persistency purpose.
    tnode = new common::Tnode(*tnode);

    if (key < *static_data[tnode->data_id]) {
        common::Tnode *new_left = NULL;
        split(tnode->l, key, left, new_left);
        tnode->l = new_left;
        right = tnode;
    } else {
        common::Tnode *new_right = NULL;
        split(tnode->r, key, new_right, right);
        tnode->r = new_right;
        left = tnode;
    }
    recompute_tnode_statistics(tnode, static_data[tnode->data_id]);
}

void PS_Treap::merge(common::Tnode *&tnode, common::Tnode *left, common::Tnode *right) {
    if (left == NULL || right == NULL) {
        tnode = (left != NULL ? left : right);
        return;
    }
    if (left->prio > right->prio) {
        common::Tnode *new_node = NULL;
        merge(new_node, left->r, right);
        tnode = new common::Tnode(*left);
        tnode->r = new_node;
    } else {
        common::Tnode *new_node = NULL;
        merge(new_node, left, right->l);
        tnode = new common::Tnode(*right);
        tnode->l = new_node;
    }
    recompute_tnode_statistics(tnode, static_data[tnode->data_id]);
}

void PS_Treap::insert(std::vector<std::unique_ptr<BaseSortedTreap>> &nodes) {
    for (std::unique_ptr<BaseSortedTreap> &unique_node : nodes) {
        common::Tnode *new_tnode = new common::Tnode(static_data.size());
        static_data.push_back(std::move(unique_node));
        insert_tnode(root, new_tnode);
        assert(root != NULL);
    }
    // clear nodes, because everything was already destructed.
    nodes.clear();
}

void PS_Treap::insert_tnode(common::Tnode *&tnode, common::Tnode *to_add) {
    if (tnode == NULL) {
        tnode = to_add;
        recompute_tnode_statistics(tnode, static_data[tnode->data_id]);
        return;
    }
    if (tnode->prio < to_add->prio) {
        split(tnode, *static_data[to_add->data_id], to_add->l, to_add->r);
        tnode = to_add;
    } else {
        // create a copy of node for persistency purpose.
        tnode = new common::Tnode(*tnode);
        insert_tnode(*static_data[to_add->data_id] < *static_data[tnode->data_id] ? tnode->l : tnode->r, to_add);
    }
    assert(tnode != NULL);
    recompute_tnode_statistics(tnode, static_data[tnode->data_id]);
}

common::Tnode* PS_Treap::find(const BaseSortedTreap &target) {
    common::Tnode *curr = root;
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

void PS_Treap::erase_node(common::Tnode *&tnode, const BaseSortedTreap &to_delete, int &deleted_data_id) {
    if (tnode == NULL) {
        // The requested tnode does not exist in the treap.
        return;
    }

    if (*static_data[tnode->data_id] < to_delete) {
        tnode = new common::Tnode(*tnode);
        erase_node(tnode->r, to_delete, deleted_data_id);
    } else if (to_delete < *static_data[tnode->data_id]) {
        tnode = new common::Tnode(*tnode);
        erase_node(tnode->l, to_delete, deleted_data_id);
    } else {
        deleted_data_id = tnode->data_id;
        common::Tnode *new_node = NULL;
        merge(new_node, tnode->l, tnode->r);
        tnode = new_node;
    }
    if (tnode != NULL) {
        recompute_tnode_statistics(tnode, static_data[tnode->data_id]);
    }
}

void PS_Treap::erase(const std::vector<uint32_t> &nodes_indices) {
    for (const uint32_t &node_id : nodes_indices) {
        int deleted_data_id = -1;
        erase_node(root, *static_data[node_id], deleted_data_id);
        
        if (deleted_data_id == -1) {
            throw std::runtime_error("ERROR -> cannot delete a value that doesn't exist in the treap");
        }
    }
}

void PS_Treap::run_tnode_callback(const common::Tnode *tnode, const std::function<void(const BaseSortedTreap &)> &callback) {
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

void PS_Treap::run_treap_query_callback_subtree(common::Tnode *tnode, 
                                                const std::function<int(common::Tnode *, const BaseSortedTreap *)> &callback_fst,
                                                const std::function<int(common::Tnode *, const BaseSortedTreap *)> &callback_scd) {
    if (tnode == NULL) {
        return;
    }
    int direction = callback_fst(tnode, static_data[tnode->data_id].get());
    // direction == -1 -> return
    // direction == 0 -> go to the left child
    // direction == 1 -> go directly to the right child
    if (direction == -1) {
        return;
    }

    if (direction == 0) {
        run_treap_query_callback_subtree(tnode->l, callback_fst, callback_scd);
        direction = callback_scd(tnode, static_data[tnode->data_id].get()); 
    }

    if (direction == 1) {
        run_treap_query_callback_subtree(tnode->r, callback_fst, callback_scd);
    }
}

void PS_Treap::query_callback_subtree(const std::function<int(common::Tnode *, const BaseSortedTreap *)> &callback_fst,
                                      const std::function<int(common::Tnode *, const BaseSortedTreap *)> &callback_scd,
                                      const std::string &snapshot) {
    if (snapshot == "") {
        run_treap_query_callback_subtree(root, callback_fst, callback_scd);
        return;
    } 
    auto it = date_to_root_idx.find(snapshot);
    if (it == date_to_root_idx.end()) {
        // todo test this line.
        throw std::runtime_error("ERROR -> query requested on a snapshot name that was not previously saved '" + snapshot + "'.");
    }
    run_treap_query_callback_subtree(root_history[it->second], callback_fst, callback_scd);
}

void PS_Treap::delete_subtree(common::Tnode *&node) {
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

PS_Treap::PS_Treap(const std::function<void(common::Tnode *, const std::unique_ptr<BaseSortedTreap>&)> &req_recompute_tnode_statistics){
    this->recompute_tnode_statistics = req_recompute_tnode_statistics;
}

// todo delete this 'treap_data' field.
PS_Treap::PS_Treap(const H5::Group &treap_group, 
                   std::vector<std::unique_ptr<BaseSortedTreap>> &treap_data,
                   const std::function<void(common::Tnode *, const std::unique_ptr<BaseSortedTreap>&)> &req_recompute_tnode_statistics) {
    this->recompute_tnode_statistics = req_recompute_tnode_statistics;

    // deserialize static_data.
    for (std::unique_ptr<BaseSortedTreap> &unique_data : treap_data) {
        static_data.push_back(std::move(unique_data));
    }
    // clear treap_data, because everything was already destructed.
    treap_data.clear();

    // deserialize tnodes_to_save
    // todo use vector instead of hopscotch_map.
    tsl::hopscotch_map<uint32_t, common::Tnode*> h5id_to_tnodes;

    uint32_t num_tnodes = H5Helper::get_uint32_attr_from(treap_group, "tnodes_size");
    for (uint32_t i = 0; i < num_tnodes; ++i) {
        H5::Group group_tnode = H5Gopen(treap_group.getLocId(), ("tnode" + std::to_string(i)).c_str(), H5P_DEFAULT);
        h5id_to_tnodes[i] = new common::Tnode(group_tnode);
        group_tnode.close();
    }

    // add 'l' and 'r' fields;
    for (uint32_t i = 0; i < num_tnodes; ++i) {
        H5::Group group_tnode = H5Gopen(treap_group.getLocId(), ("tnode" + std::to_string(i)).c_str(), H5P_DEFAULT);
        int32_t l = H5Helper::get_int32_attr_from(group_tnode, "l");
        int32_t r = H5Helper::get_int32_attr_from(group_tnode, "r");
        group_tnode.close();

        common::Tnode *t = h5id_to_tnodes.at(i);
        if (l != -1) {
            auto it = h5id_to_tnodes.find(l);
            if (it == h5id_to_tnodes.end()) {
                Logger::error("Broken hdf5 file, could not find a specific linked tnode.");
            }
            t->l = it->second;
        }
        if (r != -1) {
            auto it = h5id_to_tnodes.find(r);
            if (it == h5id_to_tnodes.end()) {
                Logger::error("Broken hdf5 file, could not find a specific linked tnode.");
            }
            t->r = it->second;
        }
    }

    // deserialize root_history
    uint32_t root_history_size = H5Helper::get_uint32_attr_from(treap_group, "root_history_size");
    for (uint32_t i = 0; i < root_history_size; ++i) {
        uint32_t root_history_id = H5Helper::get_uint32_attr_from(treap_group, ("root_history" + std::to_string(i)).c_str());
        auto it = h5id_to_tnodes.find(root_history_id);
        if (it == h5id_to_tnodes.end()) {
            Logger::error("Broken hdf5 file, could not find a specific linked tnode.");
        }
        this->root_history.push_back(it->second);
    }

    // serialize date_to_root_idx.
    uint32_t size_date_to_root = H5Helper::get_uint32_attr_from(treap_group, "date_to_root_size");

    // if (treap.date_to_root_idx_key_size() != treap.date_to_root_idx_value_size()) {
    //     Logger::error("Broken hdf5 file, date_to_root_idx 'key' and 'value' have different sizes.");
    // }
    // uint32_t size_date_to_root = treap.date_to_root_idx_key_size();
    for (uint32_t i = 0; i < size_date_to_root; ++i) {
        date_to_root_idx[H5Helper::get_string_attr_from(treap_group, "root_idx_key" + std::to_string(i))] =
                            H5Helper::get_uint64_attr_from(treap_group, "root_idx_value" + std::to_string(i));
    }

    // set root
    this->root = root_history.back();
}

void PS_Treap::save_snapshot(const std::string &name) {
    date_to_root_idx[name] = root_history.size();
    root_history.push_back(root); 
}

void PS_Treap::export_to_hdf5(H5::Group &treap_group, const std::function<void(const std::unique_ptr<BaseSortedTreap> &, H5::Group &)> &h5_append_elem) {    
    if (root != root_history.back()) {
        Logger::warn("The last root version was not saved");
    }
    tsl::hopscotch_set<common::Tnode*> tnodes_to_save;

    for (uint32_t i = 0; i < root_history.size(); ++i) {
        save_tnodes_in_subtree(tnodes_to_save, root_history[i]);
    }

    // serialize tnodes_to_save
    tsl::hopscotch_map<common::Tnode*, uint32_t> tnodes_to_h5id;

    uint32_t counter_saved_size = 0;
    for (common::Tnode* tnode : tnodes_to_save) {
        tnodes_to_h5id[tnode] = counter_saved_size++;
    }

    H5Helper::set_uint32_hdf5_attr(tnodes_to_save.size(), &treap_group, "tnodes_size");
    uint32_t cnt = 0;
    for (common::Tnode* tnode : tnodes_to_save) {
        H5::Group* group_tnode = new H5::Group(treap_group.createGroup(("tnode" + std::to_string(cnt)).c_str()));
        cnt++;

        H5Helper::set_uint32_hdf5_attr(tnode->data_id, group_tnode, "data_id");
        H5Helper::set_uint64_hdf5_attr(tnode->prio, group_tnode, "prio");

        // Set l and r fields.
        int32_t l_to_set = -1;
        if (tnode->l) {
            auto tnode_l_it = tnodes_to_h5id.find(tnode->l);
            assert(tnode_l_it != tnodes_to_h5id.end());
            l_to_set = tnode_l_it->second;
        } 
        int32_t r_to_set = -1;
        if (tnode->r) {
            auto tnode_r_it = tnodes_to_h5id.find(tnode->r);
            assert(tnode_r_it != tnodes_to_h5id.end());
            r_to_set = tnode_r_it->second;
        } 

        H5Helper::set_int32_hdf5_attr(l_to_set, group_tnode, "l");
        H5Helper::set_int32_hdf5_attr(r_to_set, group_tnode, "r");

        group_tnode->close();
    }
    H5Helper::set_uint32_hdf5_attr(this->static_data.size(), &treap_group, "static_data_size");
    // serialize data
    for (uint32_t i = 0; i < this->static_data.size(); ++i) {
        // append_elem_data(elem, proto);
        H5::Group* g1 = new H5::Group(treap_group.createGroup(("static_data" + std::to_string(i)).c_str()));
        h5_append_elem(this->static_data[i], *g1);
        g1->close();
    }

    // serialize root_history
    H5Helper::set_uint32_hdf5_attr(this->root_history.size(), &treap_group, "root_history_size");
    for (uint32_t i = 0; i < root_history.size(); ++i) {
        H5Helper::set_uint32_hdf5_attr(tnodes_to_h5id[root_history[i]], &treap_group, "root_history" + std::to_string(i));
    }

    // serialize date_to_root_idx.
    H5Helper::set_uint32_hdf5_attr(this->date_to_root_idx.size(), &treap_group, "date_to_root_size");
    cnt = 0;
    for (const std::pair<std::string, uint32_t> &date_to_root_el : this->date_to_root_idx) {
        H5Helper::set_string_hdf5_attr(date_to_root_el.first, &treap_group, "root_idx_key" + std::to_string(cnt));
        H5Helper::set_uint32_hdf5_attr(date_to_root_el.second, &treap_group, "root_idx_value" + std::to_string(cnt));
        cnt++;
    }
}

void PS_Treap::save_tnodes_in_subtree(tsl::hopscotch_set<common::Tnode*> &tnodes_to_save, common::Tnode *node) {
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
    std::vector<std::pair<std::string, uint32_t> > names_pair;
    for (const std::pair<std::string, uint32_t> &it : this->date_to_root_idx) {
        names_pair.push_back(it);
    }
    std::sort(names_pair.begin(), names_pair.end(), 
        [](const std::pair<std::string, uint32_t> &a, 
           const std::pair<std::string, uint32_t> &b) {
            return a.second < b.second;
        }
    );
    std::vector<std::string> names;
    for (const std::pair<std::string, uint32_t> &p : names_pair) {
        names.push_back(p.first);
    }
    return names;
}

uint32_t PS_Treap::get_data_size() {
    return this->static_data.size();
}

void PS_Treap::get_next_stack_tnode(std::vector<common::Tnode*> &stack) {
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
                     std::vector<uint32_t> &insertions_db_ids,
                     std::vector<uint32_t> &deletions_db_ids,
                     std::vector<std::pair<uint32_t, uint32_t>> &updates_db_ids) {
    std::vector<common::Tnode*> first_tnode_stack;
    std::vector<common::Tnode*> second_tnode_stack;
    
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
