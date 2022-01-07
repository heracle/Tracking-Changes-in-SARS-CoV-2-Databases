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

    ds::CTC *ctc = new ds::CTC();

    std::string input_fname = config->fnames[0];
    std::vector<common::SeqElem> seq_elems = common::get_aligned_seq_elements(input_fname);
    ctc->insert_and_clear_ram(&seq_elems);
    ctc->save_snapshot(input_fname);

    // Create a new file using the default property lists.
    H5::H5File h5_file(config->outdb_filepath, H5F_ACC_TRUNC);
    ctc->export_to_h5(h5_file);

    h5_file.close();
    return 0;
}   

} // namespace cli