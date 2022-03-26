#pragma once

#include "static_base.hpp"
#include "../ps_treap.hpp"
#include "../database.hpp"
#include "../tnode_types/tnode_base.hpp"

#include "../../common/json_helper.hpp"
#include "H5Cpp.h"

namespace treap_types {

class AccessionIdSorted : public BaseSortedTreap {
  public:
    std::uint64_t seq_hash;
    std::uint64_t metadata_hash;

    AccessionIdSorted(
        const std::string &req_key, 
        const uint32_t req_database_id,
        const int64_t req_seq_hash, 
        const int64_t req_metadata_hash
    );
    AccessionIdSorted(const H5::Group &h5_group);
    ~AccessionIdSorted();

    static std::unique_ptr<BaseSortedTreap> get_unique_from_SeqElem(const common::SeqElem &e, const uint32_t req_database_id, const bool reinsert = false);
    
    /*
     TODO FIX: because of this static function we have a problem when defining 2 different AccessionId treaps.
     We have this usecase in append.cpp where we need 2 AccessionId treaps to compare the old and the new state.

      Currently, all the AccessionId treaps are using the same static fields.
      We should analyse if any non void definition of 'reset_get_unique_from_SeqElem' can cause overlapping.
    */
    static void reset_get_unique_from_SeqElem(const ds::PS_Treap *accid_base_treap,
                                              const ds::PS_Treap *accid_snapshot_treap,
                                              const ds::DB *base_db,
                                              common::SeqElemReader*,
                                              const std::vector<uint32_t> &insertions_db_ids, 
                                              const std::vector<uint32_t> &deletions_db_ids, 
                                              std::vector<std::pair<uint32_t, uint32_t>> updates_db_ids);

    static std::vector<uint64_t> seq_hash_database_ids;
    static std::vector<uint64_t> metadata_hash_database_ids;
    static uint32_t next_hash_id;
    static std::unique_ptr<BaseSortedTreap> get_new_BaseSortedTreap(const std::string &key, const uint32_t database_id);
    static void reset_get_new_BaseSortedTreap(const H5::Group &group);
};

void serialize_acc_id_elem_to_hdf5(const std::vector<std::unique_ptr<BaseSortedTreap>> &elems, H5::Group &h5_group);


} // namespace treap_types