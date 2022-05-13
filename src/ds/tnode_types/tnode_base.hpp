#pragma once

#include <cstdint>
#include <vector>
#include <random>

#include "../common/h5_helper.hpp"

namespace treap_types {

class Tnode {
  //todo change this "public"
  public:
    uint32_t data_id;
    unsigned long long prio;
    uint32_t index_tnode;

    static const int rand_pool_size = (1 << 16);
    static int next_rand_idx;
    static std::vector<unsigned long long> rand_values;
    static uint64_t next_index_tnode;
    static uint64_t first_notsaved_index_tnode;

    static unsigned long long get_rand_ull();

    Tnode *l;
    Tnode *r;

    // uint32_t total_mutations;
    // tsl::hopscotch_map<std::string, uint32_t> mutation_freq;
    
    Tnode(const uint32_t index);
    Tnode(const Tnode *oth);
    Tnode(const uint32_t rc_data_id, const uint64_t rc_prio, const uint32_t rc_index_tnode);
    
    // This constructor is not initializing 'r' and 'l' fields!
    // Tnode(const H5::Group &tnode_group);

    // https://isocpp.org/wiki/faq/freestore-mgmt#delete-this
    void try_to_suicide();
};

} // namespace treap_types