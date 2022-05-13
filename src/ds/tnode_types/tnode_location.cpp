#include "tnode_location.hpp"

#include <memory>
#include <iostream>

#include "../static_types/static_base.hpp"
#include "../static_types/static_location.hpp"

namespace treap_types {

LocationTnode::LocationTnode(const LocationTnode *oth) : Tnode(static_cast<const Tnode*>(oth)) {
    this->total_versions_in_subtree = oth->total_versions_in_subtree;
    this->total_nodes_in_subtree = oth->total_nodes_in_subtree;
}

LocationTnode::LocationTnode(const uint64_t rc_total_versions,
                             const uint64_t rc_total_nodes, 
                             const uint64_t rc_data_id, 
                             const uint64_t rc_prio, 
                             const uint64_t rc_index_tnode)
: Tnode(rc_data_id, rc_prio, rc_index_tnode) {
    this->total_versions_in_subtree = rc_total_versions;
    this->total_nodes_in_subtree = rc_total_nodes;
}

LocationTnode::LocationTnode(const uint64_t index) : Tnode(index) {
    this->total_versions_in_subtree = 0;
    this->total_nodes_in_subtree = 0;
}

void recompute_location_statistics(Tnode *rec_tnode, const BaseSortedTreap* elem_base) {
    LocationTnode *tnode = static_cast<LocationTnode*>(rec_tnode);
    const LocationSorted* elem = static_cast<const LocationSorted*>(elem_base);

    tnode->total_nodes_in_subtree = 1;
    tnode->total_versions_in_subtree = elem->num_sequence_versions;

    if (tnode->l != NULL) {
        LocationTnode *left = static_cast<LocationTnode*>(tnode->l);
        tnode->total_nodes_in_subtree += left->total_nodes_in_subtree;
        tnode->total_versions_in_subtree += left->total_versions_in_subtree;
    }
    if (tnode->r != NULL) {
        LocationTnode *right = static_cast<LocationTnode*>(tnode->r);
        tnode->total_nodes_in_subtree += right->total_nodes_in_subtree;
        tnode->total_versions_in_subtree += right->total_versions_in_subtree;
    }
}

uint64_t LocationTnode::next_h5_bp_index;
std::vector<uint64_t> LocationTnode::saved_total_versions_subtree;
std::vector<uint64_t> LocationTnode::saved_total_nodes_subtree;

Tnode* LocationTnode::get_h5_tnode(const uint64_t req_data_id, const unsigned long long req_prio, const uint64_t req_index) {
    LocationTnode *tnode = new LocationTnode(saved_total_versions_subtree[next_h5_bp_index],
                                             saved_total_nodes_subtree[next_h5_bp_index],
                                             req_data_id, req_prio, req_index);
    ++next_h5_bp_index;
    return static_cast<Tnode*>(tnode);
}

void LocationTnode::reset_get_h5_tnode(const H5::Group &group) {
    LocationTnode::next_h5_bp_index = 0;
    LocationTnode::saved_total_versions_subtree = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "total_versions_subtree");
    LocationTnode::saved_total_nodes_subtree = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "total_nodes_subtree");
}

void LocationTnode::append_tnode_data(Tnode *rec_tnode) {
    LocationTnode *tnode = static_cast<LocationTnode*>(rec_tnode);
    LocationTnode::saved_total_versions_subtree.push_back(tnode->total_versions_in_subtree);
    LocationTnode::saved_total_nodes_subtree.push_back(tnode->total_nodes_in_subtree);
}

void LocationTnode::reset_append_tnode_data() {
    LocationTnode::saved_total_versions_subtree.clear();
    LocationTnode::saved_total_nodes_subtree.clear();
}

void LocationTnode::write_tnode_data_to_h5(H5::Group &group) {
    H5Helper::write_h5_int_to_dataset(LocationTnode::saved_total_versions_subtree, &group, "total_versions_subtree");
    H5Helper::write_h5_int_to_dataset(LocationTnode::saved_total_nodes_subtree, &group, "total_nodes_subtree");
}

Tnode* LocationTnode::create_new_specialized_tnode(const uint64_t data_id) {
    LocationTnode *curr = new LocationTnode(data_id);
    return static_cast<Tnode*>(curr);
}

Tnode* LocationTnode::copy_specialized_tnode(const Tnode* oth) {
    LocationTnode *curr = new LocationTnode(static_cast<const LocationTnode*>(oth));
    return static_cast<Tnode*>(curr);
}

} // namespace treap_types