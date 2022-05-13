#include "database.hpp"

#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"

#include "tnode_types/tnode_base.hpp"


#include <cstdlib>
#include <iostream>

namespace ds {

using namespace common;

SeqElem DB::get_element(uint32_t id) const {
    if (id >= data_size) {
        throw std::runtime_error("ERROR -> requested element in DB with id that doesn't exist.");
    }
    SeqElem answer;

    for (uint32_t i = 0; i < db_str_fields.size(); ++i) {
        answer.covv_data[i] = H5Helper::get_from_extendable_h5_dataset(id, group, db_str_fields[i]);
    }
    answer.prv_db_id = std::stoul(H5Helper::get_from_extendable_h5_dataset(id, group, "prv_list"));

    return answer;
}

uint32_t DB::insert_element(const SeqElem seq) {
    buff_data.push_back(seq);

    this->data_size++;
    if (buff_data.size() == flush_size) {
        write_buff_data();
    }
    return this->data_size - 1;
}

void DB::init() {
    for (const std::string &elem : SEQ_FIELDS) {
        db_str_fields.push_back(elem);
    }
}

DB::DB(H5::H5File *h5_file, const uint32_t req_flush_size)  {
    flush_size = req_flush_size;
    init();
    if (H5Lexists(h5_file->getId(), "/database", H5P_DEFAULT ) > 0) {
        this->group = H5Gopen(h5_file->getLocId(), "/database", H5P_DEFAULT);
        this->data_size = H5Helper::get_uint32_attr_from(this->group, "data_size");

        // todo move these variables (next_index_tnode and first_notsaved_index_tnode) to a different place
        treap_types::Tnode::next_index_tnode = H5Helper::get_uint64_attr_from(this->group, "next_index_tnode");
        treap_types::Tnode::first_notsaved_index_tnode = H5Helper::get_uint64_attr_from(this->group, "first_notsaved_index_tnode");
        return;
    }
    
    this->group = h5_file->createGroup("/database");
    for (const std::string &field : db_str_fields) {
        H5Helper::create_extendable_h5_dataset(group, field);
    }
    H5Helper::create_extendable_h5_dataset(group, "prv_list");
}

void DB::write_buff_data() {
    if (buff_data.size() == 0) {
        return;
    }

    std::vector<std::string> prv_linked_list;
    for (const common::SeqElem &elem : buff_data) {
        prv_linked_list.push_back(std::to_string(elem.prv_db_id));
    }
    H5Helper::append_extendable_h5_dataset(prv_linked_list, group, "prv_list");

    for (const std::string &field : db_str_fields) {
        std::vector<std::string> field_data;
        for (const common::SeqElem &elem : buff_data) {
            field_data.push_back(elem.covv_data[SEQ_FIELDS_TO_ID.at(field)]);
        }

        H5Helper::append_extendable_h5_dataset(field_data, group, field);
    }
    buff_data.clear();

    H5Helper::set_uint32_hdf5_attr(this->data_size, &(this->group), "data_size");
    H5Helper::set_uint64_hdf5_attr(treap_types::Tnode::next_index_tnode, &(this->group), "next_index_tnode");
    H5Helper::set_uint64_hdf5_attr(treap_types::Tnode::first_notsaved_index_tnode, &(this->group), "first_notsaved_index_tnode");
}

void DB::clone_db(const ds::DB &source) {
    for (uint32_t i = 0; i < source.data_size; ++i) {
        common::SeqElem curr = source.get_element(i);
        this->insert_element(curr);
    }
    write_buff_data();
}

} // namespace ds
