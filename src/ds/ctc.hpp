#pragma once

#include <fstream>
#include <vector>

#include "ps_treap.hpp"
#include "database.hpp"
#include "H5Cpp.h"

namespace ds {

struct CTCData {
    ds::PS_Treap* treap;
    // Function for creating or copying a specialized tnode.
    std::function<Tnode*(const uint32_t)> create_new_specialized_tnode;
    std::function<Tnode*(const Tnode*)> copy_specialized_tnode;
    // Regarding tnode data, read from h5.
    std::function<Tnode*(const uint32_t, const uint64_t, const uint32_t)> get_h5_tnode;
    std::function<void(const H5::Group&)> reset_get_h5_tnode;
    // Regarding tnode data, write to h5.
    std::function<void(Tnode*)> append_tnode_data;
    std::function<void()> reset_append_tnode_data;
    std::function<void(H5::Group&)> write_tnode_data_to_h5;
    // Recomputation on tnode data (access to BaseSortedTreap).
    std::function<void(Tnode*, const BaseSortedTreap*)> recompute_tnode_statistics;
    
    // Regarding static treap data, read from h5.
    std::function<std::unique_ptr<BaseSortedTreap>(const std::string&, const uint32_t)> get_new_BaseSortedTreap;
    std::function<void(const H5::Group&)> reset_get_new_BaseSortedTreap;
    // Regarding static treap data, read from seqElem.
    std::function<std::unique_ptr<BaseSortedTreap>(const common::SeqElem&, const uint32_t, const bool)> get_unique_from_SeqElem;
    std::function<void(const ds::PS_Treap*, 
                       const ds::PS_Treap*, 
                       const ds::DB *, 
                       common::SeqElemReader*, 
                       const std::vector<uint32_t>&, 
                       const std::vector<uint32_t>&, 
                       std::vector<std::pair<uint32_t, uint32_t>>)> reset_get_unique_from_SeqElem;

    // Regarding static treap data, write to h5
    std::function<void(const std::vector<std::unique_ptr<BaseSortedTreap>>&, H5::Group&)> serialize_elem_to_hdf5;
};

class CTC {
  private:
    void ctc_init();
    H5::H5File &h5file;
  
  public:
    // TODO make this db and treaps private.
    ds::DB *db;
    ds::PS_Treap *hash_treap;
    tsl::hopscotch_map<std::string, CTCData> treaps;
  
    CTC(H5::H5File *h5_file, CTC* source = NULL);
    CTC(H5::H5File &h5_file);
    ~CTC();
    void insert_seq(const std::vector<common::SeqElem> *seq_elems, const bool was_modified);
    void erase_seq(const std::vector<uint32_t> &deletions_db_ids, const bool was_modified);
    void save_snapshot(const std::string &name);
    void export_to_h5();


    void prepare_specific_get_unique_SeqElem(const ds::PS_Treap *accid_base_treap,
                                            const ds::PS_Treap *accid_snapshot_treap,
                                            const DB *base_db,
                                            const std::string &file_name,
                                            const std::vector<uint32_t> &insertions_db_ids, 
                                            const std::vector<uint32_t> &deletions_db_ids, 
                                            const std::vector<std::pair<uint32_t, uint32_t>> &updates_db_ids) const;
};

} // namespace ds