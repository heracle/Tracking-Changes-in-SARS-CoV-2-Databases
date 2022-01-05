#pragma once

#include "static_base.hpp"
#include "../../common/utils.hpp"
#include "H5Cpp.h"

namespace treap_types {

class AccessionIdSorted : public BaseSortedTreap {
  public:
    std::uint64_t seq_hash;
    std::uint64_t metadata_hash;

    AccessionIdSorted(
        const std::string &req_key, 
        const uint32_t req_database_id,
        const int64_t req_seq_hash, 
        const int64_t req_metadata_hash
    );
    AccessionIdSorted(const H5::Group &h5_group);
    ~AccessionIdSorted();

    static std::unique_ptr<BaseSortedTreap> get_unique_from_SeqElem(const common::SeqElem &e, const uint32_t req_database_id);
};

void recompute_acc_id_statistics(common::Tnode *, const std::unique_ptr<BaseSortedTreap> &);
void append_acc_id_elem_to_hdf5(const std::unique_ptr<BaseSortedTreap> &elem_unique, H5::Group &elem_group);


} // namespace treap_types