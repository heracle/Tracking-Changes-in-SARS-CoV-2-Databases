#include "gtest/gtest.h"

#include <vector>

#define private public
#define protected public

#include "../queries/freq_bp.hpp"

#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_location.hpp"

// the last BITS_FOR_STEPS_BACK are masking the index of the previous version from which those alterations were coming;
// we need to add empty spaces for these BITS_FOR_STEPS_BACK bits.
std::vector<uint32_t> get_alter_from(const std::vector<uint32_t> &v) {
    std::vector<uint32_t> alterations;
    for (uint32_t i = 0; i < v.size(); ++i) {
        alterations.push_back(v[i] * (1<<common::BITS_FOR_STEPS_BACK));
    }
    return alterations;
}

TEST(FreqBpQuery, TestResultOneSnapshot) {
    struct query_insert_erase_elem {
        std::string test_id;
        std::string description;
        std::vector<std::pair<std::string, std::vector<uint32_t>>> treap_data;
        std::vector<std::string> target_prefixes;
        std::vector<std::vector<std::pair<uint32_t, uint32_t>>> expected_answers;
    };

    std::vector<query_insert_erase_elem> tests = {
        {
            "test0",
            "one snapshot",
            {
                {"aaa", {1, 2}},
                {"aab", {1, 3}},
                {"acc", {2, 3}}
            },
            {"aa", "ac", "a"},
            {
                {{1, 2}, {2, 1}, {3, 1}},
                {{2, 1}, {3, 1}},
                {{1, 2}, {2, 2}, {3, 2}},
            }
        },
        {
            "test1",
            "one snapshot",
            {
                {"aaa", {1, 2}},
                {"aaab", {1, 3}},
                {"aaac", {2, 3}}
            },
            {"aa", "aaa", "aaab", "aaad", "", "daa"},
            {
                {{1, 2}, {2, 2}, {3, 2}},
                {{1, 2}, {2, 2}, {3, 2}},
                {{1, 1}, {3, 1}},
                {},
                {{1, 2}, {2, 2}, {3, 2}},
                {}
            }
        }
    };

    for (const query_insert_erase_elem &test : tests) {
        ds::PS_Treap *treap = new ds::PS_Treap(recompute_location_statistics,
                                                     treap_types::LocationTnode::create_new_specialized_tnode,
                                                     treap_types::LocationTnode::copy_specialized_tnode);
        EXPECT_EQ(treap->static_data.size(), 0);

        std::vector<std::unique_ptr<BaseSortedTreap>> seq_list;
        for (const std::pair<std::string, std::vector<uint32_t>> &seq_data : test.treap_data) {
            std::vector<uint32_t> alterations = get_alter_from(seq_data.second);
            seq_list.push_back(std::make_unique<LocationSorted>(seq_data.first, 0, 0, alterations));
        }
        treap->insert(seq_list);
        ASSERT_EQ(test.target_prefixes.size(), test.expected_answers.size());

        for (uint32_t id_target_prefix = 0; id_target_prefix < test.target_prefixes.size(); ++id_target_prefix) {
            query_ns::FreqBpQuery *freq_bp_query = new query_ns::FreqBpQuery(false, 0);

            treap->query_callback_subtree(freq_bp_query, test.target_prefixes[id_target_prefix], NULL, "");

            for (uint32_t i = 0; i < test.expected_answers[id_target_prefix].size(); ++i) {
                EXPECT_EQ(freq_bp_query->alterations_per_bp[test.expected_answers[id_target_prefix][i].first], test.expected_answers[id_target_prefix][i].second);
            }
            delete freq_bp_query;
        }
    } 
}


