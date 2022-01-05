#pragma once

#include <vector>
#include <string>

#include "../external_libraries/json.hpp"
#include "../common/utils.hpp"
#include "H5Cpp.h"

using Json = nlohmann::json;

namespace ds {

class DB {
  private:
    std::vector<common::SeqElem> data;
    // TODO add previous version for data!
    // std::vector<uint32_t> previous_version;

  public:
    // insert_element() insert one sequence to the database and returns the corresponding database id.
    uint32_t insert_element(const common::SeqElem &seq);
    // get_element() returns a copy of the sequence stored at the database's 'id' index.
    common::SeqElem get_element(uint32_t id);
    // export_to_hdf5() return a serialization (in hdf5 format) for 'this' object.
    void export_to_hdf5(H5::Group &database_group,
                        const std::function<void(const common::SeqElem &elem, H5::Group &elem_group)> &h5_append_elem);

    DB();
    DB(H5::Group &database_group);
};

void add_db_elem_to_hdf5(const common::SeqElem &elem, H5::Group &elem_group);

} // namespace ds
