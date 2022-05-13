#include "static_acc_id.hpp"
#include "static_base.hpp"

#include "../../common/json_helper.hpp"
#include "../../common/constants.hpp"
#include "../../common/h5_helper.hpp"

#include <iostream>

namespace treap_types {

using namespace common;

AccessionIdSorted::AccessionIdSorted(
    const std::string &req_key, 
    const uint64_t req_database_id, 
    const int64_t req_seq_hash, 
    const int64_t req_metadata_hash
) : BaseSortedTreap(req_key, req_database_id) {
    this->seq_hash = req_seq_hash;
    this->metadata_hash = req_metadata_hash;
};

AccessionIdSorted::AccessionIdSorted(const H5::Group &h5_group) : BaseSortedTreap(h5_group) {
    this->seq_hash = H5Helper::get_uint64_attr_from(h5_group, "seq_hash");
    this->metadata_hash = H5Helper::get_uint64_attr_from(h5_group, "metadata_hash");
}

AccessionIdSorted::~AccessionIdSorted() {}

std::unique_ptr<BaseSortedTreap> AccessionIdSorted::get_unique_from_snapshot_line(const SeqElem &e, const uint64_t req_database_id, const BaseSortedTreap *) {
    return std::make_unique<AccessionIdSorted>(
        e.covv_data[SEQ_FIELDS_TO_ID.at("covv_accession_id")],
        req_database_id,
        get_hash(e.covv_data[SEQ_FIELDS_TO_ID.at("sequence")], 1),
        get_hash(e.covv_data[SEQ_FIELDS_TO_ID.at("covv_accession_id")], get_hash(
                 e.covv_data[SEQ_FIELDS_TO_ID.at("covv_collection_date")], get_hash(
                 e.covv_data[SEQ_FIELDS_TO_ID.at("covv_location")], 1)))
    );
    // std::unique_ptr<BaseSortedTreap> answer_base = std::move(answer);
    // return answer_base;

    // return answer; // set a flag at compiletime to not add 'move'? https://stackoverflow.com/questions/4316727/returning-unique-ptr-from-functions
}

void AccessionIdSorted::reset_get_unique_from_snapshot_line(const ds::PS_Treap*,
                                                            const ds::PS_Treap*,
                                                            const ds::DB *,
                                                            common::SeqElemReader*,
                                                            const std::vector<uint64_t>&, 
                                                            const std::vector<uint64_t>&, 
                                                            std::vector<std::pair<uint64_t, uint64_t>>) {

}

std::vector<uint64_t> AccessionIdSorted::seq_hash_database_ids;
std::vector<uint64_t> AccessionIdSorted::metadata_hash_database_ids;
uint64_t AccessionIdSorted::next_hash_id;

void AccessionIdSorted::reset_get_new_BaseSortedTreap(const H5::Group &group) {
    AccessionIdSorted::next_hash_id = 0;
    AccessionIdSorted::seq_hash_database_ids = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "seq_hash");
    AccessionIdSorted::metadata_hash_database_ids = H5Helper::read_h5_int_to_dataset<uint64_t>(group, "metadata_hash");
    assert(AccessionIdSorted::seq_hash_database_ids.size() == AccessionIdSorted::metadata_hash_database_ids.size());
}

std::unique_ptr<BaseSortedTreap> AccessionIdSorted::get_new_BaseSortedTreap(const std::string &key, const uint64_t database_id) {
    auto aux = std::make_unique<AccessionIdSorted>(key, database_id, 
                                                                    AccessionIdSorted::seq_hash_database_ids[AccessionIdSorted::next_hash_id], 
                                                                    AccessionIdSorted::metadata_hash_database_ids[AccessionIdSorted::next_hash_id]);
    std::unique_ptr<BaseSortedTreap> answer = std::move(aux);
    AccessionIdSorted::next_hash_id++;
    return answer;
}

void serialize_acc_id_elem_to_hdf5(const std::vector<std::unique_ptr<BaseSortedTreap>> &elems, H5::Group &h5_group) {
    std::vector<std::string> saved_key;
    std::vector<uint64_t> saved_database_id;
    std::vector<uint64_t> saved_seq_hash;
    std::vector<uint64_t> saved_metadata_hash;
    for (uint64_t i = 0; i < elems.size(); ++i) {
        AccessionIdSorted* elem = static_cast<AccessionIdSorted*>(elems[i].get());

        saved_key.push_back(elem->key);
        saved_database_id.push_back(elem->database_id);
        saved_seq_hash.push_back(elem->seq_hash);
        saved_metadata_hash.push_back(elem->metadata_hash);
    }

    H5Helper::write_h5_dataset(saved_key, &h5_group, "key");
    H5Helper::write_h5_int_to_dataset(saved_database_id, &h5_group, "database_id");
    H5Helper::write_h5_int_to_dataset(saved_seq_hash, &h5_group, "seq_hash");
    H5Helper::write_h5_int_to_dataset(saved_metadata_hash, &h5_group, "metadata_hash");
}

std::unique_ptr<BaseSortedTreap> AccessionIdSorted::copy_specialized_static_field(const BaseSortedTreap* oth) {
    const AccessionIdSorted *acc_id_oth = static_cast<const AccessionIdSorted*>(oth);

    std::unique_ptr<BaseSortedTreap> unique_curr = std::make_unique<AccessionIdSorted>(
        acc_id_oth->key,
        acc_id_oth->database_id,
        acc_id_oth->seq_hash,
        acc_id_oth->metadata_hash
    );
    return unique_curr;
}

} // namespace treap_types
