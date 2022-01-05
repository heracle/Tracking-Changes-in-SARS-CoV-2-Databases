#pragma once

#include <string>
#include <memory>

#include "../../common/utils.hpp"
#include "H5Cpp.h"

namespace treap_types {

class BaseSortedTreap {
  public:
    std::string key;
    uint32_t database_id;

    bool operator < (const BaseSortedTreap &oth) const;

    BaseSortedTreap(const std::string &req_key, const uint32_t req_database_id);
    BaseSortedTreap(const H5::Group &h5_group);

    // todo do we need a virtual destructor?
    virtual ~BaseSortedTreap();

    // virtual std::unique_ptr<BaseSortedTreap> get_unique_from_SeqElem(const common::SeqElem &e, const uint32_t req_database_id) = 0;
};

} // namespace treap_types