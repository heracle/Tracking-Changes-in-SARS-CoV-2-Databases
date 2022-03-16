#include "tnode_base.hpp"

#include "../common/logger.hpp"

#include <iostream>

namespace treap_types {

int Tnode::next_rand_idx = rand_pool_size;
std::vector<unsigned long long> Tnode::rand_values = std::vector<unsigned long long>(Tnode::rand_pool_size);
uint32_t Tnode::next_index_tnode = 0;
uint32_t Tnode::first_notsaved_index_tnode = 0;

Tnode::Tnode(const uint32_t index) : data_id(index), prio(get_rand_ull()), l(NULL), r(NULL) {
    this->index_tnode = next_index_tnode++;
};

Tnode::Tnode(const Tnode *oth) : data_id(oth->data_id), prio(oth->prio), l(oth->l), r(oth->r) {
    this->index_tnode = next_index_tnode++;
};

Tnode::Tnode(const uint32_t rc_data_id, const uint64_t rc_prio, const uint32_t rc_index_tnode = 0xffffffff) : data_id(rc_data_id), prio(rc_prio), l(NULL), r(NULL) {
    if (rc_index_tnode == 0xffffffff) {
        this->index_tnode = next_index_tnode++;
    } else {
        this->index_tnode = rc_index_tnode;
    }
};

// todo, first_notsaved_index_tnode is a static variable, could be issues if we want to save only one treap for a snapshot.
void Tnode::try_to_suicide() {
    if (this->index_tnode >= first_notsaved_index_tnode) {
        delete this;
    }
}

unsigned long long Tnode::get_rand_ull() {
    if (next_rand_idx == rand_pool_size) {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> unif_dist(1, 0xFFFFFFFFFFFFFFFF);
        for (unsigned int i = 0; i < rand_pool_size; ++i) {
            rand_values[i] = unif_dist(rng);
        }
        next_rand_idx = 0;
    }
    return rand_values[next_rand_idx++];
}

} // namespace treap_types