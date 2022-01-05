#include "static_location.hpp"
#include "static_base.hpp"
#include <iostream>

#include "../../common/utils.hpp"
#include "../../common/constants.hpp"
#include "../../common/h5_helper.hpp"

namespace treap_types {

LocationSorted::LocationSorted(
    const std::string &req_key, 
    const uint32_t req_database_id, 
    const std::string &req_mutation
) : BaseSortedTreap(req_key, req_database_id) {
    this->mutations = req_mutation;
};

LocationSorted::LocationSorted(const H5::Group &h5_group) : BaseSortedTreap(h5_group) {
    // this->mutations = proto.mutations();
}

LocationSorted::~LocationSorted() {}

std::unique_ptr<BaseSortedTreap> LocationSorted::get_unique_from_SeqElem(const common::SeqElem &e, const uint32_t req_database_id) {
    return std::make_unique<LocationSorted>(
        e.covv_location + "_$_" + e.covv_collection_date,
        req_database_id,
        e.covsurver_prot_mutations
    );
    // answer->mutations = e.covsurver_prot_mutations;
    // return answer; // set a flag at compiletime to not add 'move'? https://stackoverflow.com/questions/4316727/returning-unique-ptr-from-functions
}

void recompute_location_statistics(common::Tnode *tnode, const std::unique_ptr<BaseSortedTreap> &elem_unique) {
    // std::cerr << "before recompute_location_statistics" << std::endl;
    // take the info from the 2 children.
    // tnode->mutation_freq.clear(); // maybe not needed.
    // tnode->total_mutations = 0;
    // if (tnode->l != NULL) {
    //     for (const std::pair<std::string, uint32_t> &mutation : tnode->l->mutation_freq) {
    //         tnode->mutation_freq[mutation.first] += mutation.second;
    //     }
    //     tnode->total_mutations += tnode->l->total_mutations;
    // }
    // if (tnode->r != NULL) {
    //     for (const std::pair<std::string, uint32_t> &mutation : tnode->r->mutation_freq) {
    //         tnode->mutation_freq[mutation.first] += mutation.second;
    //     }
    //     tnode->total_mutations += tnode->r->total_mutations;
    // }
    // take the info from the static data id.
    // std::vector<std::string> mutations = common::get_mutations_from_json_str(static_cast<LocationSorted*>(elem_unique.get())->mutations);

    // for (const std::string &mutation : mutations) {
    //     tnode->mutation_freq[mutation] += 1;
    // }
    // tnode->total_mutations += mutations.size();
    // std::cerr << "after recompute_location_statistics" << std::endl;
}

void append_location_elem_to_hdf5(const std::unique_ptr<BaseSortedTreap> &elem_unique, H5::Group &elem_group) {
    // get a normal pointer from elem and cast it to LocationSorted.
    LocationSorted* elem = static_cast<LocationSorted*>(elem_unique.get());

    H5Helper::set_string_hdf5_attr(elem->key, &elem_group, "key");
    H5Helper::set_uint32_hdf5_attr(elem->database_id, &elem_group, "database_id");
}

} // namespace treap_types
