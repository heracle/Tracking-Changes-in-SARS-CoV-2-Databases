#pragma once

const int seed = 15;

const uint64_t mod1 = 100000000000000003LL;
const uint64_t mod2 = 100000000000000013LL;

const std::string metadata_important_fields[] = {
    "covv_accession_id",
    "covv_clade",
    "covv_collection_date",
    "covv_location",
    "covv_type",
    "covv_variant",
    "covv_virus_name",
    "sequence_length",
    "gc_content",
    "sequence"
};

const std::string metadata_strings[] = {
    "covv_accession_id",
    "covv_clade",
    "covv_collection_date",
    "covsurver_prot_mutations",
    "covsurver_uniquemutlist",

    "covv_add_host_info",
    "covv_patient_age",
    "covv_gender",
    "covv_host",

    "covv_lineage",
    "covv_sampling_strategy",
    "pangolin_lineages_version",
    "covv_location",

    "covv_subm_date",
    "covv_type",
    "covv_variant",
    "covv_virus_name"
};

const std::string metadata_doubles[] = {
    "gc_content",
    "n_content",
    "sequence_length"
};

const std::string metadata_bool[] = {
    "is_high_coverage",
    "is_reference",
    "is_complete"
};
