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
    const std::vector<uint32_t> &req_bp_alterations) : BaseSortedTreap(req_key, req_database_id) {
    this->bp_alterations = req_bp_alterations;
};

LocationSorted::~LocationSorted() {}

std::vector<std::vector<uint32_t>> LocationSorted::alteration_list_SeqElem;
uint32_t LocationSorted::next_alteration_SeqElem_id = 0;

std::unique_ptr<BaseSortedTreap> LocationSorted::get_unique_from_snapshot_line(const SeqElem &e, const uint32_t req_database_id, const BaseSortedTreap *prv) {
    if (prv == NULL) {
        auto aux = std::make_unique<LocationSorted>(
            e.covv_data[SEQ_FIELDS_TO_ID.at("covv_location")] + "_$_" + e.covv_data[SEQ_FIELDS_TO_ID.at("covv_collection_date")],
            req_database_id,
            std::vector<uint32_t>()
        );

        std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
        return answer;
    }

    const std::vector<uint32_t> &prv_alterations = (static_cast<const LocationSorted*>(prv))->bp_alterations;
    const std::vector<uint32_t> &new_alterations = (LocationSorted::alteration_list_SeqElem[LocationSorted::next_alteration_SeqElem_id++]);
    std::vector<uint32_t> merged_alterations;
    uint32_t prv_idx = 0, new_idx = 0;

    while (prv_idx < prv_alterations.size() && new_idx < new_alterations.size()) {
        assert(prv_alterations[prv_idx] + 1 != new_alterations[new_idx]); /*
            all the prv_alterations[prv_idx]&256 are coming from the same sequence version, so, it would be impossible to have the same alteration 
            occuring two times in the same sequence version.
        */
        if (prv_alterations[prv_idx] < new_alterations[new_idx]) {
            merged_alterations.push_back(prv_alterations[prv_idx] + 1); /* 
                this +1 represents that the alteration is imported from an earlier sequence version (+1 steps back on the static data linked list)
                usually, for the 5th bp we will have an alteration = 5 * (2<<7), because the last 7 bits are used for these steps back.
            */ 
           prv_idx++;
        } else {
            merged_alterations.push_back(new_alterations[new_idx]);
            new_idx++;
        }
    }
    while (prv_idx < prv_alterations.size()) {
        merged_alterations.push_back(prv_alterations[prv_idx] + 1);
        prv_idx++;
    }
    while (new_idx < new_alterations.size()) {
        merged_alterations.push_back(new_alterations[new_idx]);
        new_idx++;
    }

    auto aux = std::make_unique<LocationSorted>(
        e.covv_data[SEQ_FIELDS_TO_ID.at("covv_location")] + "_$_" + e.covv_data[SEQ_FIELDS_TO_ID.at("covv_collection_date")],
        req_database_id,
        merged_alterations
    );
    std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
    return answer;
}

void LocationSorted::reset_get_unique_from_snapshot_line(const ds::PS_Treap *accid_base_treap,
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
            Logger::error("internal error, the sizes of the 2 aligned sequences to compare are different");
        }

        auto curr_bp_alterations = std::vector<uint32_t>();
        for (uint32_t i = 0; i < seq1.size(); ++i) {
            if (seq1[i] != seq2[i]) {
                curr_bp_alterations.push_back(i * (1<<common::BITS_FOR_STEPS_BACK));
            }
        }
        alteration_list_SeqElem.push_back(curr_bp_alterations);
    }
}

std::vector<uint64_t> LocationSorted::alteration_sizes;
std::vector<uint32_t> LocationSorted::merged_alterations;
uint32_t LocationSorted::next_node_id;
uint32_t LocationSorted::next_merged_id;

void LocationSorted::reset_get_new_BaseSortedTreap(const H5::Group &group) {
    LocationSorted::next_node_id = 0;
    LocationSorted::next_merged_id = 0;
    LocationSorted::alteration_sizes = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "alteration_sizes");
    LocationSorted::merged_alterations = H5Helper::read_h5_int_to_dataset<uint32_t>(group, "merged_alterations");
}

std::unique_ptr<BaseSortedTreap> LocationSorted::get_new_BaseSortedTreap(const std::string &key, const uint32_t database_id) {
    uint32_t num_alters = alteration_sizes[next_node_id++];
    std::vector<uint32_t> curr_alterations;
    for (uint64_t i = LocationSorted::next_merged_id; i < LocationSorted::next_merged_id + num_alters; ++i) {
        curr_alterations.push_back(LocationSorted::merged_alterations[i]);
    }
    LocationSorted::next_merged_id += num_alters;

    auto aux =  std::make_unique<LocationSorted>(key, database_id, curr_alterations); 
    std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
    return answer;
}

void serialize_location_elem_to_hdf5(const std::vector<std::unique_ptr<BaseSortedTreap>> &elems, H5::Group &h5_group) {
    std::vector<std::string> saved_key;
    std::vector<uint32_t> saved_database_id;

    std::vector<uint64_t> alteration_sizes;
    std::vector<uint32_t> merged_alterations;
    for (uint32_t i = 0; i < elems.size(); ++i) {
        LocationSorted* elem = static_cast<LocationSorted*>(elems[i].get());

        saved_key.push_back(elem->key);
        saved_database_id.push_back(elem->database_id);

        alteration_sizes.push_back(elem->bp_alterations.size());

        for (uint32_t i = 0; i < elem->bp_alterations.size(); ++i) {
            merged_alterations.push_back(elem->bp_alterations[i]);
        }
    }

    H5Helper::write_h5_dataset(saved_key, &h5_group, "key");
    H5Helper::write_h5_int_to_dataset(saved_database_id, &h5_group, "database_id");
    H5Helper::write_h5_int_to_dataset(alteration_sizes, &h5_group, "alteration_sizes");
    H5Helper::write_h5_int_to_dataset(merged_alterations, &h5_group, "merged_alterations");
}


} // namespace treap_types
