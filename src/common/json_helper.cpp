#include "json_helper.hpp"

#include <fstream>
#include <iostream>

#include "constants.hpp"
#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"

#include "../external_libraries/rapid_json/include/rapidjson/document.h"
#include "../external_libraries/xxhash/xxhash.h"

namespace common {

SeqElem SeqElem::operator = (const SeqElem &source) {
    for (uint32_t i = 0; i < SEQ_FIELDS_SZ; ++i) {
        this->covv_data[i] = source.covv_data[i];
    }
    return *this;
}

uint64_t get_hash(std::string s, uint64_t seed) {
    return XXH3_64bits_withSeed(s.c_str(), s.size(), seed);
}

SeqElem get_SeqElem_from_json(rapidjson::Document &j_obj) {
    SeqElem answer;
    for (uint32_t i = 0; i < SEQ_FIELDS_SZ; ++i) {
        answer.covv_data[SEQ_FIELDS_TO_ID.at(SEQ_FIELDS[i])] = j_obj[SEQ_FIELDS[i].c_str()].GetString();
    }
    answer.prv_db_id = 0;
    return answer;
}

SeqElemReader::~SeqElemReader() {
    f.close();
}

SeqElemReader::SeqElemReader(const std::string &input_path) {
    f.open(input_path);
    Logger::trace("Getting sequence and metadata from file '" + input_path + "'...");

    finished = false;
    last_id_read = -1;

    std::string json_line;
    std::getline(f, json_line);
    document.Parse(json_line.c_str());
    this->next_elem = get_SeqElem_from_json(document);
}

bool SeqElemReader::end_of_file() {
    return this->finished;
}

common::SeqElem SeqElemReader::get_next() {
    common::SeqElem to_return = this->next_elem;
    ++last_id_read;
    std::string json_line = "";
    while (json_line == "") {
        if (! std::getline(f, json_line)) {
            this->finished = true;
            this->next_elem = SeqElem();
            return to_return;
        }
    }

    document.Parse(json_line.c_str());
    this->next_elem = get_SeqElem_from_json(document);
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
