#include "common/constants.hpp"
#include "cli/config.hpp"
#include "common/logger.hpp"

#include "cli/create.hpp"
#include "cli/append.hpp"
#include "cli/stats.hpp"
#include "cli/query.hpp"

int main(int argc, char *argv[]) {
    srand(common::SEED);
    auto config = std::make_unique<cli::Config>(argc, argv);

    Logger::trace("Tool started");

    switch (config->module) {
        case cli::CREATE:
            return cli::create(config.get());

        case cli::APPEND:
            return cli::append(config.get());

        case cli::STATS:
            return cli::stats(config.get());
        
        case cli::QUERY:
            return cli::query(config.get());

        case cli::NO_MODULE:
            assert(false);
    }

    return 0;
}
