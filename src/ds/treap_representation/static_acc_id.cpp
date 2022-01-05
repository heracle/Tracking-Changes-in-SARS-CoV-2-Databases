#include "static_acc_id.hpp"
#include "static_base.hpp"

#include "../../common/utils.hpp"
#include "../../common/constants.hpp"
#include "../../common/h5_helper.hpp"

namespace treap_types {

AccessionIdSorted::AccessionIdSorted(
    const std::string &req_key, 
    const uint32_t req_database_id, 
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

std::unique_ptr<BaseSortedTreap> AccessionIdSorted::get_unique_from_SeqElem(const common::SeqElem &e, const uint32_t req_database_id) {
    return std::make_unique<AccessionIdSorted>(
        e.covv_accession_id,
        req_database_id,
        common::get_hash(e.sequence, 1),
        common::get_hash(e.covv_accession_id, common::get_hash(e.covv_collection_date, common::get_hash(e.covv_location, 1)))
    );
    // std::unique_ptr<BaseSortedTreap> answer_base = std::move(answer);
    // return answer_base;

    // return answer; // set a flag at compiletime to not add 'move'? https://stackoverflow.com/questions/4316727/returning-unique-ptr-from-functions
}

void recompute_acc_id_statistics(common::Tnode *, const std::unique_ptr<BaseSortedTreap> &) {}

void append_acc_id_elem_to_hdf5(const std::unique_ptr<BaseSortedTreap> &elem_unique, H5::Group &elem_group) {
    AccessionIdSorted* elem = static_cast<AccessionIdSorted*>(elem_unique.get());

    // todo don't duplicate the set of key and database_id. 
    H5Helper::set_uint64_hdf5_attr(elem->metadata_hash, &elem_group, "metadata_hash");
    H5Helper::set_uint64_hdf5_attr(elem->seq_hash, &elem_group, "seq_hash");
    H5Helper::set_string_hdf5_attr(elem->key, &elem_group, "key");
    H5Helper::set_uint32_hdf5_attr(elem->database_id, &elem_group, "database_id");
}

} // namespace treap_types
