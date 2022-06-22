#include "stats.hpp"
#include "config.hpp"

#include "../common/logger.hpp"
#include "../common/h5_helper.hpp"
#include "../ds/ps_treap.hpp"
#include "../ds/database.hpp"

#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_location.hpp"

using namespace treap_types;

#include <iostream>
#include <fstream>

namespace cli {

int stats(Config *config) {

    std::ifstream instrm(config->indb_filepath, std::ios::in | std::ios::binary);
    if (!instrm.is_open()) {
        Logger::error("Can't open output file " + config->indb_filepath);
    }
    H5::H5File h5_file = H5::H5File(config->indb_filepath, H5F_ACC_RDONLY);    

    // read data before calling the constructor
    std::vector<std::unique_ptr<BaseSortedTreap>> location_data;
    H5::Group location_group = H5Gopen(h5_file.getLocId(), "/location_treap", H5P_DEFAULT);
    // uint64_t size_location_data = H5Helper::get_uint64_attr_from(location_group, "static_data_size");
    std::vector<std::string> location_keys = H5Helper::read_h5_dataset(location_group, "key");
    std::vector<uint64_t> location_database_ids = H5Helper::read_h5_int_to_dataset<uint64_t>(location_group, "database_id");
    assert(location_keys.size() == location_database_ids.size());

    for (uint64_t i = 0; i < location_keys.size(); ++i) {
        // H5::Group curr_data_group = H5Gopen(location_group.getLocId(), ("static_data" + std::to_string(i)).c_str(), H5P_DEFAULT);
        std::unique_ptr<BaseSortedTreap> curr_location = std::make_unique<LocationSorted>(location_keys[i], location_database_ids[i], 0, std::vector<uint64_t>(), 0);
        // curr_data_group.close();
        location_data.push_back(std::move(curr_location));
    }

    treap_types::LocationTnode::reset_get_h5_tnode(location_group);
    ds::PS_Treap *treap_location = new ds::PS_Treap(location_group, 
                                                    location_data, 
                                                    recompute_location_statistics, 
                                                    treap_types::LocationTnode::create_new_specialized_tnode,
                                                    treap_types::LocationTnode::copy_specialized_tnode,
                                                    treap_types::LocationTnode::get_h5_tnode);

    std::vector<std::string> snapshot_names = treap_location->get_snapshots_names();
    std::cout << "Total number of saved snapshots: " << snapshot_names.size() << "\n";
    std::cout << "Size of 'data' field:" << treap_location->get_data_size() << "\n";

    for (uint64_t i = 0; i < snapshot_names.size(); ++i) {
        uint64_t total_treap_nodes = 0;
        treap_location->iterate_ordered([&](const BaseSortedTreap &) {++total_treap_nodes;}, snapshot_names[i]);
        std::cout << "Snapshot '" << snapshot_names[i] << "' contains " << total_treap_nodes << " treap nodes.\n";
    }

    delete treap_location;

    return 0;
}

} // namespace cli
