#include "database.hpp"

#include "../common/constants.hpp"
#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"

#include <cstdlib>

namespace ds {

void add_db_elem_to_hdf5(const common::SeqElem &elem, H5::Group &elem_group) {
    H5Helper::set_string_hdf5_attr(elem.covv_accession_id, &elem_group, "covv_accession_id");
    H5Helper::set_string_hdf5_attr(elem.covv_collection_date, &elem_group, "covv_collection_date");
    H5Helper::set_string_hdf5_attr(elem.covv_location, &elem_group, "covv_location");
    H5Helper::set_string_hdf5_attr(elem.sequence, &elem_group, "sequence");
}

common::SeqElem DB::get_element(uint32_t id) {
    if (id >= data.size()) {
        throw std::runtime_error("ERROR -> requested element in DB with id that doesn't exist.");
    }
    return data[id];
}

uint32_t DB::insert_element(const common::SeqElem &seq) {
    data.push_back(seq);
    return data.size() - 1;
}

void DB::export_to_hdf5(H5::Group &database_group,
                        const std::function<void(const common::SeqElem &elem, H5::Group &elem_group)> &h5_append_elem) {
    // Create new string datatype for attribute
    H5Helper::set_uint32_hdf5_attr(this->data.size(), &database_group, "data_size");

    // serialize data
    for (uint32_t i = 0; i < this->data.size(); ++i) {
        H5::Group* g1 = new H5::Group(database_group.createGroup(std::to_string(i).c_str()));
        h5_append_elem(this->data[i], *g1);
        g1->close();
    }

}

DB::DB(H5::Group &database_group) {
    uint32_t size_db = H5Helper::get_uint32_attr_from(database_group, "data_size");
    this->data.resize(size_db);

    for (uint32_t i = 0; i < size_db; ++i) {
        H5::Group curr_group = H5Gopen(database_group.getLocId(), std::to_string(i).c_str(), H5P_DEFAULT);
        this->data[i] = common::SeqElem::get_from_hdf5(curr_group);
        curr_group.close();
    }
}

DB::DB() {}

} // namespace ds