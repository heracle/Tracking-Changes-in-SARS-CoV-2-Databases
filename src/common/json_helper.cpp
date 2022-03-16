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
}

bool SeqElemReader::end_of_file() {
    return this->finished;
}

common::SeqElem SeqElemReader::get_next() {
    while (true) {
        try {
            f >> j_obj;
        }
        catch (Json::exception& e) {
            // todo - more precise error catch
            break;
        }
        ++last_id_read;
        return get_SeqElem_from_json(j_obj);
    }
    // fout_fasta.close();
    this->finished = true;
    return common::SeqElem();
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
    if (last_id_read > id) {
        Logger::error("Elem reader got end of file before id=" + std::to_string(id));
    }
}

std::vector<common::SeqElem> SeqElemReader::get_aligned_seq_elements() {
    std::vector<common::SeqElem> seq_elems;

    for (uint32_t i = 0; i < common::H5_APPEND_SIZE; ++i) {
        SeqElem seqelem_val = get_next();
        if (this->finished) {
            break;
        }
        seq_elems.push_back(seqelem_val);
    }
    return seq_elems;
}

std::vector<std::string> get_mutations_from_json_str(const std::string &mutation_str) {
    std::vector<std::string> answer;

    /* 
       we need to save the mutations for this current tnode.
    */
    std::string curr_mutation = "";
    // mutation string contains brackets, e.g. "(NSP15_A283V,NSP12_P323L,Spike_D614G)".
    for (uint32_t i = 1; i < mutation_str.size() - 1; ++i) {
        if (mutation_str[i] == ',') {
            answer.push_back(curr_mutation);
            curr_mutation.clear();
        } else {
            curr_mutation.push_back(mutation_str[i]);
        }
    }
    // add last mutation.
    if (curr_mutation != "") {
        answer.push_back(curr_mutation);
    }

    return answer;
}

} // namespace common
