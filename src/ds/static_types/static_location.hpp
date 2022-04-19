#pragma once

#include "static_base.hpp"
#include "../ps_treap.hpp"
#include "../database.hpp"
#include "../../common/json_helper.hpp"
#include "../tnode_types/tnode_base.hpp"

namespace treap_types {

class LocationSorted : public BaseSortedTreap {
  private:
    static std::vector<std::vector<uint32_t>> alteration_list_SeqElem;
    static uint32_t next_alteration_SeqElem_id;
  public:
    std::vector<uint32_t> bp_alterations;

    LocationSorted(const std::string &req_key, 
                   const uint32_t req_database_id, 
                   const std::vector<uint32_t> &req_bp_alterations = std::vector<uint32_t>());
    ~LocationSorted();

    /*
      * get_unique_from_SeqElem returns an unique ptr for a new LocationSorted object according to the arguments.
    */
    static std::unique_ptr<BaseSortedTreap> get_unique_from_snapshot_line(const common::SeqElem &e, const uint32_t req_database_id, const BaseSortedTreap *prv);
    static void reset_get_unique_from_snapshot_line(const ds::PS_Treap *accid_base_treap,
                                                    const ds::PS_Treap *accid_snapshot_treap,
                                                    const ds::DB *base_db,
                                                    common::SeqElemReader *snapshot_reader,
                                                    const std::vector<uint32_t> &insertions_db_ids, 
                                                    const std::vector<uint32_t> &deletions_db_ids, 
                                                    std::vector<std::pair<uint32_t, uint32_t>> updates_db_ids);

    static std::vector<uint64_t> alteration_sizes;
    static std::vector<uint32_t> merged_alterations;
    static uint32_t next_node_id;
    static uint32_t next_merged_id;

    static std::unique_ptr<BaseSortedTreap> get_new_BaseSortedTreap(const std::string &key, const uint32_t database_id);
    static void reset_get_new_BaseSortedTreap(const H5::Group &group);

    static std::unique_ptr<BaseSortedTreap> copy_specialized_static_field(const BaseSortedTreap*);
};

void serialize_location_elem_to_hdf5(const std::vector<std::unique_ptr<BaseSortedTreap>> &elems, H5::Group &h5_group);

} // namespace treap_types