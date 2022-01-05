#pragma once

#include "static_base.hpp"
#include "../../common/utils.hpp"

namespace treap_types {

class LocationSorted : public BaseSortedTreap {
  public:
    std::string mutations;

    LocationSorted(const std::string &req_key, const uint32_t req_database_id, const std::string &req_mutation);
    LocationSorted(const H5::Group &h5_group);
    ~LocationSorted();

    static std::unique_ptr<BaseSortedTreap> get_unique_from_SeqElem(const common::SeqElem &e, const uint32_t req_database_id);
};

void recompute_location_statistics(common::Tnode *tnode, const std::unique_ptr<BaseSortedTreap> &elem_unique);
void append_location_elem_to_hdf5(const std::unique_ptr<BaseSortedTreap> &elem_unique, H5::Group &elem_group);

} // namespace treap_types