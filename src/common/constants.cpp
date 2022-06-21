#include "constants.hpp"

#include "../common/logger.hpp"

namespace common {

void validate_seq_fields() {
    if (SEQ_FIELDS.size() != SEQ_FIELDS_TO_ID.size()) {
        Logger::error("Sequence fields to parse are not properly set : size ");
    }

    for (uint64_t i = 0; i < SEQ_FIELDS.size(); ++i) {
        auto it = SEQ_FIELDS_TO_ID.find(SEQ_FIELDS[i]);
        if (it == SEQ_FIELDS_TO_ID.end()) {
            Logger::error("Sequence fields to parse are not properly set : can't find " + SEQ_FIELDS[i]);
        }

        if (it->second != i) {
            Logger::error("Sequence fields to parse are not properly set : invalid index for " + SEQ_FIELDS[i]);
        }
    }
}

} // namespace common