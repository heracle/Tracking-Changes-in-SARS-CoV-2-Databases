#pragma once

#include <vector>
#include <string>
#include <random>

#include "../external_libraries/hopscotch-map/include/tsl/hopscotch_map.h"
#include "../ds/treap_representation/static_base.hpp"

#include "H5Cpp.h"
#include "../external_libraries/json.hpp"
using Json = nlohmann::json;

namespace common {

struct SeqElem {
    std::string covv_accession_id;
    std::string covv_collection_date;
    std::string covv_location;
    std::string sequence;
    // todo delete 'covsurver_prot_mutations'
    std::string covsurver_prot_mutations;
    // std::string lab_owner;

    SeqElem();
    static SeqElem get_from_hdf5(const H5::Group &group);
};

class Tnode {
  //todo change this "public"
  public:
    uint32_t data_id;
    unsigned long long prio;

    static const int rand_pool_size = (1 << 16);
    static int next_rand_idx;
    static std::vector<unsigned long long> rand_values;

    static unsigned long long get_rand_ull();

    Tnode *l;
    Tnode *r;

    // uint32_t total_mutations;
    // tsl::hopscotch_map<std::string, uint32_t> mutation_freq;
    
    Tnode(const int index);
    Tnode(const Tnode &oth);
    
    // This constructor is not initializing 'r' and 'l' fields!
    Tnode(const H5::Group &tnode_group);
};

uint64_t get_hash(std::string s, uint64_t hash);
common::SeqElem get_SeqElem_from_json(Json j_obj);
std::vector<std::string> get_mutations_from_json_str(const std::string &mutation_str);
std::vector<common::SeqElem> get_aligned_seq_elements(const std::string &input_path);

} // namespace common
