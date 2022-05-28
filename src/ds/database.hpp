#pragma once

#include <vector>
#include <string>

#include "../common/constants.hpp"
#include "../common/json_helper.hpp"
#include "H5Cpp.h"

namespace ds {

class DB {
  private:
    // TODO add previous version for data!
    // std::vector<common::SeqElem> data;
    std::vector<common::SeqElem> buff_data;
    std::vector<std::string> db_str_fields;
    void init();
    uint64_t flush_size;

  public:
    //todo make private
    uint64_t data_size = 0;
    H5::Group group;
    void write_buff_data();
    // insert_element() insert one sequence to the database and returns the corresponding database id.
    uint64_t insert_element(const common::SeqElem seq);
    // get_element() returns a copy of the sequence stored at the database's 'id' index.
    common::SeqElem get_element(uint64_t id) const;
    std::vector<common::SeqElem> get_multiple_element(const uint64_t start_id, const uint64_t num_elements) const;
    
    // export_to_hdf5() return a serialization (in hdf5 format) for 'this' object.
    // void export_to_hdf5(H5::Group &database_group);

    DB(H5::H5File *h5_file, const uint64_t req_flush_size = common::H5_APPEND_SIZE);

    void clone_db(const ds::DB &source);
};

} // namespace ds
