#include "tnode_location.hpp"

#include <memory>
#include <iostream>

#include "../static_types/static_base.hpp"
#include "../static_types/static_location.hpp"

namespace treap_types {

LocationTnode::LocationTnode(const LocationTnode *oth) : Tnode(static_cast<const Tnode*>(oth)) {
    this->altered_bp = oth->altered_bp;
}

LocationTnode::LocationTnode(const std::vector<std::pair<uint32_t, uint32_t> > &rc_altered_bp, const uint32_t rc_data_id, const uint64_t rc_prio, const uint32_t rc_index_tnode = 0xffffffff)
: Tnode(rc_data_id, rc_prio, rc_index_tnode) {
    this->altered_bp = rc_altered_bp;
}

LocationTnode::LocationTnode(const uint32_t index) : Tnode(index) {
    // this->altered_bp = std::vector<std::pair<uint32_t, uint32_t>>();
}

void recompute_location_statistics(Tnode *rec_tnode, const BaseSortedTreap* elem_base) {
    LocationTnode *tnode = static_cast<LocationTnode*>(rec_tnode); 
    const LocationSorted* elem = static_cast<const LocationSorted*>(elem_base);
    tnode->altered_bp = elem->bp_alterations;
}

std::vector<uint32_t> LocationTnode::saved_altered_bp;
uint32_t LocationTnode::next_altered_bp_index;

Tnode* LocationTnode::get_h5_tnode(const uint32_t req_data_id, const unsigned long long req_prio, const uint32_t req_index) {
    uint32_t altered_bp_size = LocationTnode::saved_altered_bp[next_altered_bp_index++];
    std::vector<std::pair<uint32_t, uint32_t> > curr_altered_bp;
    for (uint32_t j = 0; j < altered_bp_size; ++j) {
        std::pair<uint32_t, uint32_t> act = {saved_altered_bp[next_altered_bp_index], saved_altered_bp[next_altered_bp_index + 1]};
        next_altered_bp_index += 2;
        curr_altered_bp.push_back(act);
    }
    LocationTnode *tnode = new LocationTnode(curr_altered_bp, req_data_id, req_prio, req_index);
    return static_cast<Tnode*>(tnode);
}

void LocationTnode::reset_get_h5_tnode(const H5::Group &group) {
    LocationTnode::saved_altered_bp = H5Helper::read_h5_int_to_dataset<uint32_t>(group, "altered_bp");
    LocationTnode::next_altered_bp_index = 0;
}

void LocationTnode::append_tnode_data(Tnode *rec_tnode) {
    LocationTnode *tnode = static_cast<LocationTnode*>(rec_tnode);
    LocationTnode::saved_altered_bp.push_back(tnode->altered_bp.size());
    for(uint32_t i = 0; i < tnode->altered_bp.size(); ++i) {
        LocationTnode::saved_altered_bp.push_back(tnode->altered_bp[i].first);
        LocationTnode::saved_altered_bp.push_back(tnode->altered_bp[i].second);
    }
}

void LocationTnode::reset_append_tnode_data() {
    LocationTnode::saved_altered_bp.clear();
}

void LocationTnode::write_tnode_data_to_h5(H5::Group &group) {
    H5Helper::write_h5_int_to_dataset(LocationTnode::saved_altered_bp, &group, "altered_bp");
}

Tnode* LocationTnode::create_new_specialized_tnode(const uint32_t data_id) {
    LocationTnode *curr = new LocationTnode(data_id);
    return static_cast<Tnode*>(curr);
}

Tnode* LocationTnode::copy_specialized_tnode(const Tnode* oth) {
    LocationTnode *curr = new LocationTnode(static_cast<const LocationTnode*>(oth));
    return static_cast<Tnode*>(curr);
}

} // namespace treap_types