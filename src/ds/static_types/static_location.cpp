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
    const uint64_t req_database_id,
    const uint64_t req_num_seq_versions,
    const std::vector<uint64_t> &req_bp_alterations,
    const bool req_is_human_host) : BaseSortedTreap(req_key, req_database_id) {
    this->bp_alterations = req_bp_alterations;
    this->num_sequence_versions = req_num_seq_versions;
    this->is_human_host = req_is_human_host;
};

LocationSorted::~LocationSorted() {}

std::vector<std::vector<uint64_t>> LocationSorted::alteration_list_SeqElem;
uint64_t LocationSorted::next_alteration_SeqElem_id = 0;

std::unique_ptr<BaseSortedTreap> LocationSorted::get_unique_from_snapshot_line(const SeqElem &e, const uint64_t req_database_id, const BaseSortedTreap *prv) {
    if (prv == NULL) {
        auto aux = std::make_unique<LocationSorted>(
            e.covv_data[SEQ_FIELDS_TO_ID.at("covv_location")] + "_$_" + e.covv_data[SEQ_FIELDS_TO_ID.at("covv_collection_date")],
            req_database_id,
            0,
            std::vector<uint64_t>(),
            e.covv_data[SEQ_FIELDS_TO_ID.at("covv_host")] == "Human"
        );

        std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
        return answer;
    }

    const LocationSorted *location_prv = static_cast<const LocationSorted*>(prv);

    const std::vector<uint64_t> &prv_alterations = location_prv->bp_alterations;
    const std::vector<uint64_t> &new_alterations = (LocationSorted::alteration_list_SeqElem[LocationSorted::next_alteration_SeqElem_id++]);
    std::vector<uint64_t> merged_alterations;
    uint64_t prv_idx = 0, new_idx = 0;

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
        location_prv->num_sequence_versions + 1,
        merged_alterations,
        e.covv_data[SEQ_FIELDS_TO_ID.at("covv_host")] == "Human"
    );
    std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
    return answer;
}

void LocationSorted::reset_get_unique_from_snapshot_line(const ds::PS_Treap *accid_base_treap,
                                                        const ds::PS_Treap *accid_snapshot_treap,
                                                        const ds::DB *base_db,
                                                        common::SeqElemReader *snapshot_reader,
                                                        const std::vector<uint64_t>&, 
                                                        const std::vector<uint64_t>&, 
                                                        std::vector<std::pair<uint64_t, uint64_t>> updates_db_ids) {
    LocationSorted::alteration_list_SeqElem.clear();

    std::sort(updates_db_ids.begin(), updates_db_ids.end(), [](const std::pair<uint64_t, uint64_t> &a, const std::pair<uint64_t, uint64_t> &b) {
        if (a.second != b.second) {
            return a.second < b.second;
        }
        return a.first < b.first;
    });

    for (const std::pair<uint64_t, uint64_t> pair_ids : updates_db_ids) {
        AccessionIdSorted* ctc_elem = static_cast<AccessionIdSorted*>(accid_base_treap->static_data[pair_ids.first].get());
        AccessionIdSorted* snapshot_elem = static_cast<AccessionIdSorted*>(accid_snapshot_treap->static_data[pair_ids.second].get());

        if (ctc_elem->seq_hash == snapshot_elem->seq_hash) {
            alteration_list_SeqElem.push_back(std::vector<uint64_t>());
            continue;
        }

        // find all bp indexes where the 2 sequences differ.
        std::string seq1 = base_db->get_element(ctc_elem->database_id).covv_data[SEQ_FIELDS_TO_ID.at("sequence")];
        std::string seq2 = snapshot_reader->get_elem(pair_ids.second).covv_data[SEQ_FIELDS_TO_ID.at("sequence")];
        if (seq1.size() != seq2.size()) {
            Logger::error("internal error, the sizes of the 2 aligned sequences to compare are different");
        }

        auto curr_bp_alterations = std::vector<uint64_t>();
        for (uint64_t i = 0; i < seq1.size(); ++i) {
            if (seq1[i] != seq2[i]) {
                curr_bp_alterations.push_back(i * (1<<common::BITS_FOR_STEPS_BACK));
            }
        }
        alteration_list_SeqElem.push_back(curr_bp_alterations);
    }
}

