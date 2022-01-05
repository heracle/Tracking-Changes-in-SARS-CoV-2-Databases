#include "stats.hpp"
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

int stats(Config *config) {

    std::ifstream instrm(config->indb_filepath, std::ios::in | std::ios::binary);
    if (!instrm.is_open()) {
        Logger::error("Can't open output file " + config->indb_filepath);
    }
    H5::H5File h5_file = H5::H5File(config->indb_filepath, H5F_ACC_RDONLY);    

    // read data before calling the constructor
    std::vector<std::unique_ptr<BaseSortedTreap>> location_data;
    H5::Group location_group = H5Gopen(h5_file.getLocId(), "/location_treap", H5P_DEFAULT);
    uint32_t size_location_data = H5Helper::get_uint32_attr_from(location_group, "static_data_size");

    for (uint32_t i = 0; i < size_location_data; ++i) {
        H5::Group curr_data_group = H5Gopen(location_group.getLocId(), ("static_data" + std::to_string(i)).c_str(), H5P_DEFAULT);
        std::unique_ptr<BaseSortedTreap> curr_location = std::make_unique<LocationSorted>(curr_data_group);
        curr_data_group.close();
        location_data.push_back(std::move(curr_location));
    }

    ds::PS_Treap *treap_location = new ds::PS_Treap(location_group, location_data, recompute_location_statistics);

    std::vector<std::string> snapshot_names = treap_location->get_snapshots_names();
    std::cout << "Total number of saved snapshots: " << snapshot_names.size() << "\n";
    std::cout << "Size of 'data' field:" << treap_location->get_data_size() << "\n";

    for (uint32_t i = 0; i < snapshot_names.size(); ++i) {
        uint32_t total_treap_nodes = 0;
        treap_location->iterate_ordered([&](const BaseSortedTreap &x) {++total_treap_nodes;}, snapshot_names[i]);
        std::cout << "Snapshot '" << snapshot_names[i] << "' contains " << total_treap_nodes << " treap nodes.\n";
    }

    return 0;
}

} // namespace cli