TEST(FreqBpQuery, TestResultMultipleSnapshots) {
    ds::PS_Treap *treap = new ds::PS_Treap(recompute_location_statistics,
                                            treap_types::LocationTnode::create_new_specialized_tnode,
                                            treap_types::LocationTnode::copy_specialized_tnode);
    EXPECT_EQ(treap->static_data.size(), 0);

    std::vector<std::pair<std::string, std::vector<uint32_t>>> first_insert5 = {
        {"aaa", {1, 2}},
        {"aab", {1, 3}},
        {"acc", {1, 2, 3}},
        {"baa", {4, 6}},
        {"bac", {4, 7}}
    };

    std::vector<std::unique_ptr<BaseSortedTreap>> seq_list;
    for (const std::pair<std::string, std::vector<uint32_t>> &seq_data : first_insert5) {
        std::vector<uint32_t> alterations = get_alter_from(seq_data.second);
        seq_list.push_back(std::make_unique<LocationSorted>(seq_data.first, 0, 0, alterations));
    }
    treap->insert(seq_list);
    treap->save_snapshot("first");

    std::vector<std::pair<std::string, std::vector<uint32_t>>> second_insert3 = {
        {"aad", {1, 4}},
        {"bab", {2, 3}},
        {"caa", {10}}
    };

    seq_list.clear();
    for (const std::pair<std::string, std::vector<uint32_t>> &seq_data : second_insert3) {
        std::vector<uint32_t> alterations = get_alter_from(seq_data.second);
        seq_list.push_back(std::make_unique<LocationSorted>(seq_data.first, 0, 0, alterations));
    }

    treap->insert(seq_list);
    treap->save_snapshot("second");

    treap->erase(std::vector<uint32_t>{1, 2, 4, 6}); //"aab", "acc", "bac", "bab"
    treap->save_snapshot("third");

    std::vector<std::pair<std::string, std::vector<uint32_t>>> fourth_insert2 = {
        {"aab", {1, 4}},
        {"aavv", {20}}
    };

    seq_list.clear();
    for (const std::pair<std::string, std::vector<uint32_t>> &seq_data : fourth_insert2) {
        std::vector<uint32_t> alterations = get_alter_from(seq_data.second);
        seq_list.push_back(std::make_unique<LocationSorted>(seq_data.first, 0, 0, alterations));
    }
    treap->insert(seq_list);
    treap->save_snapshot("fourth");

    struct query_insert_erase_elem {
        std::string target_prefixes;
        std::string snapshot;
        std::vector<std::pair<uint32_t, uint32_t>> expected_answers;
    };

    std::vector<query_insert_erase_elem> tests = {
        {
            "aaa",
            "first",
            {
                {1, 1},
                {2, 1},
            }
        },
        {
            "aaa",
            "second",
            {
                {1, 1},
                {2, 1}
            }
        },
        {
            "aaa",
            "third",
            {
                {1, 1},
                {2, 1}
            }
        },
        {
            "aaa",
            "fourth",
            {
                {1, 1},
                {2, 1}
            }
        },
        {
            "aab",
            "first",
            {
                {1, 1},
                {3, 1},
            }
        },
        {
            "aab",
            "second",
            {
                {1, 1},
                {3, 1}
            }
        },
        {
            "aab",
            "third",
            {
            }
        },
        {
            "aab",
            "fourth",
            {
                {1, 1},
                {4, 1}
            }
        },
        {
            "aa",
            "first",
            {
                {1, 2},
                {2, 1},
                {3, 1}
            }
        },
        {
            "ba",
            "second",
            {
                {2, 1},
                {3, 1},
                {4, 2},
                {6, 1},
                {7, 1}
            }
        },
        {
            "ba",
            "third",
            {
                {4, 1},
                {6, 1},
            }
        },
        {
            "ba",
            "fourth",
            {
                {4, 1},
                {6, 1},
            }
        },
        {
            "aav",
            "third",
            {
            }
        },
        {
            "a",
            "fourth",
            {
                {1, 3},
                {2, 1},
                {4, 2},
                {20, 1}
            }
        },
        {
            "c",
            "second",
            {
                {10, 1}
            }
        },
        {
            "",
            "third",
            {
                {1, 2},
                {2, 1},
                {4, 2},
                {6, 1},
                {10, 1}
            }
        },
    };

    for (const query_insert_erase_elem &test : tests) {
        query_ns::FreqBpQuery *freq_bp_query = new query_ns::FreqBpQuery(false, 0);
        treap->query_callback_subtree(freq_bp_query, test.target_prefixes, NULL, test.snapshot);

        for (uint32_t i = 0; i < test.expected_answers.size(); ++i) {
            EXPECT_EQ(
                std::make_pair(test.target_prefixes + "_" + test.snapshot, freq_bp_query->alterations_per_bp[test.expected_answers[i].first]),
                std::make_pair(test.target_prefixes + "_" + test.snapshot, test.expected_answers[i].second)
            );
        }
        delete freq_bp_query;
    }

}