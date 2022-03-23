#include "json_helper.hpp"

#include <fstream>
#include <iostream>

#include "constants.hpp"
#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"

#include "../external_libraries/json.hpp"
using Json = nlohmann::json;

namespace common {

SeqElem SeqElem::operator = (const SeqElem &source) {
    for (uint32_t i = 0; i < SEQ_FIELDS_SZ; ++i) {
        this->covv_data[i] = source.covv_data[i];
    }
    return *this;
}

uint64_t get_hash(std::string s, uint64_t hash) {
    for (const auto c : s) {
        hash = (hash * common::HASH_MOD + c) % MOD1;
    }
    return hash;
}

SeqElem get_SeqElem_from_json(Json j_obj) {
    SeqElem answer;
    for (uint32_t i = 0; i < SEQ_FIELDS_SZ; ++i) {
        answer.covv_data[SEQ_FIELDS_TO_ID.at(SEQ_FIELDS[i])] = j_obj[SEQ_FIELDS[i]];
    }
    return answer;
}

SeqElemReader::~SeqElemReader() {
    f.close();
}

SeqElemReader::SeqElemReader(const std::string &input_path) {
    f.open(input_path, std::ifstream::binary);
    Logger::trace("Getting sequence and metadata from file '" + input_path + "'...");

    finished = false;
    last_id_read = -1;

    f >> j_obj; 
    this->next_elem = get_SeqElem_from_json(j_obj);
}

bool SeqElemReader::end_of_file() {
    return this->finished;
}

common::SeqElem SeqElemReader::get_next() {
    common::SeqElem to_return = this->next_elem;
    ++last_id_read;
    try {
        f >> j_obj;
    }
    catch (Json::exception& e) { // todo - more precise error catch
        this->finished = true;
        this->next_elem = SeqElem();
        return to_return;
    }
    this->next_elem = get_SeqElem_from_json(j_obj);
    return to_return;
}

common::SeqElem SeqElemReader::get_elem(const int32_t id) {
    common::SeqElem elem;
    if (last_id_read > id) {
        Logger::error("Elem reader requests elems in not ascending order.");
    }
    while(!this->finished) {
        elem = get_next();
        if (last_id_read == id) {
            return elem;
        }
    }
    Logger::error("Elem reader got end of file before id=" + std::to_string(id));
}

std::vector<common::SeqElem> SeqElemReader::get_aligned_seq_elements(const uint32_t append_size) {
    std::vector<common::SeqElem> seq_elems;

    for (uint32_t i = 0; i < append_size; ++i) {
        SeqElem seqelem_val = get_next();
        seq_elems.push_back(seqelem_val);
        if (this->finished) {
            break;
        }
    }
    return seq_elems;
}

} // namespace common
