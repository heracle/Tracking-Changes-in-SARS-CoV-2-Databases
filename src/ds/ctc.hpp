#pragma once

#include <fstream>
#include <vector>

#include "ps_treap.hpp"
#include "database.hpp"
#include "H5Cpp.h"

namespace ds {

class CTC {
  public:
    ds::DB *db;
    ds::PS_Treap *treap_location;
    ds::PS_Treap *treap_acc_id;

    CTC();
    CTC(H5::H5File &h5_file);
    ~CTC();
    void insert_and_clear_ram(std::vector<common::SeqElem> *seq_elems);
    void erase(const std::vector<uint32_t> &deletions_db_ids);
    void save_snapshot(const std::string &name);
    void export_to_h5(H5::H5File &h5_file);
};

} // namespace ds