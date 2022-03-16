#include "ctc.hpp"

#include "../common/constants.hpp"
#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"
#include "../common/json_helper.hpp"

#include "static_types/static_acc_id.hpp"
#include "static_types/static_location.hpp"
#include "tnode_types/tnode_base.hpp"
#include "tnode_types/tnode_location.hpp"
#include "tnode_types/tnode_acc_id.hpp"

#include <iostream>

namespace ds {

void CTC::ctc_init() {
    this->treaps["location_treap"] = {
        NULL,
        treap_types::LocationTnode::create_new_specialized_tnode,
        treap_types::LocationTnode::copy_specialized_tnode,
        treap_types::LocationTnode::get_h5_tnode,
        treap_types::LocationTnode::reset_get_h5_tnode,
        treap_types::LocationTnode::append_tnode_data,
        treap_types::LocationTnode::reset_append_tnode_data,
        treap_types::LocationTnode::write_tnode_data_to_h5,
        recompute_location_statistics,
        treap_types::LocationSorted::get_new_BaseSortedTreap,
        treap_types::LocationSorted::reset_get_new_BaseSortedTreap,
        treap_types::LocationSorted::get_unique_from_SeqElem,
        treap_types::LocationSorted::reset_get_unique_from_SeqElem,
        treap_types::serialize_location_elem_to_hdf5
    };
    this->treaps["acc_id_treap"] = {
        NULL,
        treap_types::AccIdTnode::create_new_specialized_tnode,
        treap_types::AccIdTnode::copy_specialized_tnode,
        treap_types::AccIdTnode::get_h5_tnode,
        treap_types::AccIdTnode::reset_get_h5_tnode,
        treap_types::AccIdTnode::append_tnode_data,
        treap_types::AccIdTnode::reset_append_tnode_data,
        treap_types::AccIdTnode::write_tnode_data_to_h5,
        recompute_acc_id_statistics,
        treap_types::AccessionIdSorted::get_new_BaseSortedTreap,
        treap_types::AccessionIdSorted::reset_get_new_BaseSortedTreap,
        treap_types::AccessionIdSorted::get_unique_from_SeqElem,
        treap_types::AccessionIdSorted::reset_get_unique_from_SeqElem,
        treap_types::serialize_acc_id_elem_to_hdf5
    };
}

// todo fix constuctor duplicatio (used by create)
CTC::CTC(H5::H5File *h5_file, CTC* source) : h5file(*h5_file) {
    this->ctc_init();
    this->db = new ds::DB(h5_file);
    
    if (source != NULL) {
        this->db->clone_db(*(source->db));
        for (auto [treap_name, treap_data] : this->treaps) {
            this->treaps[treap_name].treap = new ds::PS_Treap(source->treaps.at(treap_name).treap);
        }
    } else {
        for (auto [treap_name, treap_data] : this->treaps) {
            this->treaps[treap_name].treap = new ds::PS_Treap(treap_data.recompute_tnode_statistics, 
                                                            treap_data.create_new_specialized_tnode,
                                                            treap_data.copy_specialized_tnode);
        }
    }
    
    this->hash_treap = this->treaps["acc_id_treap"].treap;
}

// todo keep only one h5 constructor..
// todo fix constuctor duplicatio (used by append)
CTC::CTC(H5::H5File &h5_file) : h5file(h5_file) {
    this->ctc_init();
    this->db = new ds::DB(&h5_file);
    
    for (auto [treap_name, treap_data] : this->treaps) {
        H5::Group treap_group = H5Gopen(h5_file.getLocId(), ("/" + treap_name).c_str(), H5P_DEFAULT);

        // read data before calling the LocationSorted Treap constructor
        std::vector<std::string> keys = H5Helper::read_h5_dataset(treap_group, "key");
        std::vector<uint32_t> database_ids = H5Helper::read_h5_int_to_dataset<uint32_t>(treap_group, "database_id");
        assert(keys.size() == database_ids.size());

        std::vector<std::unique_ptr<BaseSortedTreap>> treap_static_data;
        treap_data.reset_get_new_BaseSortedTreap(treap_group);
        for (uint32_t i = 0; i < keys.size(); ++i) {
            std::unique_ptr<BaseSortedTreap> curr = treap_data.get_new_BaseSortedTreap(keys[i], database_ids[i]);
            treap_static_data.push_back(std::move(curr));
        }
        treap_data.reset_get_h5_tnode(treap_group);
        this->treaps[treap_name].treap = new ds::PS_Treap(treap_group, 
                                            treap_static_data, 
                                            treap_data.recompute_tnode_statistics, 
                                            treap_data.create_new_specialized_tnode, 
                                            treap_data.copy_specialized_tnode, 
                                            treap_data.get_h5_tnode);
    }
    this->hash_treap = this->treaps.at("acc_id_treap").treap;    
}

CTC::~CTC() {
    // todo make sure that the destructors for db and treaps are working properly.
    delete db;
    for (auto [treap_name, treap_data] : this->treaps) {
        delete treap_data.treap;
    }
}

void CTC::insert_seq(const std::vector<common::SeqElem> *seq_elems, bool was_modified) {
    std::vector<uint32_t> seq_ids(seq_elems->size());
    for (uint32_t i = 0; i < seq_elems->size(); ++i) {
        seq_ids[i] = db->insert_element((*seq_elems)[i]);
    }

    for (auto [treap_name, treap_data] : this->treaps) {
        std::vector<std::unique_ptr<BaseSortedTreap>> unique_seq_to_insert;
        for (uint32_t i = 0; i < seq_elems->size(); i++) {
            unique_seq_to_insert.push_back(std::move(treap_data.get_unique_from_SeqElem((*seq_elems)[i], seq_ids[i], was_modified)));
        }
        
        Logger::trace("Total number of elements to insert in '" + treap_name + "' treap: " + std::to_string(unique_seq_to_insert.size()));
        treap_data.treap->insert(unique_seq_to_insert);
    }
}

void CTC::erase_seq(const std::vector<uint32_t> &deletions_db_ids, const bool) {
    for (auto [treap_name, treap_data] : this->treaps) {
        treap_data.treap->erase(deletions_db_ids);
    }
}

void CTC::save_snapshot(const std::string &name) {
    for (auto [treap_name, treap_data] : this->treaps) {
        treap_data.treap->save_snapshot(name);
    }
}

void CTC::export_to_h5() {
    for (auto [treap_name, treap_data] : this->treaps) {
        H5::Group treap_group;
        if (H5Lexists( h5file.getId(), ("/" + treap_name).c_str(), H5P_DEFAULT ) > 0) {
            treap_group = h5file.openGroup(("/" + treap_name).c_str());

            // std::function<void(H5::H5Object&, const std::string, void *)> attr_op = 
            // [&treap_group](H5::H5Object&, const std::string attr_name, void *){
            //      treap_group.removeAttr(attr_name);
            //  };

            // H5::attr_operator_t attr_op_h5 = attr_op;

            // // delete all attrs:
            // treap_group.iterateAttrs(attr_op_h5);

            
        } else {
            treap_group = h5file.createGroup(("/" + treap_name).c_str());
        }
        treap_data.reset_append_tnode_data();
        treap_data.treap->export_to_hdf5(treap_group, treap_data.serialize_elem_to_hdf5, treap_data.append_tnode_data);
        treap_data.write_tnode_data_to_h5(treap_group);
        treap_group.close();
    }

    // todo make a method for last steps.
    db->write_buff_data();
    // TODO create in a different piece of code.
    // H5::Group database_group = h5_file.createGroup("/database");
    // db->export_to_hdf5(database_group);
    // database_group.close();
    H5Helper::set_uint32_hdf5_attr(db->data_size, &(db->group), "data_size");
    H5Helper::set_uint32_hdf5_attr(treap_types::Tnode::next_index_tnode, &(db->group), "next_index_tnode");
    H5Helper::set_uint32_hdf5_attr(treap_types::Tnode::first_notsaved_index_tnode, &(db->group), "first_notsaved_index_tnode");
}

void CTC::prepare_specific_get_unique_SeqElem(const ds::PS_Treap *accid_base_treap,
                                              const ds::PS_Treap *accid_snapshot_treap,
                                              const DB *base_db,
                                              const std::string &file_name,
                                              const std::vector<uint32_t> &insertions_db_ids, 
                                              const std::vector<uint32_t> &deletions_db_ids, 
                                              const std::vector<std::pair<uint32_t, uint32_t>> &updates_db_ids) const {
    for (auto [treap_name, treap_data] : this->treaps) {
        common::SeqElemReader *seq_reader = new common::SeqElemReader(file_name);
        treap_data.reset_get_unique_from_SeqElem(accid_base_treap, accid_snapshot_treap, base_db, seq_reader, insertions_db_ids, deletions_db_ids, updates_db_ids);
        delete seq_reader;
    }
}

} // namespace ds