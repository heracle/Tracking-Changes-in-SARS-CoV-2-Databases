#pragma once

#include <string>
#include <vector>

namespace cli {

enum ModuleType {
    NO_MODULE = -1,
    CREATE = 0,
    APPEND = 1,
    STATS = 2,
    QUERY = 3,
};

enum QueryType {
    NO_QUERY = -1,
    FREQ_BP = 0,
    CNT_INDELS = 1,
    CNT_HOSTS = 2,
};

class Config {
  public:
    ModuleType module = ModuleType::NO_MODULE;
    QueryType query_type = QueryType::NO_QUERY;
    std::string indb_filepath = "";
    std::string outdb_filepath = "";
    std::vector<std::string> fnames;
    std::string snapshot = "";
    bool verbosity = false;
    bool compute_total_owner_cnt = false;
    bool exclude_deleted = false;
    uint64_t num_to_print = 50;

    std::string location_prefix = "";
  
    Config(int argc, char *argv[]);
    void print_helper(const std::string &prog_name, ModuleType module = NO_MODULE);
    /*
      Run validations on each field of Config object according to the current module.
    */
    void validate_config();
};

} // namespace cli
