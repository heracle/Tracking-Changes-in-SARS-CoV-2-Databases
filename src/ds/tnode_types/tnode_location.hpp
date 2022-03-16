#pragma once

#include <memory>

#include "tnode_base.hpp"

#include "../static_types/static_base.hpp"

namespace treap_types {

class LocationTnode : public Tnode {
  public:    
    // todo, check if we can use uint16_t instead
    std::vector<std::pair<uint32_t, uint32_t>> altered_bp;

    LocationTnode(const std::vector<std::pair<uint32_t, uint32_t> > &rc_altered_bp, const uint32_t rc_data_id, const uint64_t rc_prio, const uint32_t rc_index_tnode);
    // This constructor is not initializing 'r' and 'l' fields!
    // LocationTnode(const H5::Group &tnode_group);
    LocationTnode(const LocationTnode *oth);
    LocationTnode(const uint32_t index);

    static std::vector<uint32_t> saved_altered_bp;

    static uint32_t next_altered_bp_index;
    static Tnode* get_h5_tnode(const uint32_t req_data_id, const unsigned long long req_prio, const uint32_t req_index);
    static void reset_get_h5_tnode(const H5::Group &group);

    static void append_tnode_data(Tnode *tnode);
    static void reset_append_tnode_data();
    static void write_tnode_data_to_h5(H5::Group &group);

    static Tnode* create_new_specialized_tnode(const uint32_t data_id);
    static Tnode* copy_specialized_tnode(const Tnode* oth);
};

void recompute_location_statistics(Tnode *rec_tnode, const BaseSortedTreap* elem_base);

} // namespace treap_types