#include "gtest/gtest.h"

#include <vector>

#define private public
#define protected public

#include "../queries/count_hosts.hpp"
#include "../ds/database.hpp"

#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_location.hpp"

const std::string H5_FILENAME = "__tmp_h5_for_unit_tests.h5";

TEST(CountHostsQuery, TestCntHosts) {
    ds::PS_Treap *treap = new ds::PS_Treap(recompute_location_statistics,
                                            treap_types::LocationTnode::create_new_specialized_tnode,
                                            treap_types::LocationTnode::copy_specialized_tnode);
    EXPECT_EQ(treap->static_data.size(), 0);
    
    H5::H5File h5_file(H5_FILENAME, H5F_ACC_TRUNC);
    ds::DB *db = new ds::DB(&h5_file);

    std::unique_ptr<BaseSortedTreap> unique_node;

    std::vector<std::string> seq_keys = {"dcbae", "dcbac", "dcc", "da", "dcbadf", "dcbb", "dcd", "a", "dcbab", "dcbade", "dcbadg", "dcbaz", "dcbba", "dccaa", "ddaaa"};

    for (uint64_t i = 0; i < seq_keys.size(); ++i) {
        common::SeqElem curr_seq_elem;
        if (i % 2) {
            curr_seq_elem.covv_data[common::SEQ_FIELDS_TO_ID.at("covv_host")] = "Human";
        } else {
            curr_seq_elem.covv_data[common::SEQ_FIELDS_TO_ID.at("covv_host")] = "Dog";
        }
        
        db->insert_element(curr_seq_elem);

        unique_node = std::make_unique<LocationSorted>(seq_keys[i], i, 0, std::vector<uint64_t>());
        treap->static_data.push_back(std::move(unique_node));
    }
    db->write_buff_data();

    Tnode *tnode[seq_keys.size()];
    
    for (uint64_t i = 0; i < seq_keys.size(); ++i) {
        tnode[i] = LocationTnode::create_new_specialized_tnode(i);
    }    

    treap->root = tnode[0];

    for (uint64_t i = 1; i <= seq_keys.size() / 2; ++i) {
        tnode[i - 1]->l = tnode[2*i - 1];
        tnode[i - 1]->r = tnode[2*i];
    }

    for (int64_t i = seq_keys.size() - 1; i >= 0; --i) {
        recompute_location_statistics(tnode[i], treap->static_data[i].get());
    }

    struct CntIndelsTestStr {
        std::string test_id;
        std::string description;
        std::string target_prefix;
        tsl::hopscotch_map<std::string, uint64_t> hosts_distribution;
    };

    std::vector<CntIndelsTestStr> tests = {
        {
            "test0_",
            "todo",
            "",
            {{"Human", 7}, {"Dog", 8}}
        },
        {
            "test1_",
            "todo",
            "d",
            {{"Human", 6}, {"Dog", 8}}
        },
        {
            "test2_",
            "todo",
            "a",
            {{"Human", 1}}
        },
        {
            "test3_",
            "todo",
            "dcc",
            {{"Human", 1}, {"Dog", 1}}
        },
        {
            "test4_",
            "todo",
            "dcb",
            {{"Human", 4}, {"Dog", 5}}
        },
        {
            "test5_",
            "todo",
            "dda",
            {{"Dog", 1}}
        },
        {
            "test6_",
            "todo",
            "ddd",
            {}
        },
        {
            "test7_",
            "todo",
            "dcba",
            {{"Human", 3}, {"Dog", 4}}
        },
        {
            "test8_",
            "todo",
            "dcbb",
            {{"Human", 1}, {"Dog", 1}}
        },
        {
            "test9_",
            "todo",
            "dcbad",
            {{"Human", 1}, {"Dog", 2}}
        },
        {
            "test10_",
            "todo",
            "dcbab",
            {{"Dog", 1}}
        },
        {
            "test11_",
            "todo",
            "dcbade",
            {{"Human", 1}}
        },
        {
            "test12_",
            "todo",
            "ddaaa",
            {{"Dog", 1}}
        }
    };
    query_ns::CountHostsQuery *query = new query_ns::CountHostsQuery(tests.size());

    for (CntIndelsTestStr test : tests) {
        treap->query_callback_subtree(query, test.target_prefix, db);
        query->snapshot_idx++;
        query->set_deletion_mode(false);
    }

    for (uint64_t i = 0; i < tests.size(); ++i) {
        const CntIndelsTestStr &test = tests[i];
        EXPECT_EQ(test.test_id + std::to_string(query->total_host_occurrences[i].size()), test.test_id + std::to_string(test.hosts_distribution.size()));

        for (const auto it : query->total_host_occurrences[i]) {
            EXPECT_EQ(test.test_id + it.first + "_" + std::to_string(it.second), test.test_id + it.first + "_" + std::to_string(test.hosts_distribution.at(it.first)));
        }
    }
}
