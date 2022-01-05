#include "query.hpp"
#include "config.hpp"

#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"
#include "../ds/ps_treap.hpp"
#include "../ds/database.hpp"

#include "../ds/treap_representation/static_base.hpp"
#include "../ds/treap_representation/static_location.hpp"
using namespace treap_types;

#include <iostream>
#include <fstream>

namespace cli {

int query(Config *config) {

    std::ifstream instrm(config->indb_filepath, std::ios::in | std::ios::binary);
    if (!instrm.is_open()) {
        Logger::error("Can't open output file " + config->indb_filepath);
    }

    H5::H5File h5_file_read = H5::H5File(config->indb_filepath, H5F_ACC_RDONLY); 
    // ds::DB *db = new ds::DB(database_group);

    // read data before calling the constructor
    std::vector<std::unique_ptr<BaseSortedTreap>> location_data;
    H5::Group location_group = H5Gopen(h5_file_read.getLocId(), "/location_treap", H5P_DEFAULT);
    uint32_t size_location_data = H5Helper::get_uint32_attr_from(location_group, "static_data_size");

    for (uint32_t i = 0; i < size_location_data; ++i) {
        H5::Group curr_data_group = H5Gopen(location_group.getLocId(), ("static_data" + std::to_string(i)).c_str(), H5P_DEFAULT);
        std::unique_ptr<BaseSortedTreap> curr_location = std::make_unique<LocationSorted>(curr_data_group);
        curr_data_group.close();

        location_data.push_back(std::move(curr_location));
    }

    ds::PS_Treap *treap_location = new ds::PS_Treap(location_group, location_data, recompute_location_statistics);

    tsl::hopscotch_map<std::string, uint32_t> mutation_freq;
    uint32_t total_mutations = 0;

    std::string &target = config->location_prefix;
    // std::string lca_key = "";
    common::Tnode *lca_tnode = NULL;
    bool found_first = false;
    bool before_lca_tnode = true;

    treap_location->query_callback_subtree(
        [&](common::Tnode *tnode, const BaseSortedTreap *elem_unique) {
            // Lambda function to run before going to the left child. 

            const LocationSorted* elem = static_cast<const LocationSorted*>(elem_unique);

            std::cerr << "fst target=" <<  target << " tnode_key=" << elem->key << " mutations=" << elem->mutations << "\n";

            // find lcp between elem->key and config->location
            uint32_t lcp = 0;
            for (lcp = 0; lcp < target.size() && lcp < elem->key.size(); ++lcp) {
                if (config->location_prefix[lcp] != elem->key[lcp]) {
                    break;
                }
            }

            std::cerr << "\t fst lcp=" << lcp << " ";

            if (lca_tnode == NULL && lcp == target.size()) {
                std::cerr << " --- set lca tnode\n";
                lca_tnode = tnode;
                // lca_key = elem->key;
                return 0;
            }
            std::cerr << "\n";

            if (elem->key < target) {
                return 1; // go directly to the right child.
            }
            return 0;
        },
        [&](common::Tnode *tnode, const BaseSortedTreap *elem_unique) {
            // Lambda function to run after processing the left child.

            const LocationSorted* elem = static_cast<const LocationSorted*>(elem_unique);

            std::cerr << "scd target=" <<  target << " tnode_key=" << elem->key << "\n";

            // find lcp between elem->key and config->location
            uint32_t lcp = 0;
            for (lcp = 0; lcp < target.size() && lcp < elem->key.size(); ++lcp) {
                if (config->location_prefix[lcp] != elem->key[lcp]) {
                    break;
                }
            }

            if (elem->key > target && lcp != target.size()) {
                std::cerr << "\t elem key greater than target\n";
                return -1;
            }

            if (elem->key >= target) {
                std::cerr << "\t found first";
                found_first = true;
            }

            if (found_first == false) {
                std::cerr << "\t go to the right child\n";
                return 1; // go to the right child.
            }
            std::vector<std::string> curr_mutations = common::get_mutations_from_json_str(elem->mutations);

            for (const std::string mutation : curr_mutations) {
                mutation_freq[mutation] += 1;
            }
            total_mutations += curr_mutations.size();

            assert (lca_tnode != NULL);

            std::cerr << "\n";

            if (tnode == lca_tnode) {
                before_lca_tnode = false;
                return 1; // go to the right child
            }

            // if (before_lca_tnode && tnode->r != NULL) {
            //     // get everything from tnode->r 's subtree.
            //     for (auto mutation : tnode->r->mutation_freq) {
            //         mutation_freq[mutation.first] += mutation.second;
            //         total_mutations += mutation.second;   
            //     }
            // } 
            // if (!before_lca_tnode && tnode->l != NULL) {
            //     // get everything from tnode->l 's subtree.
            //     for (auto mutation : tnode->l->mutation_freq) {
            //         mutation_freq[mutation.first] += mutation.second;
            //         total_mutations += mutation.second;   
            //     }
            // }
            return 1; 
        },
        config->snapshot
    );

    std::cout << "total_mutations=" << total_mutations << "\n";
    for (auto it : mutation_freq) {
        std::cout << it.first << "\t" << it.second << "\t" << (double)it.second / total_mutations << "\n";
    }

    // function for skipping subtree
    // function as callback for all the useful tnodes.

    return 0;
}

} // namespace cli