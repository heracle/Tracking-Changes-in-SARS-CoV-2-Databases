#pragma once

#include "../external_libraries/hopscotch-map/include/tsl/hopscotch_map.h"

#include "../external_libraries/json.hpp"
using Json = nlohmann::json;

namespace common {

const int SEED = 2021;

const uint64_t MOD1 = 10000000000000061LL;
const uint64_t MOD2 = 10000000000000069LL;
const uint64_t HASH_MOD = 257; // make sure that MOD1 * HASH_MOD does not exceed long long.

// todo understand more exactly what 'H5_CHUNK_SIZE' is doing. Currently set to be equal to unix page size.
const uint32_t H5_APPEND_SIZE = 10000;
const uint32_t ALIGNED_SEQ_SIZE = 40000;
const uint32_t H5_CHUNK_SIZE = 100 * ALIGNED_SEQ_SIZE;

const std::string DS_FILE_FORMAT = ".h5";

const uint32_t SEQ_FIELDS_SZ = 4;
const std::vector<std::string> SEQ_FIELDS = {
    "covv_accession_id",
    "covv_collection_date",
    "covv_location",
    "sequence"
};

const tsl::hopscotch_map<std::string, uint32_t> SEQ_FIELDS_TO_ID = {
    {"covv_accession_id", 0},
    {"covv_collection_date", 1},
    {"covv_location", 2},
    {"sequence", 3}
};

void validate_seq_fields();

// const std::string metadata_strings[] = {
//     "covv_accession_id",
//     "covv_clade",
//     "covv_collection_date",
//     "covsurver_prot_mutations",
//     "covsurver_uniquemutlist",

//     "covv_add_host_info",
//     "covv_patient_age",
//     "covv_gender",
//     "covv_host",

//     "covv_lineage",
//     "covv_sampling_strategy",
//     "pangolin_lineages_version",
//     "covv_location",

//     "covv_subm_date",
//     "covv_type",
//     "covv_variant",
//     "covv_virus_name"
// };

// const std::string metadata_doubles[] = {
//     "gc_content",
//     "n_content",
//     "sequence_length"
// };

// const std::string metadata_bool[] = {
//     "is_high_coverage",
//     "is_reference",
//     "is_complete"
// };

} // namespace common