std::vector<uint64_t> LocationSorted::alteration_sizes;
std::vector<uint64_t> LocationSorted::merged_alterations;
std::vector<uint64_t> LocationSorted::num_seq_vers_h5;
std::vector<uint64_t> LocationSorted::is_human_host_list_SeqElem;
uint64_t LocationSorted::next_node_id;
uint64_t LocationSorted::next_merged_id;

void LocationSorted::reset_get_new_BaseSortedTreap(const H5::Group &group) {
    LocationSorted::next_node_id = 0;
    LocationSorted::next_merged_id = 0;
    LocationSorted::alteration_sizes = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "alteration_sizes");
    LocationSorted::merged_alterations = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "merged_alterations");
    LocationSorted::num_seq_vers_h5 = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "num_seq_vers_h5");
    LocationSorted::is_human_host_list_SeqElem = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "is_human_host");
}

std::unique_ptr<BaseSortedTreap> LocationSorted::get_new_BaseSortedTreap(const std::string &key, const uint64_t database_id) {
    uint64_t num_alters = alteration_sizes[next_node_id];
    std::vector<uint64_t> curr_alterations;
    for (uint64_t i = LocationSorted::next_merged_id; i < LocationSorted::next_merged_id + num_alters; ++i) {
        curr_alterations.push_back(LocationSorted::merged_alterations[i]);
    }
    LocationSorted::next_merged_id += num_alters;

    bool curr_is_human_host = false;
    if (is_human_host_list_SeqElem[next_node_id] == 0) {
        curr_is_human_host = true;
    }

    auto aux =  std::make_unique<LocationSorted>(key, database_id, num_seq_vers_h5[next_node_id], curr_alterations, curr_is_human_host); 
    next_node_id++;
    std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
    return answer;
}

void serialize_location_elem_to_hdf5(const std::vector<std::unique_ptr<BaseSortedTreap>> &elems, H5::Group &h5_group) {
    std::vector<std::string> saved_key;
    std::vector<uint64_t> saved_database_id;

    std::vector<uint64_t> alteration_sizes;
    std::vector<uint64_t> merged_alterations;
    std::vector<uint64_t> saved_num_seq_vers;
    std::vector<uint64_t> saved_is_human_host;
    for (uint64_t i = 0; i < elems.size(); ++i) {
        LocationSorted* elem = static_cast<LocationSorted*>(elems[i].get());

        saved_key.push_back(elem->key);
        saved_database_id.push_back(elem->database_id);
        saved_num_seq_vers.push_back(elem->num_sequence_versions);

        alteration_sizes.push_back(elem->bp_alterations.size());

        for (uint64_t i = 0; i < elem->bp_alterations.size(); ++i) {
            merged_alterations.push_back(elem->bp_alterations[i]);
        }

        if (elem->is_human_host) {
            saved_is_human_host.push_back(0);
        } else {
            saved_is_human_host.push_back(1);
        }
    }

    H5Helper::write_h5_dataset(saved_key, &h5_group, "key");
    H5Helper::write_h5_int_to_dataset(saved_database_id, &h5_group, "database_id");
    H5Helper::write_h5_int_to_dataset(alteration_sizes, &h5_group, "alteration_sizes");
    H5Helper::write_h5_int_to_dataset(merged_alterations, &h5_group, "merged_alterations");
    H5Helper::write_h5_int_to_dataset(saved_num_seq_vers, &h5_group, "num_seq_vers_h5");
    H5Helper::write_h5_int_to_dataset(saved_is_human_host, &h5_group, "is_human_host");
}

std::unique_ptr<BaseSortedTreap> LocationSorted::copy_specialized_static_field(const BaseSortedTreap* oth) {
    const LocationSorted *acc_id_oth = static_cast<const LocationSorted*>(oth);

    std::unique_ptr<BaseSortedTreap> unique_curr = std::make_unique<LocationSorted>(
        acc_id_oth->key,
        acc_id_oth->database_id,
        acc_id_oth->num_sequence_versions,
        acc_id_oth->bp_alterations,
        acc_id_oth->is_human_host
    );
    return unique_curr;
}

} // namespace treap_types
