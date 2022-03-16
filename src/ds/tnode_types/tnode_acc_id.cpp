#include "tnode_acc_id.hpp"

#include <memory>

#include "../static_types/static_base.hpp"

namespace treap_types {

void recompute_acc_id_statistics(Tnode *, const BaseSortedTreap*) {}

AccIdTnode::AccIdTnode(const AccIdTnode *oth) : Tnode(static_cast<const Tnode*>(oth)) {}

AccIdTnode::AccIdTnode(const uint32_t rc_data_id, const uint64_t rc_prio, const uint32_t rc_index_tnode = 0xffffffff)
    : Tnode(rc_data_id, rc_prio, rc_index_tnode) {}

AccIdTnode::AccIdTnode(const uint32_t index) : Tnode(index) {}

void AccIdTnode::reset_get_h5_tnode(const H5::Group &) {}

Tnode* AccIdTnode::get_h5_tnode(const uint32_t req_data_id, const unsigned long long req_prio, const uint32_t req_index) {
    AccIdTnode *tnode = new AccIdTnode(req_data_id, req_prio, req_index);
    return static_cast<Tnode*>(tnode);
}

void AccIdTnode::append_tnode_data(Tnode*) {}
void AccIdTnode::reset_append_tnode_data() {}
void AccIdTnode::write_tnode_data_to_h5(H5::Group &) {}

Tnode* AccIdTnode::create_new_specialized_tnode(const uint32_t data_id) {
    AccIdTnode *curr = new AccIdTnode(data_id);
    return static_cast<Tnode*>(curr);
}

Tnode* AccIdTnode::copy_specialized_tnode(const Tnode* oth) {
    AccIdTnode *curr = new AccIdTnode(static_cast<const AccIdTnode*>(oth));
    return static_cast<Tnode*>(curr);
}

} // namespace treap_types







