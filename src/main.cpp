#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <string>
#include <sstream>
#include <memory>

#include "external_libraries/json.hpp"
#include "external_libraries/hopscotch-map/include/tsl/hopscotch_map.h"

#include "common/constants.hpp"
#include "cli/config.hpp"
#include "common/logger.hpp"

common::Logger logger;

int main(int argc, char *argv[]) {
    auto config = std::make_unique<cli::Config>(argc, argv);

    config->print_module();

    // logger->set_level(common::get_verbose()
    //                         ? spdlog::level::trace
    //                         : spdlog::level::info);

    logger.trace("Tool started");

    // switch (config->identity) {
    //     case cli::ADD_SNAPSHOT:
    //         return cli::build_graph(config.get());

    //     case cli::NO_MODULE:
    //         assert(false);
    // }

    return 0;
}
