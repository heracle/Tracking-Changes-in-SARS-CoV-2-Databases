#include "static_location.hpp"

#include <iostream>

#include "static_base.hpp"
#include "static_acc_id.hpp"
#include "../../common/json_helper.hpp"
#include "../../common/constants.hpp"
#include "../../common/logger.hpp"
#include "../../common/h5_helper.hpp"
#include "../tnode_types/tnode_location.hpp"

namespace treap_types {

using namespace common;

LocationSorted::LocationSorted(
    const std::string &req_key, 
    const uint32_t req_database_id,
    const std::vector<std::pair<uint32_t, uint32_t>> &req_bp_alterations) : BaseSortedTreap(req_key, req_database_id) {
    this->bp_alterations = req_bp_alterations;
};

LocationSorted::~LocationSorted() {}

std::vector<std::vector<uint32_t>> LocationSorted::alteration_list_SeqElem;
uint32_t LocationSorted::next_alteration_SeqElem_id = 0;

std::unique_ptr<BaseSortedTreap> LocationSorted::get_unique_from_SeqElem(const SeqElem &e, const uint32_t req_database_id, const bool reinsert) {
    if (! reinsert) {
        auto aux = std::make_unique<LocationSorted>(
            e.covv_data[SEQ_FIELDS_TO_ID.at("covv_location")] + "_$_" + e.covv_data[SEQ_FIELDS_TO_ID.at("covv_collection_date")],
            req_database_id,
            std::vector<std::pair<uint32_t, uint32_t>>()
        );

        std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
        return answer;
    }
    std::vector<uint32_t> &alteration_SeqElem = LocationSorted::alteration_list_SeqElem[LocationSorted::next_alteration_SeqElem_id++];

    std::vector<std::pair<uint32_t, uint32_t>> curr_alteration_pr;
    // make clusters of 100 bp indexes.
    std::pair<uint32_t, uint32_t> pr(0, 0);

    for (uint32_t j = 0; j < alteration_SeqElem.size(); ++j) {
        if ((alteration_SeqElem[j] / 1000) > pr.first) {
            if (pr.second != 0) {
                curr_alteration_pr.push_back(pr);
            }
            pr.first = (alteration_SeqElem[j] / 1000);
            pr.second = 0;
        }
        pr.second++;
    }
    if (pr.second != 0) {
        curr_alteration_pr.push_back(pr);
    }

    auto aux = std::make_unique<LocationSorted>(
        e.covv_data[SEQ_FIELDS_TO_ID.at("covv_location")] + "_$_" + e.covv_data[SEQ_FIELDS_TO_ID.at("covv_collection_date")],
        req_database_id,
        curr_alteration_pr
    );
    std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
    return answer;
}

void LocationSorted::reset_get_unique_from_SeqElem(const ds::PS_Treap *accid_base_treap,
                                                   const ds::PS_Treap *accid_snapshot_treap,
                                                   const ds::DB *base_db,
                                                   common::SeqElemReader *snapshot_reader,
                                                   const std::vector<uint32_t>&, 
                                                   const std::vector<uint32_t>&, 
                                                   std::vector<std::pair<uint32_t, uint32_t>> updates_db_ids) {
    LocationSorted::alteration_list_SeqElem.clear();

    std::sort(updates_db_ids.begin(), updates_db_ids.end(), [](const std::pair<uint32_t, uint32_t> &a, const std::pair<uint32_t, uint32_t> &b) {
        if (a.second != b.second) {
            return a.second < b.second;
        }
        return a.first < b.first;
    });

    for (const std::pair<uint32_t, uint32_t> pair_ids : updates_db_ids) {
        AccessionIdSorted* ctc_elem = static_cast<AccessionIdSorted*>(accid_base_treap->static_data[pair_ids.first].get());
        AccessionIdSorted* snapshot_elem = static_cast<AccessionIdSorted*>(accid_snapshot_treap->static_data[pair_ids.second].get());

        if (ctc_elem->seq_hash == snapshot_elem->seq_hash) {
            alteration_list_SeqElem.push_back(std::vector<uint32_t>());
            continue;
        }

        // find all bp indexes where the 2 sequences differ.
        std::string seq1 = base_db->get_element(ctc_elem->database_id).covv_data[SEQ_FIELDS_TO_ID.at("sequence")];
        std::string seq2 = snapshot_reader->get_elem(pair_ids.second).covv_data[SEQ_FIELDS_TO_ID.at("sequence")];
        if (seq1.size() != seq2.size()) {
            Logger::error("internal error, the sizes of the 2 algined sequences to compare are different");
        }

        auto curr_bp_alterations = std::vector<uint32_t>();
        for (uint32_t i = 0; i < seq1.size(); ++i) {
            if (seq1[i] != seq2[i]) {
                curr_bp_alterations.push_back(i);
            }
        }
        alteration_list_SeqElem.push_back(curr_bp_alterations);
    }
}

void LocationSorted::reset_get_new_BaseSortedTreap(const H5::Group&) {}

std::unique_ptr<BaseSortedTreap> LocationSorted::get_new_BaseSortedTreap(const std::string &key, const uint32_t database_id) {
    auto aux =  std::make_unique<LocationSorted>(key, database_id); 
    std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
    return answer;
}

void serialize_location_elem_to_hdf5(const std::vector<std::unique_ptr<BaseSortedTreap>> &elems, H5::Group &h5_group) {
    std::vector<std::string> saved_key;
    std::vector<uint32_t> saved_database_id;
    for (uint32_t i = 0; i < elems.size(); ++i) {
        LocationSorted* elem = static_cast<LocationSorted*>(elems[i].get());

        saved_key.push_back(elem->key);
        saved_database_id.push_back(elem->database_id);
    }

    H5Helper::write_h5_dataset(saved_key, &h5_group, "key");
    H5Helper::write_h5_int_to_dataset(saved_database_id, &h5_group, "database_id");
}


} // namespace treap_types
