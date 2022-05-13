#pragma once

#include <memory>

#include "tnode_base.hpp"

#include "../static_types/static_base.hpp"

namespace treap_types {

class LocationTnode : public Tnode {
  public:
    uint64_t total_versions_in_subtree;
    uint64_t total_nodes_in_subtree;

    LocationTnode(const uint64_t rc_total_versions,
                  const uint64_t rc_total_nodes, 
                  const uint64_t rc_data_id, 
                  const uint64_t rc_prio, 
                  const uint64_t rc_index_tnode = 0xffffffff);
    LocationTnode(const LocationTnode *oth);
    LocationTnode(const uint64_t index);

    static std::vector<uint64_t> saved_total_versions_subtree;
    static std::vector<uint64_t> saved_total_nodes_subtree;

    static uint64_t next_h5_bp_index;
    static Tnode* get_h5_tnode(const uint64_t req_data_id, const unsigned long long req_prio, const uint64_t req_index);
    static void reset_get_h5_tnode(const H5::Group &group);

    static void append_tnode_data(Tnode *tnode);
    static void reset_append_tnode_data();
    static void write_tnode_data_to_h5(H5::Group &group);

    static Tnode* create_new_specialized_tnode(const uint64_t data_id);
    static Tnode* copy_specialized_tnode(const Tnode* oth);
};

void recompute_location_statistics(Tnode *rec_tnode, const BaseSortedTreap* elem_base);

} // namespace treap_types