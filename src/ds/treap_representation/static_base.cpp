#include "static_base.hpp"
#include "../../common/constants.hpp"
#include "../../common/h5_helper.hpp"

namespace treap_types {

bool BaseSortedTreap::operator < (const BaseSortedTreap &oth) const {
    return key < oth.key;
}

BaseSortedTreap::BaseSortedTreap(const std::string &req_key, const uint32_t req_database_id) {
    this->key = req_key;
    this->database_id = req_database_id;
}

BaseSortedTreap::BaseSortedTreap(const H5::Group &h5_group) {
    this->key = H5Helper::get_string_attr_from(h5_group, "key");
    this->database_id = H5Helper::get_uint32_attr_from(h5_group, "database_id");
}

BaseSortedTreap::~BaseSortedTreap() {}

} // namespace treap_types