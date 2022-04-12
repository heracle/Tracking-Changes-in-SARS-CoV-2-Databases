#include "append.hpp"

#include "config.hpp"
#include "../common/logger.hpp"
#include "../common/constants.hpp"

#include "../ds/ps_treap.hpp"
#include "../ds/database.hpp"
#include "../ds/ctc.hpp"
#include "../ds/static_types/static_base.hpp"
#include "../ds/static_types/static_location.hpp"
#include "../ds/static_types/static_acc_id.hpp"

#include "../ds/tnode_types/tnode_acc_id.hpp"

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

    // we need this H5F_ACC_RDWR for append to the existent file.
    H5::H5File h5_file_in = H5::H5File(config->indb_filepath, H5F_ACC_RDONLY);    
    ds::CTC *ctc_in = new ds::CTC(h5_file_in);

    H5::H5File h5_file_out = H5::H5File(config->outdb_filepath, H5F_ACC_TRUNC);    
    ds::CTC *ctc_out = new ds::CTC(&h5_file_out, ctc_in);
    ds::PS_Treap *hash_treap = ctc_out->hash_treap;
    
    delete ctc_in; // it is also made unusable by the previous constructor.
    h5_file_in.close();

    for (uint32_t i = 0; i < config->fnames.size(); ++i) {
        Logger::trace("Parsing snapshot '" + config->fnames[i] + "'...");
        std::vector<common::SeqElem> seq_elems;
        Logger::trace("Composing snapshot treap to compute the diferences...");
        ds::PS_Treap *snapshot_treap_acc_ids = new ds::PS_Treap(recompute_acc_id_statistics,
                                                                AccIdTnode::create_new_specialized_tnode,
                                                                AccIdTnode::copy_specialized_tnode);
        common::SeqElemReader *seq_reader = new common::SeqElemReader(config->fnames[i]);
        while (!seq_reader->end_of_file()) {
            std::vector<common::SeqElem> seq_elems = seq_reader->get_aligned_seq_elements();

            // Create a treap only for seq_id to do the comparison.
            std::vector<std::unique_ptr<BaseSortedTreap>> acc_id_seq_snapshot;

            uint32_t seq_counter = 0;
            for (const common::SeqElem &seq : seq_elems) {
                acc_id_seq_snapshot.push_back(AccessionIdSorted::get_unique_from_snapshot_line(seq, seq_counter++));
            }

            snapshot_treap_acc_ids->insert(acc_id_seq_snapshot);
        }
        delete seq_reader;

        // compare ctc treap with 'snapshot_treap_location'.
        std::vector<uint32_t> insertions_db_ids;
        std::vector<uint32_t> deletions_db_ids;
        std::vector<std::pair<uint32_t, uint32_t>> updates_db_ids;

        Logger::trace("Getting the difference between the input treap and the current snapshot...");
        ds::PS_Treap::get_differences(hash_treap, snapshot_treap_acc_ids, insertions_db_ids, deletions_db_ids, updates_db_ids);
        Logger::trace("Found " + std::to_string(insertions_db_ids.size()) + " new sequences.");
        Logger::trace("Found " + std::to_string(deletions_db_ids.size()) + " deleted sequences.");
        Logger::trace("Found " + std::to_string(updates_db_ids.size()) + " modified sequences.");

        ctc_out->prepare_specific_get_unique_SeqElem(hash_treap, snapshot_treap_acc_ids, ctc_out->db, config->fnames[i], insertions_db_ids, deletions_db_ids, updates_db_ids);
        Logger::trace("Erasing the deleted elements from the main treap...");
        // erase the deleted and modified sequence from treap:
        ctc_out->erase_seq(deletions_db_ids, false);

        deletions_db_ids.clear();
        // append the modified sequences id to the 'deletions_db_ids' list.
        for (std::pair<uint32_t, uint32_t> pair_ids : updates_db_ids) {
            deletions_db_ids.push_back(pair_ids.first);
        }

        Logger::trace("Erasing the deleted/modified elements from the main treap...");
        // erase the deleted and modified sequence from treap:
        ctc_out->erase_seq(deletions_db_ids, true);

        // Create a pair for id in snapshot and bool -> false for insertion and true for alteration.
        std::vector<std::pair<uint32_t, uint32_t>> snapshot_indices_to_insert;
        for (uint32_t insertion_id : insertions_db_ids) {
            snapshot_indices_to_insert.push_back({insertion_id, UINT_MAX});
        }
        for (std::pair<uint32_t, uint32_t> pair_ids : updates_db_ids) {
            snapshot_indices_to_insert.push_back({pair_ids.second, pair_ids.first});
        }

        // sort snapshot_indices_to_insert for being able to open one single SeqElemReader.
        std::sort(snapshot_indices_to_insert.begin(), snapshot_indices_to_insert.end());

        Logger::trace("Inserting the new/modified elements from the main treap...");
        seq_reader = new common::SeqElemReader(config->fnames[i]);

        for (uint32_t i = 0; i < snapshot_indices_to_insert.size(); ++i) {
            common::SeqElem seq = seq_reader->get_elem(snapshot_indices_to_insert[i].first);
            if (snapshot_indices_to_insert[i].second == UINT_MAX) {
                // is new add
                seq.prv_db_id = 0;
            } else {
                // is modified
                seq.prv_db_id = snapshot_indices_to_insert[i].second;
            }

            std::vector<common::SeqElem> curr = {seq};
            ctc_out->insert_seq(&curr, snapshot_indices_to_insert[i].second != UINT_MAX);
        }
        delete seq_reader;

        hash_treap = ctc_out->hash_treap;
        Logger::trace("Saving a current treap status/snapshot...");
        ctc_out->save_snapshot(config->fnames[i]);
    }

    Logger::trace("Exporting the ctc file...");

    ctc_out->export_to_h5();
    delete ctc_out;
    h5_file_out.close();
    return 0;
}

} // namespace cli
