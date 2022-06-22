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
#include "../queries/count_indels.hpp"
#include "../queries/count_hosts.hpp"

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
    ds::CTC *ctc = new ds::CTC(h5_file_read);

    std::vector<std::string> snapshot_list;
    std::string curr_snapshot = "";
    std::cout << "Snapshots:\n";
    for (uint64_t i = 0; i < config->snapshot.size(); ++i) {
        if (config->snapshot[i] == ',') {
            std::cout << snapshot_list.size() << ".\t" << curr_snapshot << "\n";
            snapshot_list.push_back(curr_snapshot);
            curr_snapshot.clear();
        } else {
            curr_snapshot.push_back(config->snapshot[i]);
        }
    }
    std::cout << snapshot_list.size() << ".\t" << curr_snapshot << "\n";
    snapshot_list.push_back(curr_snapshot);

    query_ns::BaseQuery *query;
    if (config->query_type == FREQ_BP) {
        if (config->fnames.size() != 1) {
            Logger::error("Frequent Basepair query must receive exactly one target key.");
        }
        query = new query_ns::FreqBpQuery(config->compute_total_owner_cnt, config->num_to_print);
    } else if (config->query_type == CNT_INDELS) {
        query = new query_ns::CountIndelsQuery(snapshot_list.size());
    } else if (config->query_type == CNT_HOSTS){
        query = new query_ns::CountHostsQuery(snapshot_list.size());
    } else {
        Logger::error("Query type not recognised.");
        exit(1);
    }

    const std::string treap_name = query->get_treap_name();

    for (uint64_t i = 0; i < config->fnames.size(); ++i) {
        std::cout << "\nTarget key '" << config->fnames[i] << "'\n";
        
        query->reset();
        for (query->snapshot_idx = 0; query->snapshot_idx < snapshot_list.size(); ++query->snapshot_idx) {
            query->set_deletion_mode(false);

            ctc->treaps[treap_name].treap->query_callback_subtree(
                query,
                config->fnames[i],
                ctc->db,
                snapshot_list[query->snapshot_idx]
            );

            if (!config->exclude_deleted) {
                std::string treap_for_deletions = "deleted_" + treap_name; 
                if (! ctc->treaps.count(treap_for_deletions)) {
                    Logger::error("Could not find a treap for deletions with name '" + treap_for_deletions + "'.");
                }
                query->set_deletion_mode(true);
                ctc->treaps[treap_for_deletions].treap->query_callback_subtree(
                    query,
                    config->fnames[i],
                    ctc->db,
                    snapshot_list[query->snapshot_idx]
                );
            }
        }
        query->print_results();
    }

    delete ctc;
    delete query;

    return 0;
}

} // namespace cli
