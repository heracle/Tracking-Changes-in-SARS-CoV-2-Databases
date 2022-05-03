#include "config.hpp"

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <sys/stat.h>

#include "../common/constants.hpp"
#include "../common/logger.hpp"

namespace cli {

void Config::print_helper(const std::string &prog_name, ModuleType module) {
    switch (module)
    {
        case NO_MODULE:
            fprintf(stderr, "Usage: %s <command> [command specific options]\n\n", prog_name.c_str());
            fprintf(stderr, "Available commands:\n");
            
            fprintf(stderr, "\tappend\t\tAppend snapshots to an already existent DB file. All the given snapshots\n");
            fprintf(stderr, "\t\t\tmust be files in '.provision.json[.gz]' format.\n\n");

            fprintf(stderr, "\tcreate\t\tCreate a new DB file from snapshots in '.provision.json[.gz]' format.\n\n");

            fprintf(stderr, "\tstats\t\tList details regarding an existent DB file: names of stored snapshots,\n");
            fprintf(stderr, "\t\t\ttotal number of nodes, total number of database elements.\n\n");

            fprintf(stderr, "\tquery\t\tRun queries todo add more details.\n\n");
            break;

        case APPEND:
            fprintf(stderr, "Usage: %s append [options] -i <DB> -o <DB*> FILE1 [[FILE2] ... ]\n"
                            "\t Each input FILE is in '.provision.json' or `.provision.json.xz` format\n"
                            "\t The input DB file must be a '.ctc' file specific for this tool\n", prog_name.c_str());

            fprintf(stderr, "Available options for APPEND:\n");
            fprintf(stderr, "\t   TODO fill in\n");
            break;
        
        case CREATE:
            fprintf(stderr, "Usage: %s create [options] -o <DB*> FILE1 \n"
                            "\t Each input FILE is in '.provision.json' or `.provision.json.xz` format\n", prog_name.c_str());

            fprintf(stderr, "Available options for CREATE:\n");
            fprintf(stderr, "\t   TODO fill in\n");
            break;

        case STATS:
            // todo integration tests for both -i <DB*> and simple <DB*>.
            fprintf(stderr, "Usage: %s stats <DB*> \n"
                            "\t The input DB file must be a '.ctc' file specific for this tool\n", prog_name.c_str());

            fprintf(stderr, "Available options for STATS:\n");
            fprintf(stderr, "\t   TODO fill in\n");
            break;

        case QUERY:
            fprintf(stderr, "Usage: %s query [options] -q <query_type> -i <DB*> [location_prefix] \n"
                            "\t The input DB file must be a '.h5' file specific for this tool\n", prog_name.c_str());

            fprintf(stderr, "\n\nAvailable query tyes: 'bp_freq', 'cnt_indels'");

            fprintf(stderr, "\nAvailable options for query bp_freq:\n");
            fprintf(stderr, "\t   --snapshot [str] \t Indicate what snapshot to use as target for the query.\n");
            fprintf(stderr, "\t   --compute-total-owner-cnt \t print how many uploads has each owner group.\n");
            fprintf(stderr, "\t   --top-to-print [int] \t Set the number of results to print.\n");
            fprintf(stderr, "\t   --exclude-deleted [int] \t Exclude the results from the previously deleted sequences.\n");

            fprintf(stderr, "\nAvailable options for query cnt_indels:\n");
            fprintf(stderr, "\t   --snapshot [str,str,..] \t Compute one column for each requested snapshot (e.g. '--snapshot snap1,snap2,snap3' with no spaces).\n");

            break;
            
        default:
            fprintf(stderr, "TODO: implement");
            break;
    }
}

Config::Config(int argc, char *argv[]) {
    if (argc == 1) {
        print_helper(argv[0]);
        Logger::error("Invalid cli usage, no any arguments received");
    }

    // parse module from first command line argument
    if (!strcmp(argv[1], "create")) {
        module = CREATE;
    } else if (!strcmp(argv[1], "append")) {
        module = APPEND;
    } else if (!strcmp(argv[1], "stats")) {
        module = STATS;
    } else if (!strcmp(argv[1], "query")) {
        module = QUERY;
    } else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        print_helper(argv[0]);
        exit(0);
    } else {
        print_helper(argv[0]);
        Logger::error("Invalid cli usage, the requested module '" + std::string(argv[1]) + "' is not recognised");
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
            Logger::error("Error: no value provided could be parsed from " + std::string(argv[i]));
        }
        return argv[i + 1];
    };

    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            Logger::set_verbose(true);
        } else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--indb")) {
            indb_filepath = std::string(get_value(i++));
        } else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--outdb")) {
            outdb_filepath = std::string(get_value(i++));
        } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            verbosity = true;
        } else if (!strcmp(argv[i], "--compute-total-owner-cnt")) {
            compute_total_owner_cnt = true;
        } else if (!strcmp(argv[i], "--exclude-deleted")) {
            exclude_deleted = true;
        } else if (!strcmp(argv[i], "--num-to-print")) {
            num_to_print = std::atol(get_value(i++));
        } else if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--query")) {
            std::string aux = std::string(get_value(i++));
            if (aux == "bp_freq") {
                query_type = QueryType::FREQ_BP;
            } else if (aux == "cnt_indels") {
                query_type = QueryType::CNT_INDELS;
            } else {
                print_helper(argv[0], module);
                Logger::error("\nERROR: Unknown query type " + aux + "\n\n");
            }
        } else if (!strcmp(argv[i], "--snapshot")){
            snapshot = std::string(get_value(i++));
        } else if (!strcmp(argv[i], "--location")) {
            location_prefix = std::string(get_value(i++));
        } else if (argv[i][0] == '-') {
            print_helper(argv[0], module);
            Logger::error("\nERROR: Unknown option " + std::string(argv[i]) + "\n\n");
        } else {
            fnames.push_back(argv[i]);
        }
    }

    validate_config();
}

