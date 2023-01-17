#pragma once

#include "../external_libraries/hopscotch-map/include/tsl/hopscotch_map.h"

namespace common {

const int SEED = 2021;

const uint64_t MOD1 = 10000000000000061LL;
const uint64_t MOD2 = 10000000000000069LL;
const uint64_t HASH_MOD = 257; // make sure that MOD1 * HASH_MOD does not exceed long long.
const uint64_t BITS_FOR_STEPS_BACK = 8; // assume that there will be at most 2**8 = 256 updates of the same sequence; Can be increased while "ALIGNED_SEQ_SIZE<<BITS_FOR_STEPS_BACK" fits into 32 bits.

// TODO compute it according to RAM available.
const uint64_t H5_APPEND_SIZE = 200;
const uint64_t ALIGNED_SEQ_SIZE = 30000;
const uint64_t H5_CHUNK_SIZE = 10 * ALIGNED_SEQ_SIZE;

const std::string DS_FILE_FORMAT = ".h5";

const uint64_t SEQ_FIELDS_SZ = 6;
const std::vector<std::string> SEQ_FIELDS = {
    "covv_accession_id",
    "covv_collection_date",
    "covv_location",
    "sequence",
    "owner",
    "covv_host"
};

const tsl::hopscotch_map<std::string, uint64_t> SEQ_FIELDS_TO_ID = {
    {"covv_accession_id", 0},
    {"covv_collection_date", 1},
    {"covv_location", 2},
    {"sequence", 3},
    {"owner", 4},
    {"covv_host", 5}
};

void validate_seq_fields();

} // namespace common
