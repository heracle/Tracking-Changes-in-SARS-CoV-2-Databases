#include "gtest/gtest.h"

#include <vector>

#define private public
#define protected public

#include "../queries/count_indels.hpp"

#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_location.hpp"

TEST(CountIndelsQuery, TestCntIndels) {
    ds::PS_Treap *treap = new ds::PS_Treap(recompute_location_statistics,
                                            treap_types::LocationTnode::create_new_specialized_tnode,
                                            treap_types::LocationTnode::copy_specialized_tnode);
    EXPECT_EQ(treap->static_data.size(), 0);

    std::unique_ptr<BaseSortedTreap> unique_node;

    std::vector<std::string> seq_keys = {"dcbae", "dcbac", "dcc", "da", "dcbadf", "dcbb", "dcd", "a", "dcbab", "dcbade", "dcbadg", "dcbaz", "dcbba", "dccaa", "ddaaa"};

    for (uint64_t i = 0; i < 15; ++i) {
        unique_node = std::make_unique<LocationSorted>(seq_keys[i], i, 1<<(i+1), std::vector<uint64_t>(), 0);
        treap->static_data.push_back(std::move(unique_node));
    }

    Tnode *tnode[15];
    
    for (uint64_t i = 0; i < 15; ++i) {
        tnode[i] = LocationTnode::create_new_specialized_tnode(i);
    }    

    treap->root = tnode[0];

    for (uint64_t i = 1; i <= 7; ++i) {
        tnode[i - 1]->l = tnode[2*i - 1];
        tnode[i - 1]->r = tnode[2*i];
    }

    for (int64_t i = 14; i >= 0; --i) {
        recompute_location_statistics(tnode[i], treap->static_data[i].get());
    }

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
        {
            "test12_",
            "todo",
            "ddaaa",
            1,
            32768
        }
    };
    query_ns::CountIndelsQuery *query = new query_ns::CountIndelsQuery(tests.size());

    for (CntIndelsTestStr test : tests) {
        query->snapshot_current_name = std::to_string(query->snapshot_idx);
        treap->query_callback_subtree(query, test.target_prefix, NULL);
        query->snapshot_idx++;
        query->set_deletion_mode(false);
    }

    for (uint64_t i = 0; i < tests.size(); ++i) {
        const CntIndelsTestStr &test = tests[i];
        EXPECT_EQ(test.test_id + std::to_string(query->saved_data(std::to_string(i))("inserted").GetValInt()), test.test_id + std::to_string(test.expected_num_sequences));
        EXPECT_EQ(test.test_id + std::to_string(query->saved_data(std::to_string(i))("modified").GetValInt()), test.test_id + std::to_string(test.expected_sum_versions));
    }
}

TEST(CountIndelsQuery, TestDeletionsTreap) {
    ds::PS_Treap *treap = new ds::PS_Treap(recompute_location_statistics,
                                            treap_types::LocationTnode::create_new_specialized_tnode,
                                            treap_types::LocationTnode::copy_specialized_tnode);
    EXPECT_EQ(treap->static_data.size(), 0);

    std::unique_ptr<BaseSortedTreap> unique_node;

    std::vector<std::string> seq_keys = {"dcbae", "dcbac", "dcc", "dcaa", "eee"}; // first 3 are normal sequences, the last 2 are deletions.

    for (uint64_t i = 0; i < 3; ++i) {
        unique_node = std::make_unique<LocationSorted>(seq_keys[i], i, 1<<(i+1), std::vector<uint64_t>(), 0);
        treap->static_data.push_back(std::move(unique_node));
    }

    Tnode *tnode[5];
    
    for (uint64_t i = 0; i < 3; ++i) {
        tnode[i] = LocationTnode::create_new_specialized_tnode(i);
    }    

    treap->root = tnode[0];

    tnode[0]->l = tnode[1];
    tnode[0]->r = tnode[2];

    ds::PS_Treap *deletions_treap = new ds::PS_Treap(recompute_location_statistics,
                                            treap_types::LocationTnode::create_new_specialized_tnode,
                                            treap_types::LocationTnode::copy_specialized_tnode);


    for (uint64_t i = 3; i < 5; ++i) {
        unique_node = std::make_unique<LocationSorted>(seq_keys[i], i, 1<<(i+1), std::vector<uint64_t>(), 0);
        deletions_treap->static_data.push_back(std::move(unique_node));
    }

    for (uint64_t i = 3; i < 5; ++i) {
        tnode[i] = LocationTnode::create_new_specialized_tnode(i-3);
    }

    deletions_treap->root = tnode[4];
    tnode[4]->l = tnode[3];

    for (int64_t i = 4; i >= 3; --i) {
        recompute_location_statistics(tnode[i], deletions_treap->static_data[i - 3].get());
    }
    for (int64_t i = 2; i >= 0; --i) {
        recompute_location_statistics(tnode[i], treap->static_data[i].get());
    }

    query_ns::CountIndelsQuery *query = new query_ns::CountIndelsQuery(3);

    query->snapshot_current_name="0";
    treap->query_callback_subtree(query, "", NULL);
    query->set_deletion_mode(true);
    deletions_treap->query_callback_subtree(query, "", NULL);

    query->snapshot_current_name="1";
    query->set_deletion_mode(false);
    query->snapshot_idx++;
    treap->query_callback_subtree(query, "d", NULL);
    query->set_deletion_mode(true);
    deletions_treap->query_callback_subtree(query, "d", NULL);

    query->snapshot_current_name="2";
    query->set_deletion_mode(false);
    query->snapshot_idx++;
    treap->query_callback_subtree(query, "e", NULL);
    query->set_deletion_mode(true);
    deletions_treap->query_callback_subtree(query, "e", NULL);

    EXPECT_EQ(query->saved_data(std::to_string(0))("inserted").GetValInt(), 3);
    EXPECT_EQ(query->saved_data(std::to_string(0))("modified").GetValInt(), 62);
    EXPECT_EQ(query->saved_data(std::to_string(0))("deleted").GetValInt(), 2);

    EXPECT_EQ(query->saved_data(std::to_string(1))("inserted").GetValInt(), 3);
    EXPECT_EQ(query->saved_data(std::to_string(1))("modified").GetValInt(), 30);
    EXPECT_EQ(query->saved_data(std::to_string(1))("deleted").GetValInt(), 1);

    EXPECT_EQ(query->saved_data(std::to_string(2))("inserted").GetValInt(), 0);
    EXPECT_EQ(query->saved_data(std::to_string(2))("modified").GetValInt(), 32);
    EXPECT_EQ(query->saved_data(std::to_string(2))("deleted").GetValInt(), 1);
}