void Config::validate_config() {
    /*
     * this.module was already validated in the constructor.
    */
    assert(module != ModuleType::NO_MODULE);

    /* ------------------------------------------------------------------------
     * Validate indb_filepath.
    */
    if (module == ModuleType::STATS) {
        if (indb_filepath == "") {
            // "-i" was missed, thus, get the ctc_filepath from fnames[0].
            if (fnames.size() == 0) {
                Logger::error("STATS cli cmd must receive exactly one indb_filepath in '" + common::DS_FILE_FORMAT + "' format.");
            }
            if (fnames.size() > 1) {
                Logger::error("STATS cli cmd cannot receive more than one indb_filepath.");
            }
            indb_filepath = fnames[0];
        }
    }
    if (module == ModuleType::STATS || module == ModuleType::APPEND || module == ModuleType::QUERY) {
        if (indb_filepath == "") {
            Logger::error("Missing input db filepath.");
        }
        if (
            indb_filepath.size() < common::DS_FILE_FORMAT.size() &&
            indb_filepath.substr(indb_filepath.size() - common::DS_FILE_FORMAT.size()) != common::DS_FILE_FORMAT
        ) {
            Logger::error("The input db filepath must be in '" + common::DS_FILE_FORMAT +
                          "' format. Received " + indb_filepath);
        }
        if (access(indb_filepath.c_str(), F_OK ) == -1) {
            Logger::error("The input db filepath cannot be accessed " + indb_filepath);
        }
    }

    /* ------------------------------------------------------------------------
     * Validate outdb_filepath.
    */
    if (module == ModuleType::APPEND || module == ModuleType::CREATE) {
        if (outdb_filepath == "") {
            Logger::error("Missing output db filepath.");
        }
        // Append 'DS_FILE_FORMAT' to the filepath only if it is missing.
        if (outdb_filepath.substr(outdb_filepath.size() - common::DS_FILE_FORMAT.size()) != common::DS_FILE_FORMAT) {
            outdb_filepath += common::DS_FILE_FORMAT;
        }
    }
    /* ------------------------------------------------------------------------
     * validate not void query
    */
    if (module == ModuleType::QUERY) {
        if (query_type == QueryType::NO_QUERY) {
            Logger::error("QUERY cli cmd must specify what query to run (use '-q' flag).");
        }
    }

    /* ------------------------------------------------------------------------
     * Validate fnames.
    */
    if (module == ModuleType::CREATE) {
        if (fnames.size() != 1) {
            Logger::error("CREATE cli cmd must receive exactly one file to process (in '" + common::DS_FILE_FORMAT + "' format).");
        }
    }
    if (module == ModuleType::APPEND) {
        if (fnames.size() < 1) {
            Logger::error("APPEND cli cmd must receive at least one file to process (in '" + common::DS_FILE_FORMAT + "' format).");
        }
    }

    if(module != ModuleType::QUERY) {
        for (const std::string &filepath : fnames) {
            if (access(filepath.c_str(), F_OK ) == -1) {
                Logger::error("One input file cannot be accessed " + filepath);
            }
        }
    }    
}

} // namespace cli
