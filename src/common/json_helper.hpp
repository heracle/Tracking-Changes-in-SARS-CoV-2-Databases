#pragma once

#include <vector>
#include <string>
#include <random>
#include <fstream>

#include "constants.hpp"

#include "../external_libraries/hopscotch-map/include/tsl/hopscotch_map.h"
#include "../ds/static_types/static_base.hpp"

#include "H5Cpp.h"

#include "../external_libraries/rapid_json/include/rapidjson/document.h"

namespace common {

struct SeqElem {
    // covv_data size will be equal to constants.hpp/SEQ_FIELDS
    std::string covv_data[SEQ_FIELDS_SZ];    
    // todo add std::string lab_owner;

    SeqElem operator = (const SeqElem &source);
};

/* 
  * Compute a hash on an extendable string. Considering that the received 'hash' is corresponding to a string 'v',
  the returned value will be the hash corresponding to the concatenation 'v + s':
  * e.g. get_hash("abc", get_hash("def", 0)) = get_hash("defabc", 0);
 */
uint64_t get_hash(std::string s, uint64_t hash);
/*
 * get_SeqElem_from_json returns a SeqElem object from json
 */
SeqElem get_SeqElem_from_json(rapidjson::Document &j_obj);

/*
 * SeqElemReader is a handler for reading the snapshot file.
 */
class SeqElemReader {
  private:
    rapidjson::Document document;
    std::ifstream f;
    bool finished;
    int32_t last_id_read;

    // Always keep the next SeqElem in 'next_elem' for being one step ahead in file and keep 'end_of_file()' up to date.
    SeqElem next_elem;
    SeqElem get_next();

  public:
    std::vector<SeqElem> get_aligned_seq_elements(const uint32_t append_size = common::H5_APPEND_SIZE);
    SeqElemReader(const std::string &input_path);
    ~SeqElemReader();
    bool end_of_file();

    SeqElem get_elem(const int32_t id);
};

} // namespace common
