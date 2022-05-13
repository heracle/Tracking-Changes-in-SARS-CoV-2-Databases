#pragma once

#include <cstdint>
#include <vector>
#include <random>

#include "../common/h5_helper.hpp"

namespace treap_types {

class Tnode {
  //todo change this "public"
  public:
    uint64_t data_id;
    unsigned long long prio;
    uint64_t index_tnode;

    static const int64_t rand_pool_size = (1 << 16);
    static int64_t next_rand_idx;
    static std::vector<unsigned long long> rand_values;
    static uint64_t next_index_tnode;
    static uint64_t first_notsaved_index_tnode;

    static unsigned long long get_rand_ull();

    Tnode *l;
    Tnode *r;
    
    Tnode(const uint64_t index);
    Tnode(const Tnode *oth);
    Tnode(const uint64_t rc_data_id, const uint64_t rc_prio, const uint64_t rc_index_tnode);

    // https://isocpp.org/wiki/faq/freestore-mgmt#delete-this
    void try_to_suicide();
};

} // namespace treap_types