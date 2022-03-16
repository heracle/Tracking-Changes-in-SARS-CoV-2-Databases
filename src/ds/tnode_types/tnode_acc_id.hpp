#pragma once

#include <memory>

#include "tnode_base.hpp"

#include "../static_types/static_base.hpp"

namespace treap_types {

class AccIdTnode : public Tnode {
  public:
    AccIdTnode(const uint32_t rc_data_id, const uint64_t rc_prio, const uint32_t rc_index_tnode);
    AccIdTnode(const AccIdTnode *oth);
    // todo rename this 'index' to 'data_id'!
    AccIdTnode(const uint32_t index);

    static Tnode* get_h5_tnode(const uint32_t req_data_id, const unsigned long long req_prio, const uint32_t req_index);
    static void reset_get_h5_tnode(const H5::Group &);

    static void append_tnode_data(Tnode *tnode);
    static void reset_append_tnode_data();
    static void write_tnode_data_to_h5(H5::Group &group);

    static Tnode* create_new_specialized_tnode(const uint32_t data_id);
    static Tnode* copy_specialized_tnode(const Tnode* oth);
};

void recompute_acc_id_statistics(Tnode *, const BaseSortedTreap*);

} // namespace treap_types