#include "append.hpp"

#include "config.hpp"
#include "../common/logger.hpp"
#include "../common/constants.hpp"

#include "../ds/ps_treap.hpp"
#include "../ds/database.hpp"
#include "../ds/ctc.hpp"
#include "../ds/treap_representation/static_base.hpp"
#include "../ds/treap_representation/static_location.hpp"
#include "../ds/treap_representation/static_acc_id.hpp"
#include "H5Cpp.h"

using namespace treap_types;

#include <fstream>
#include <iostream>

namespace cli {

int append(Config *config) {
    std::ifstream instrm(config->indb_filepath, std::ios::in | std::ios::binary);
    if (!instrm.is_open()) {
        Logger::error("Can't open output file " + config->indb_filepath);
    }

    Logger::trace("Importing input file '" + config->indb_filepath + "'...");

    H5::H5File h5_file_read = H5::H5File(config->indb_filepath, H5F_ACC_RDONLY);    
    ds::CTC *ctc = new ds::CTC(h5_file_read);

    for (uint32_t i = 0; i < config->fnames.size(); ++i) {
        Logger::trace("Parsing snapshot '" + config->fnames[i] + "'...");
        std::vector<common::SeqElem> seq_elems = common::get_aligned_seq_elements(config->fnames[i]);

        // Create a treap only for seq_id to do the comparison.
        std::vector<std::unique_ptr<BaseSortedTreap>> acc_id_seq_snapshot;
        std::vector<common::SeqElem> seqelem_from_snapshot;

        uint32_t seq_counter = 0;
        for (const common::SeqElem &seq : seq_elems) {
            acc_id_seq_snapshot.push_back(AccessionIdSorted::get_unique_from_SeqElem(seq, seq_counter++));
            seqelem_from_snapshot.push_back(seq);
        }

        Logger::trace("Composing snapshot treap to compute the diferences...");
        ds::PS_Treap *snapshot_treap_acc_ids = new ds::PS_Treap(recompute_acc_id_statistics);
        snapshot_treap_acc_ids->insert(acc_id_seq_snapshot);

        // compare ctc treap with 'snapshot_treap_location'.
        std::vector<uint32_t> insertions_db_ids;
        std::vector<uint32_t> deletions_db_ids;
        std::vector<std::pair<uint32_t, uint32_t>> updates_db_ids;
        
        Logger::trace("Getting the difference between the input treap and the current snapshot...");
        ds::PS_Treap::get_differences(ctc->treap_acc_id, snapshot_treap_acc_ids, insertions_db_ids, deletions_db_ids, updates_db_ids);

        Logger::trace("Found " + std::to_string(insertions_db_ids.size()) + " new sequences.");
        Logger::trace("Found " + std::to_string(deletions_db_ids.size()) + " deleted sequences.");
        Logger::trace("Found " + std::to_string(updates_db_ids.size()) + " modified sequences.");

        // append the modified sequences id to the 'deletions_db_ids' list.
        for (std::pair<uint32_t, uint32_t> pair_ids : updates_db_ids) {
            deletions_db_ids.push_back(pair_ids.first);
        }

        Logger::trace("Erasing the deleted/modified elements from the main treap...");
        // erase the deleted and modified sequence from treap:
        ctc->erase(deletions_db_ids);

        std::vector<common::SeqElem> seq_elems_to_insert;
        for (uint32_t insertion_id : insertions_db_ids) {
            seq_elems_to_insert.push_back(seqelem_from_snapshot[insertion_id]);
        }
        for (std::pair<uint32_t, uint32_t> pair_ids : updates_db_ids) {
            seq_elems_to_insert.push_back(seqelem_from_snapshot[pair_ids.second]);
        }
        Logger::trace("Inserting the new/modified elements into the main treap...");
        ctc->insert_and_clear_ram(&seq_elems_to_insert);
        Logger::trace("Saving a the current treap status/snapshot...");
        ctc->save_snapshot(config->fnames[i]);
    }

    std::ofstream ostrm(config->outdb_filepath, std::ios::binary);
    if (!ostrm.is_open()) {
        Logger::error("Can't open output file " + config->outdb_filepath);
    }
    H5::H5File h5_file(config->outdb_filepath, H5F_ACC_TRUNC);
    ctc->export_to_h5(&ostrm, h5_file);
    h5_file.close();

    delete ctc;
    ostrm.close();
    return 0;
}

} // namespace cli
