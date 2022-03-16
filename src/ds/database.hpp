#pragma once

#include <vector>
#include <string>

#include "../external_libraries/json.hpp"
#include "../common/json_helper.hpp"
#include "H5Cpp.h"

using Json = nlohmann::json;

namespace ds {

class DB {
  private:
    // TODO add previous version for data!
    // std::vector<common::SeqElem> data;
    std::vector<common::SeqElem> buff_data;
    std::vector<std::string> db_str_fields;
    void init();

  public:
    //todo make private
    uint32_t data_size = 0;
    H5::Group group;
    void write_buff_data();
    // insert_element() insert one sequence to the database and returns the corresponding database id.
    uint32_t insert_element(const common::SeqElem seq);
    // get_element() returns a copy of the sequence stored at the database's 'id' index.
    common::SeqElem get_element(uint32_t id) const;
    // export_to_hdf5() return a serialization (in hdf5 format) for 'this' object.
    // void export_to_hdf5(H5::Group &database_group);

    DB(H5::H5File *h5_file);

    void clone_db(const ds::DB &source);
};

void add_db_elem_to_hdf5(const common::SeqElem &elem, H5::Group &elem_group);

} // namespace ds
