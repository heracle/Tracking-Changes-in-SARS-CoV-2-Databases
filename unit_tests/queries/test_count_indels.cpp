#include "gtest/gtest.h"

#include <vector>

#define private public
#define protected public

#include "../queries/count_indels.hpp"

#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_location.hpp"

TEST(CountIndelsQuery, TestDifferentSubtrees) {
    ds::PS_Treap *treap = new ds::PS_Treap(recompute_location_statistics,
                                            treap_types::LocationTnode::create_new_specialized_tnode,
                                            treap_types::LocationTnode::copy_specialized_tnode);
    EXPECT_EQ(treap->static_data.size(), 0);

    std::unique_ptr<BaseSortedTreap> unique_node;

    std::vector<std::string> seq_keys = {"dcbae", "dcbac", "dcc", "da", "dcbadf", "dcbb", "dcd", "a", "dcbab", "dcbade", "dcbadg", "dcbaz", "dcbba", "dccaa", "ddaaa"};

    for (uint32_t i = 0; i < 15; ++i) {
        unique_node = std::make_unique<LocationSorted>(seq_keys[i], i, 1<<(i+1), std::vector<uint32_t>());
        treap->static_data.push_back(std::move(unique_node));
    }

    Tnode *tnode[15];
    
    for (uint32_t i = 0; i < 15; ++i) {
        tnode[i] = LocationTnode::create_new_specialized_tnode(i);
    }    

    treap->root = tnode[0];

    for (uint32_t i = 1; i <= 7; ++i) {
        tnode[i - 1]->l = tnode[2*i - 1];
        tnode[i - 1]->r = tnode[2*i];
    }

    for (int32_t i = 14; i >= 0; --i) {
        recompute_location_statistics(tnode[i], treap->static_data[i].get());
    }

    // treap->iterate_ordered([&](const BaseSortedTreap &x) {std::cerr << x.key << "\n";});
    query_ns::CountIndelsQuery *query = new query_ns::CountIndelsQuery();

    struct CntIndelsTestStr {
        std::string test_id;
        std::string description;
        std::string target_prefix;
        uint64_t expected_num_sequences;
        uint64_t expected_sum_versions;
    };

    std::vector<CntIndelsTestStr> tests = {
        {
            "test0_",
            "todo",
            "",
            15,
            65534
        },
        {
            "test1_",
            "todo",
            "d",
            14,
            65278
        },
        {
            "test2_",
            "todo",
            "a",
            1,
            256
        },
        {
            "test3_",
            "todo",
            "dcc",
            2,
            16392
        },
        {
            "test4_",
            "todo",
            "dcb",
            9,
            15974
        },
        {
            "test5_",
            "todo",
            "dda",
            1,
            32768
        },
        {
            "test6_",
            "todo",
            "ddd",
            0,
            0
        },
        {
            "test7_",
            "todo",
            "dcba",
            7,
            7718
        },
        {
            "test8_",
            "todo",
            "dcbb",
            2,
            8256
        },
        {
            "test9_",
            "todo",
            "dcbad",
            3,
            3104
        },
        {
            "test10_",
            "todo",
            "dcbab",
            1,
            512
        },
        {
            "test11_",
            "todo",
            "dcbade",
            1,
            1024
        },
    };

    for (CntIndelsTestStr test : tests) {
        treap->query_callback_subtree(query, test.target_prefix, NULL);
        EXPECT_EQ(test.test_id + std::to_string(query->num_sequences), test.test_id + std::to_string(test.expected_num_sequences));
        EXPECT_EQ(test.test_id + std::to_string(query->sum_versions), test.test_id + std::to_string(test.expected_sum_versions));
        query->reset();
    }
}