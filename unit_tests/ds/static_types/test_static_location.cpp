#include "gtest/gtest.h"

#include <vector>
#define private public
#define protected public

#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_location.hpp"


TEST(StaticLocation, GetUnique) {
    treap_types::LocationSorted::alteration_list_SeqElem = std::vector<std::vector<uint32_t>>({
        std::vector<uint32_t>{1, 5, 7},
        std::vector<uint32_t>{1, 8, 11},
        std::vector<uint32_t>{1, 5, 11}
    });
    treap_types::LocationSorted::next_alteration_SeqElem_id = 0;

    common::SeqElem e;
    e.covv_data[0] = e.covv_data[1] = e.covv_data[2] = e.covv_data[3] = "Y";

    for (uint32_t i = 0; i < treap_types::LocationSorted::alteration_list_SeqElem.size(); ++i) {
        std::unique_ptr<BaseSortedTreap> base = LocationSorted::get_unique_from_snapshot_line(e, 0, true);
        LocationSorted *curr = static_cast<LocationSorted*>(base.get());

        EXPECT_EQ(curr->bp_alterations, treap_types::LocationSorted::alteration_list_SeqElem[i]);
    }


    // ds::PS_Treap *first_treap = new ds::PS_Treap(recompute_location_statistics,
    //                                              treap_types::LocationTnode::create_new_specialized_tnode,
    //                                              treap_types::LocationTnode::copy_specialized_tnode);
    // EXPECT_EQ(first_treap->static_data.size(), 0);

    // ds::PS_Treap *second_treap = new ds::PS_Treap(recompute_location_statistics,
    //                                               treap_types::LocationTnode::create_new_specialized_tnode,
    //                                               treap_types::LocationTnode::copy_specialized_tnode);
    // EXPECT_EQ(second_treap->static_data.size(), 0);

    // std::vector<std::unique_ptr<BaseSortedTreap>> first_seq_list;
    // first_seq_list.push_back(std::make_unique<LocationSorted>("aaa", 0, std::vector<std::pair<uint32_t, uint32_t>>()));
    // first_seq_list.push_back(std::make_unique<LocationSorted>("aab", 10, std::vector<std::pair<uint32_t, uint32_t>>()));
    // first_seq_list.push_back(std::make_unique<LocationSorted>("aac", 20, std::vector<std::pair<uint32_t, uint32_t>>()));
    // first_seq_list.push_back(std::make_unique<LocationSorted>("acd", 30, std::vector<std::pair<uint32_t, uint32_t>>()));
    // first_seq_list.push_back(std::make_unique<LocationSorted>("db0", 40, std::vector<std::pair<uint32_t, uint32_t>>()));
    // uint32_t first_seq_list_sz = first_seq_list.size();

    // first_treap->insert(first_seq_list);
    // uint32_t cnt = 0;
    // first_treap->iterate_ordered([&](const BaseSortedTreap &x) {cnt++;});
    // EXPECT_EQ(cnt, first_seq_list_sz);

    // std::vector<std::unique_ptr<BaseSortedTreap>> second_seq_list;

    // second_seq_list.push_back(std::make_unique<LocationSorted>("aac", 20, {()}));
    // second_seq_list.push_back(std::make_unique<LocationSorted>("aaaG", 50, {}));
    // second_seq_list.push_back(std::make_unique<LocationSorted>("acd", 30, {}));
    // second_seq_list.push_back(std::make_unique<LocationSorted>("db0", 40, {}));
    // second_seq_list.push_back(std::make_unique<LocationSorted>("aaa", 0, {}));
    // uint32_t second_seq_list_sz = second_seq_list.size();
 
    // second_treap->insert(second_seq_list);
    // cnt = 0;
    // second_treap->iterate_ordered([&](const BaseSortedTreap &x) {cnt++;});
    // EXPECT_EQ(cnt, second_seq_list_sz);

    // LocationSorted::get_unique_from_snapshot_line();
    // EXPECT_EQ(0, 1);
}
