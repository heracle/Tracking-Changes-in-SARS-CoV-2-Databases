#pragma once

#include <string>
#include <vector>

namespace cli {

enum ModuleType {
    NO_MODULE = -1,
    CREATE = 0,
    ADD = 1,
};

class Config {
  ModuleType module;
  std::string indb_filepath;
  std::string outdb_filepath;
  std::vector<std::string> fnames;
  public:
    Config(int argc, char *argv[]);
    void print_helper(const std::string &prog_name, ModuleType module = NO_MODULE);
    void print_module();
};

} // namespace cli
