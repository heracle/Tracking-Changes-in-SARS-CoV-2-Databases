#include "query.hpp"
#include "config.hpp"

#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"
#include "../ds/ps_treap.hpp"
#include "../ds/database.hpp"
#include "../ds/ctc.hpp"

#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_base.hpp"
#include "../ds/tnode_types/tnode_location.hpp"

#include "../queries/base_query.hpp"
#include "../queries/freq_bp.hpp"

using namespace treap_types;

#include <iostream>
#include <fstream>

namespace cli {

// void add_alters(std::vector<std::pair<uint32_t, uint32_t>> &main_altered_bp, const std::vector<std::pair<uint32_t, uint32_t>> &secondary) {
//     for (uint32_t index_s = 0; index_s < secondary.size(); ++index_s) {
//         bool found = false;
//         for (uint32_t index_main = 0; index_main < main_altered_bp.size(); ++index_main) {
//             if (main_altered_bp[index_main].first == secondary[index_s].first) {
//                 found = true;
//                 main_altered_bp[index_main].second += secondary[index_s].second;
//             }
//         }
//         if (!found) {
//             main_altered_bp.push_back(secondary[index_s]);
//         }
//     }
// }

int query(Config *config) {

    std::ifstream instrm(config->indb_filepath, std::ios::in | std::ios::binary);
    if (!instrm.is_open()) {
        Logger::error("Can't open output file " + config->indb_filepath);
    }

    H5::H5File h5_file_read = H5::H5File(config->indb_filepath, H5F_ACC_RDONLY);    
    ds::CTC *ctc = new ds::CTC(h5_file_read);

    query_ns::BaseQuery *query;
    if (config->query_type == FREQ_BP) {
        query = new query_ns::FreqBpQuery(config->fnames);
    } else {
        Logger::error("Query type not recognised.");
    }

    const std::string treap_name = query->get_treap_name();

    ctc->treaps[treap_name].treap->query_callback_subtree(
        query,
        config->snapshot
    );

    delete ctc;
    query->print_results();
    delete query;

    return 0;

    //  -----------------------------------------------------------
    //  -----------------------------------------------------------
    //  -----------------------------------------------------------
    //  -----------------------------------------------------------
    //  -----------------------------------------------------------

    // for (uint32_t i = 0; i < size_location_data; ++i) {
    //     H5::Group curr_data_group = H5Gopen(location_group.getLocId(), ("static_data" + std::to_string(i)).c_str(), H5P_DEFAULT);
    //     std::unique_ptr<BaseSortedTreap> curr_location = std::make_unique<LocationSorted>(curr_data_group);
    //     curr_data_group.close();

    //     location_data.push_back(std::move(curr_location));
    // }

    // ds::PS_Treap *treap_location = new ds::PS_Treap(location_group, location_data, recompute_location_statistics);

    // tsl::hopscotch_map<std::string, uint32_t> mutation_freq;
    // uint32_t total_mutations = 0;

    // std::vector<std::pair<uint32_t, uint32_t>> answer_altered_bp;

    // std::string &target = config->location_prefix;
    // // std::string lca_key = "";
    // Tnode *lca_tnode = NULL;
    // bool found_first = false;
    // bool before_lca_tnode = true;

    // ctc->treaps["location_treap"].treap->query_callback_subtree(
    //     [&](Tnode *tnode, const BaseSortedTreap *elem_unique) {
    //         // Lambda function to run before going to the left child. 

    //         const LocationSorted* elem = static_cast<const LocationSorted*>(elem_unique);

    //         // std::cerr << "fst target=" <<  target << " tnode_key=" << elem->key << "\n";

    //         // find lcp between elem->key and config->location
    //         uint32_t lcp = 0;
    //         for (lcp = 0; lcp < target.size() && lcp < elem->key.size(); ++lcp) {
    //             if (target[lcp] != elem->key[lcp]) {
    //                 break;
    //             }
    //         }

    //         // std::cerr << "\t fst lcp=" << lcp << " ";

    //         if (lca_tnode == NULL && lcp == target.size()) {
    //             // std::cerr << " --- set lca tnode\n";
    //             lca_tnode = tnode;
    //             // lca_key = elem->key;
    //             return 0;
    //         }
    //         // std::cerr << "\n";

    //         if (elem->key < target) {
    //             return 1; // go directly to the right child.
    //         }
    //         return 0;
    //     },
    //     [&](Tnode *tnode, const BaseSortedTreap *elem_unique) {
    //         // Lambda function to run after processing the left child.

    //         const LocationSorted* elem = static_cast<const LocationSorted*>(elem_unique);

    //         // std::cerr << "scd target=" <<  target << " tnode_key=" << elem->key << "\n";

    //         // find lcp between elem->key and config->location
    //         uint32_t lcp = 0;
    //         for (lcp = 0; lcp < target.size() && lcp < elem->key.size(); ++lcp) {
    //             if (target[lcp] != elem->key[lcp]) {
    //                 break;
    //             }
    //         }

    //         if (elem->key > target && lcp != target.size()) {
    //             // std::cerr << "\t elem key greater than target\n";
    //             return -1;
    //         }

    //         if (elem->key >= target) {
    //             // std::cerr << "\t found first";
    //             found_first = true;
    //         }

    //         if (found_first == false) {
    //             // std::cerr << "\t go to the right child\n";
    //             return 1; // go to the right child.
    //         }
    //         // std::vector<std::string> curr_mutations = common::get_mutations_from_json_str(elem->mutations);
    //         add_alters(answer_altered_bp, elem->bp_alterations);

    //         // for (const std::string mutation : curr_mutations) {
    //         //     mutation_freq[mutation] += 1;
    //         // }
    //         // total_mutations += curr_mutations.size();

    //         assert (lca_tnode != NULL);

    //         // std::cerr << "\n";

    //         if (tnode == lca_tnode) {
    //             before_lca_tnode = false;
    //             return 1; // go to the right child
    //         }

    //         if (before_lca_tnode && tnode->r != NULL) {
    //             // get everything from tnode->r 's subtree.
    //             const LocationTnode *right_tnode = static_cast<const LocationTnode*>(tnode->r);
    //             add_alters(answer_altered_bp, right_tnode->altered_bp);
    //             // for (auto mutation : tnode->r->mutation_freq) {
    //             //     mutation_freq[mutation.first] += mutation.second;
    //             //     total_mutations += mutation.second;   
    //             // }
    //         } 
    //         if (!before_lca_tnode && tnode->l != NULL) {
    //             // get everything from tnode->l 's subtree.
    //             const LocationTnode *left_tnode = static_cast<const LocationTnode*>(tnode->l);
    //             add_alters(answer_altered_bp, left_tnode->altered_bp);
    //             // for (auto mutation : tnode->l->mutation_freq) {
    //             //     mutation_freq[mutation.first] += mutation.second;
    //             //     total_mutations += mutation.second;   
    //             // }
    //         }
    //         return 1; 
    //     },
    //     config->snapshot
    // );

    // delete ctc;

    // std::cout << "\n\n\n\nanswer_altered_bp size=" << answer_altered_bp.size() << std::endl;
    // for (const auto &pr : answer_altered_bp) {
    //     std::cout << pr.first << "\t" << pr.second << std::endl;
    // }

    // // function for skipping subtree
    // // function as callback for all the useful tnodes.

    // return 0;
}

} // namespace cli

// (base) [rmuntean@eu-login-06 build]$ bsub -J test_create_1kk -W 4:00 -R "rusage[mem=180000]" ./run create -o /cluster/scratch/rmuntean/gisaid_data/june_1kk_test /cluster/scratch/rmuntean/gisaid_data/2021-06-27_1kk_aligned.provision.json -v^C

// (base) [rmuntean@eu-login-06 build]$ bsub -J test_append_1kk -w "done(test_create_1kk)" -W 4:00 -R "rusage[mem=185000]" -n 1 ./run append -i /cluster/scratch/rmuntean/gisaid_data/june_1kk_test.h5 -o /cluster/scratch/rmuntean/gisaid_data/june_july_1kk_test /cluster/scratch/rmuntean/gisaid_data/2021-07-11_1kk_aligned.provision.json -v

// (base) [rmuntean@eu-login-06 build]$ bsub -J test_query_1kk -w "done(test_append_1kk)" -W 4:00 -R "rusage[mem=185000]" -n 1 ./run query -i /cluster/scratch/rmuntean/gisaid_data/june_july_1kk_test.h5 --location "" -v
