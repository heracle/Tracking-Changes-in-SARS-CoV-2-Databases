#include "create.hpp"

#include "config.hpp"
#include "../common/logger.hpp"
#include "../common/constants.hpp"
#include "../ds/ps_treap.hpp"
#include "../ds/ctc.hpp"
#include "../ds/database.hpp"
#include "H5Cpp.h"

// TODO delete iostream
#include <iostream>
#include <fstream>

namespace cli {

int create(Config *config) {
    Logger::trace("Starting 'create' CLI...");
    
    ds::CTC *ctc = new ds::CTC(NULL, config->outdb_filepath);

    std::string input_fname = config->fnames[0];

    common::SeqElemReader *seq_reader = new common::SeqElemReader(input_fname);

    while (!seq_reader->end_of_file()) {
        std::vector<common::SeqElem> read_seq_elems = seq_reader->get_aligned_seq_elements();
        std::vector<std::pair<common::SeqElem, uint64_t>> seq_elems_to_insert;
        for (const common::SeqElem &seq : read_seq_elems) {
            seq_elems_to_insert.push_back(std::make_pair(seq, ULLONG_MAX));
        }
        ctc->insert_seq(seq_elems_to_insert);
    }
    ctc->save_snapshot(input_fname);
    ctc->export_to_h5();
    delete ctc;
    return 0;
}   

} // namespace cli