#include "ctc.hpp"

#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"

#include "treap_representation/static_acc_id.hpp"
#include "treap_representation/static_location.hpp"

#include <iostream>

namespace ds {

CTC::CTC() {
    this->db = new ds::DB();
    this->treap_location = new ds::PS_Treap(recompute_location_statistics);
    this->treap_acc_id = new ds::PS_Treap(recompute_acc_id_statistics);
}

CTC::CTC(H5::H5File &h5_file) {
    H5::Group database_group = H5Gopen(h5_file.getLocId(), "/database", H5P_DEFAULT);
    this->db = new ds::DB(database_group);
    H5::Group location_group = H5Gopen(h5_file.getLocId(), "/location_treap", H5P_DEFAULT);
    uint32_t size_location_data = H5Helper::get_uint32_attr_from(location_group, "static_data_size");

    // read data before calling the LocationSorted Treap constructor
    std::vector<std::unique_ptr<BaseSortedTreap>> location_data;
    for (uint32_t i = 0; i < size_location_data; ++i) {
        H5::Group curr_data_group = H5Gopen(location_group.getLocId(), ("static_data" + std::to_string(i)).c_str(), H5P_DEFAULT);
        std::unique_ptr<BaseSortedTreap> curr_location = std::make_unique<LocationSorted>(curr_data_group);
        curr_data_group.close();

        location_data.push_back(std::move(curr_location));
    }
    this->treap_location = new ds::PS_Treap(location_group, location_data, recompute_location_statistics);

    H5::Group acc_id_group = H5Gopen(h5_file.getLocId(), "/acc_id_treap", H5P_DEFAULT);
    uint32_t size_acc_id_data = H5Helper::get_uint32_attr_from(acc_id_group, "static_data_size");

    // read data before calling the AccessionId Treap constructor
    std::vector<std::unique_ptr<BaseSortedTreap>> acc_id_data;
        
    for (uint32_t i = 0; i < size_acc_id_data; ++i) {
        H5::Group curr_data_group = H5Gopen(acc_id_group.getLocId(), ("static_data" + std::to_string(i)).c_str(), H5P_DEFAULT);
        std::unique_ptr<BaseSortedTreap> curr_acc_id = std::make_unique<AccessionIdSorted>(curr_data_group);
        curr_data_group.close();

        acc_id_data.push_back(std::move(curr_acc_id));
    }
    this->treap_acc_id = new ds::PS_Treap(acc_id_group, acc_id_data, recompute_acc_id_statistics);
}

CTC::~CTC() {
    // todo make sure that the destructors for db and treaps are working properly.
    delete db;
    delete treap_location;
    delete treap_acc_id;
}

void CTC::insert_and_clear_ram(std::vector<common::SeqElem> *seq_elems) {
    std::vector<std::unique_ptr<BaseSortedTreap>> seq_AccessionIdSorted_to_insert;
    std::vector<std::unique_ptr<BaseSortedTreap>> seq_LocationSorted_to_insert;
    for (int i = seq_elems->size() - 1; i >= 0; --i) {
    // for (const common::SeqElem &seq : seq_elems) {
        uint32_t seq_id = db->insert_element((*seq_elems)[i]);
        seq_AccessionIdSorted_to_insert.push_back(std::move(AccessionIdSorted::get_unique_from_SeqElem((*seq_elems)[i], seq_id)));
        seq_LocationSorted_to_insert.push_back(std::move(LocationSorted::get_unique_from_SeqElem((*seq_elems)[i], seq_id)));
        seq_elems->pop_back();
    }

    Logger::trace("Total number of elements to insert: " + std::to_string(seq_LocationSorted_to_insert.size()));
    
    treap_acc_id->insert(seq_AccessionIdSorted_to_insert);
    treap_location->insert(seq_LocationSorted_to_insert);
}

void CTC::erase(const std::vector<uint32_t> &deletions_db_ids) {
    treap_acc_id->erase(deletions_db_ids);
    treap_location->erase(deletions_db_ids);
}

void CTC::save_snapshot(const std::string &name) {
    treap_acc_id->save_snapshot(name);
    treap_location->save_snapshot(name);
}

void CTC::export_to_h5(H5::H5File &h5_file) {
    // treap_location->iterate_ordered([&](const common::LocationSorted &x) {std::cerr << x.key << "\n";});

    H5::Group location_treap_group = h5_file.createGroup("/location_treap");
    treap_location->export_to_hdf5(location_treap_group, append_location_elem_to_hdf5);
    H5::Group acc_id_treap_group = h5_file.createGroup("/acc_id_treap");
    treap_acc_id->export_to_hdf5(acc_id_treap_group, append_acc_id_elem_to_hdf5);
    H5::Group database_group = h5_file.createGroup("/database");
    db->export_to_hdf5(database_group, ds::add_db_elem_to_hdf5);

    location_treap_group.close();
    acc_id_treap_group.close();
    database_group.close();
}

} // namespace ds