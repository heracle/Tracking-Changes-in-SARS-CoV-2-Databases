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
#include <ctime>
#include <chrono>

namespace cli {

int append(Config *config) {
    auto start_tstamp = std::chrono::system_clock::now();
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

    auto end_tstamp = std::chrono::system_clock::now();
 
    std::chrono::duration<double> elapsed_seconds = end_tstamp - start_tstamp;
    std::time_t end_time_t = std::chrono::system_clock::to_time_t(end_tstamp);
 
    std::cerr << "finished DB clone at " << std::ctime(&end_time_t)
              << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;

    for (uint64_t i = 0; i < config->fnames.size(); ++i) {
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

            uint64_t seq_counter = 0;
            for (const common::SeqElem &seq : seq_elems) {
                acc_id_seq_snapshot.push_back(AccessionIdSorted::get_unique_from_snapshot_line(seq, seq_counter++, NULL));
            }

            snapshot_treap_acc_ids->insert(acc_id_seq_snapshot);
        }
        delete seq_reader;

        // compare ctc treap with 'snapshot_treap_location'.
        std::vector<uint64_t> insertions_db_ids;
        std::vector<uint64_t> deletions_db_ids;
        // .first is the idx in the main hash_treap and .second in the current snapshot.
        std::vector<std::pair<uint64_t, uint64_t>> updates_db_ids;

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
        for (std::pair<uint64_t, uint64_t> pair_ids : updates_db_ids) {
            deletions_db_ids.push_back(pair_ids.first);
        }

        Logger::trace("Erasing the deleted/modified elements from the main treap...");
        // erase the deleted and modified sequence from treap:
        ctc_out->erase_seq(deletions_db_ids, true);

        // Create a pair for id in snapshot and previous id of the same sequence to set the linked list.
        std::vector<std::pair<uint64_t, uint64_t>> snapshot_indices_to_insert;
        for (uint64_t insertion_id : insertions_db_ids) {
            snapshot_indices_to_insert.push_back({insertion_id, ULLONG_MAX});
        }
        for (std::pair<uint64_t, uint64_t> pair_ids : updates_db_ids) {
            snapshot_indices_to_insert.push_back({pair_ids.second, pair_ids.first});
        }

        // sort snapshot_indices_to_insert for being able to open one single SeqElemReader.
        std::sort(snapshot_indices_to_insert.begin(), snapshot_indices_to_insert.end());

        Logger::trace("Inserting the new/modified elements from the main treap...");
        seq_reader = new common::SeqElemReader(config->fnames[i]);

        std::vector<std::pair<common::SeqElem, uint64_t> > elems_to_insert;
        
        for (uint64_t i = 0; i < snapshot_indices_to_insert.size(); ++i) {
            common::SeqElem seq = seq_reader->get_elem(snapshot_indices_to_insert[i].first);
            if (snapshot_indices_to_insert[i].second == ULLONG_MAX) {
                // is new add
                seq.prv_db_id = ULLONG_MAX;
            } else {
                // is modified
                seq.prv_db_id = snapshot_indices_to_insert[i].second;
            }
            elems_to_insert.push_back(std::make_pair(seq, snapshot_indices_to_insert[i].second));
            if (elems_to_insert.size() == common::H5_APPEND_SIZE) {
                ctc_out->insert_seq(elems_to_insert);
                elems_to_insert.clear();
            }
        }
        if (elems_to_insert.size()) {
            ctc_out->insert_seq(elems_to_insert);
        }
        delete seq_reader;

        hash_treap = ctc_out->hash_treap;
        Logger::trace("Saving a current treap status/snapshot...");
        ctc_out->save_snapshot(config->fnames[i].substr(config->fnames[i].find_last_of('/') + 1));
    }

    Logger::trace("Exporting the ctc file...");

    ctc_out->export_to_h5();
    
    end_tstamp = std::chrono::system_clock::now();
 
    elapsed_seconds = end_tstamp - start_tstamp;
    end_time_t = std::chrono::system_clock::to_time_t(end_tstamp);
 
    std::cerr << "finished snapshot proc at " << std::ctime(&end_time_t)
              << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;

    delete ctc_out;
    h5_file_out.close();
    return 0;
}

} // namespace cli
