#include "config.hpp"

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "../common/logger.hpp"

namespace cli {

common::Logger logger;

void Config::print_helper(const std::string &prog_name, ModuleType module) {
    switch (module)
    {
        case NO_MODULE:
            fprintf(stderr, "Usage: %s <command> [command specific options]\n\n", prog_name.c_str());
            fprintf(stderr, "Available commands:\n");
            
            fprintf(stderr, "\tadd\t\tAdd snapshots to an already existent DB file. All the given snapshots\n");
            fprintf(stderr, "\t\t\tmust be files in '.provision.json[.gz]' format.\n\n");

            fprintf(stderr, "\tcreate\t\tCreate a new DB file from snapshots in '.provision.json[.gz]' format.\n\n");
            break;

        case ADD:
            fprintf(stderr, "Usage: %s add [options] -i <DB> -o <DB*> FILE1 [[FILE2] ... ]\n"
                            "\t Each input FILE is in '.provision.json' or `.provision.json.xz` format\n"
                            "\t The input DB file must be a '.ge' file specific for this tool\n", prog_name.c_str());

            fprintf(stderr, "Available options for ADD:\n");
            fprintf(stderr, "\t   TODO fill in\n");
            break;
        
        case CREATE:
            fprintf(stderr, "Usage: %s create [options] -o <DB*> FILE1 [[FILE2] ... ]\n"
                            "\t Each input FILE is in '.provision.json' or `.provision.json.xz` format\n", prog_name.c_str());

            fprintf(stderr, "Available options for CREATE:\n");
            fprintf(stderr, "\t   TODO fill in\n");
            break;

        default:
            fprintf(stderr, "TODO: implement");
            break;
    }
}

void Config::print_module() {
    std::cerr << "mode=" << this->module << std::endl;
}

Config::Config(int argc, char *argv[]) {
    if (argc == 1) {
        print_helper(argv[0]);
        logger.error("Invalid cli usage, no any arguments received");
    }

    // parse module from first command line argument
    if (!strcmp(argv[1], "create")) {
        module = CREATE;
    } else if (!strcmp(argv[1], "add")) {
        module = ADD;
    } else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        // print_welcome_message();
        print_helper(argv[0]);
        exit(0);
    } else {
        print_helper(argv[0]);
        logger.error("Invalid cli usage, the requested module '" + std::string(argv[1]) + "' is not recognised");
    }

    // provide help screen for chosen identity
    if (argc == 2) {
        print_helper(argv[0], module);
        exit(-1);
    }

    const auto get_value = [&](int i) {
        assert(i > 0);
        assert(i < argc);

        if (i + 1 == argc) {
            print_helper(argv[0], module);
            logger.error("Error: no value provided could be parsed from " + std::string(argv[i]));
        }
        return argv[i + 1];
    };

    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            logger.set_verbose(true);
        } else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--indb")) {
            indb_filepath = std::string(get_value(i++));
        } else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--outdb")) {
            outdb_filepath = std::string(get_value(i++));
        } else if (argv[i][0] == '-') {
            print_helper(argv[0], module);
            logger.error("\nERROR: Unknown option " + std::string(argv[i]) + "\n\n");
        } else {
            fnames.push_back(argv[i]);
        }
    }
}

} // namespace cli
