#include "gtest/gtest.h"

#include <vector>
#define private public
#define protected public

#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_location.hpp"


TEST(StaticLocation, GetUnique) {
    treap_types::LocationSorted::alteration_list_SeqElem = std::vector<std::vector<uint64_t>>({
        std::vector<uint64_t>{1, 5, 7},
        std::vector<uint64_t>{1, 8, 11},
        std::vector<uint64_t>{1, 5, 11}
    });
    treap_types::LocationSorted::next_alteration_SeqElem_id = 0;

    common::SeqElem e;
    e.covv_data[0] = e.covv_data[1] = e.covv_data[2] = e.covv_data[3] = "Y";

    LocationSorted *prv = new LocationSorted("key", 0, 0, std::vector<uint64_t>(), 0);

    for (uint64_t i = 0; i < treap_types::LocationSorted::alteration_list_SeqElem.size(); ++i) {
        std::unique_ptr<BaseSortedTreap> base = LocationSorted::get_unique_from_snapshot_line(e, 0, prv);
        LocationSorted *curr = static_cast<LocationSorted*>(base.get());

        EXPECT_EQ(curr->bp_alterations, treap_types::LocationSorted::alteration_list_SeqElem[i]);
    }
}

TEST(StaticLocation, SumUpOldAndNewAlterations) {
    treap_types::LocationSorted::alteration_list_SeqElem = std::vector<std::vector<uint64_t>>({
        std::vector<uint64_t>{100, 500, 700},
        std::vector<uint64_t>{100, 800, 1100},
        std::vector<uint64_t>{100, 500, 1100},
        std::vector<uint64_t>{1000},
        std::vector<uint64_t>{1000},
        std::vector<uint64_t>{},
    });
    LocationSorted *prv[6];
    prv[0] = new LocationSorted("key", 0, 0, std::vector<uint64_t>{600, 1000}, 0);
    prv[1] = new LocationSorted("key", 0, 0, std::vector<uint64_t>{800, 900, 1000, 1100, 2000}, 0);
    prv[2] = new LocationSorted("key", 0, 0, std::vector<uint64_t>{}, 0);
    prv[3] = new LocationSorted("key", 0, 0, std::vector<uint64_t>{1000}, 0);
    prv[4] = new LocationSorted("key", 0, 0, std::vector<uint64_t>{800, 900}, 0);
    prv[5] = new LocationSorted("key", 0, 0, std::vector<uint64_t>{800, 900}, 0);

    std::vector<uint64_t> expected_results[6] = {
        std::vector<uint64_t>{100, 500, 601, 700, 1001},
        std::vector<uint64_t>{100, 800, 801, 901, 1001, 1100, 1101, 2001},
        std::vector<uint64_t>{100, 500, 1100},
        std::vector<uint64_t>{1000, 1001},
        std::vector<uint64_t>{801, 901, 1000},
        std::vector<uint64_t>{801, 901},
    };

    treap_types::LocationSorted::next_alteration_SeqElem_id = 0;

    common::SeqElem e;
    e.covv_data[0] = e.covv_data[1] = e.covv_data[2] = e.covv_data[3] = "Y";


    for (uint64_t i = 0; i < treap_types::LocationSorted::alteration_list_SeqElem.size(); ++i) {
        std::unique_ptr<BaseSortedTreap> base = LocationSorted::get_unique_from_snapshot_line(e, 0, prv[i]);
        LocationSorted *curr = static_cast<LocationSorted*>(base.get());

        EXPECT_EQ(curr->bp_alterations, expected_results[i]);
    }
}

TEST(StaticLocation, VersionIncrementationWhenEditing) {

    LocationSorted *prv[6];
    prv[0] = new LocationSorted("key", 0, 0, std::vector<uint64_t>(), 0);
    prv[1] = new LocationSorted("key", 1, 1, std::vector<uint64_t>(), 0);
    prv[2] = new LocationSorted("key", 2, 1, std::vector<uint64_t>(), 0);
    prv[3] = new LocationSorted("key", 3, 2, std::vector<uint64_t>(), 0);
    prv[4] = new LocationSorted("key", 4, 2, std::vector<uint64_t>(), 0);
    prv[5] = new LocationSorted("key", 5, 3, std::vector<uint64_t>(), 0);

    treap_types::LocationSorted::next_alteration_SeqElem_id = 0;

    common::SeqElem e;
    e.covv_data[0] = e.covv_data[1] = e.covv_data[2] = e.covv_data[3] = "Y";

    for (uint64_t i = 0; i < 6; ++i) {
        std::unique_ptr<BaseSortedTreap> base = LocationSorted::get_unique_from_snapshot_line(e, 6 + i, prv[i]);
        LocationSorted *curr = static_cast<LocationSorted*>(base.get());

        EXPECT_EQ(curr->num_sequence_versions, prv[i]->num_sequence_versions + 1);
    }
}