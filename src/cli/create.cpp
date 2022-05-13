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

    // Create a new file using the default property lists.
    H5::H5File h5_file(config->outdb_filepath, H5F_ACC_TRUNC);
    ds::CTC *ctc = new ds::CTC(&h5_file);

    std::string input_fname = config->fnames[0];

    common::SeqElemReader *seq_reader = new common::SeqElemReader(input_fname);

    while (!seq_reader->end_of_file()) {
        std::vector<common::SeqElem> read_seq_elems = seq_reader->get_aligned_seq_elements();
        std::cerr << "read_seq_elems size=" << read_seq_elems.size() << std::endl;
        std::vector<std::pair<common::SeqElem, uint64_t>> seq_elems_to_insert;
        for (const common::SeqElem &seq : read_seq_elems) {
            seq_elems_to_insert.push_back(std::make_pair(seq, ULLONG_MAX));
        }
        ctc->insert_seq(seq_elems_to_insert);
    }
    ctc->save_snapshot(input_fname);
    ctc->export_to_h5();

    delete ctc;
    h5_file.close();
    return 0;
}   

} // namespace cli