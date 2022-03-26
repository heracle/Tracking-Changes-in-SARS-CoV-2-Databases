#include "gtest/gtest.h"

#include <string>
#include <stdexcept>

#define private public
#define protected public

#include "../ds/ps_treap.hpp"
#include "../ds/tnode_types/tnode_acc_id.hpp"
#include "../ds/static_types/static_acc_id.hpp"

Tnode* copy_test_tnode(const treap_types::Tnode* target) {
    treap_types::Tnode *tnode = new treap_types::Tnode(target);
    return tnode;
}

Tnode* create_new_test_tnode(const uint32_t x) {
    treap_types::Tnode *tnode = new treap_types::Tnode(x);
    return tnode;
}

TEST(DsTreap, InsertEraseElementInteger) {
    //add multiple equal elements

    enum TreapRequest {insert, erase, query};
    struct query_insert_erase_elem {
        std::string test_id;
        std::string description;
        std::vector<std::pair<TreapRequest, std::vector<int>>> requests;
    };

    std::vector<query_insert_erase_elem> tests = {
        {
            "test0",
            "no insertions",
            {
                {query, {}},
            }
        },
        {
            "test1",
            "insertions only",
            {
                {insert, {5, 7, 3}},
                {query, {3, 5, 7}},
                {insert, {4, 10, 1, 2, 6}},
                {query, {1, 10, 2, 3, 4, 5, 6, 7}},
            }
        },
        {
            "test2",
            "insertions and deletions",
            {
                {insert, {2, 3, 1}},
                {erase, {2}},
                {query, {1, 3}},
                {insert, {2}},
                {query, {1, 2, 3}},
                {erase, {2}},
                {query, {1, 3}},
                {insert, {2}},
                {query, {1, 2, 3}},
                {erase, {1}},
                {query, {2, 3}},
                {insert, {1}},
                {query, {1, 2, 3}},
                {erase, {3}},
                {query, {1, 2}},
                {erase, {1}},
                {query, {2}},
                {insert, {1}},
                {query, {1, 2}},
                {erase, {2}},
                {query, {1}},
                {erase, {1}},
                {query, {}},
                {insert, {5, 10, 2, 1}},
                {query, {1, 10, 2, 5}},
                {insert, {8}},
                {query, {1, 10, 2, 5, 8}},
            }
        },
        {
            "test3",
            "multiple occurances of the same value: insertions and deletions",
            {
                {insert, {1, 2, 3}},
                {insert, {3}},
                {query, {1, 2, 3, 3}},
                {erase, {3}},
                {query, {1, 2, 3}},
                {insert, {1, 3, 4, 4}}, 
                {query, {1, 1, 2, 3, 3, 4, 4}},
                {erase, {1, 1, 3, 4}},
                {query, {2, 3, 4}},
                {insert, {1, 3, 2}},
                {query, {1, 2, 2, 3, 3, 4}},
                {erase, {1, 4}},
                {query, {2, 2, 3, 3}},
                {erase, {2, 2, 3, 3}},
                {query, {}},
                {insert, {10}},
                {query, {10}},
            }
        }
    };

    for (query_insert_erase_elem test : tests) {
        ds::PS_Treap *treap = new ds::PS_Treap([&](Tnode *, const BaseSortedTreap*) {},
                                                create_new_test_tnode,
                                                copy_test_tnode);
        EXPECT_EQ(treap->static_data.size(), 0);
        uint32_t id_query = 0;
        for (std::pair<TreapRequest, std::vector<int>> treap_request : test.requests) {
            // create nodes for the requested int number.
            std::vector<std::unique_ptr<BaseSortedTreap>> curr_nodes;
            std::vector<uint32_t> indices_to_erase;
            for (const int x : treap_request.second) {
                curr_nodes.push_back(std::move(
                    std::make_unique<BaseSortedTreap>(std::to_string(x), 0)
                ));
            }
            for (const std::unique_ptr<BaseSortedTreap> &unique : curr_nodes) {
                auto tnode = treap->find(*unique);
                if (tnode != NULL) {
                    indices_to_erase.push_back(tnode->data_id);
                }
            }

            switch (treap_request.first) {
                case insert:
                    treap->insert(curr_nodes);
                    break;
                case erase:
                    treap->erase(indices_to_erase);
                    break;
                case query:
                    treap->save_snapshot(std::to_string(id_query++));
                    break;
            }
        }

        id_query = 0;
        for (std::pair<TreapRequest, std::vector<int>> treap_request : test.requests) {
            if (treap_request.first != query) {
                continue;
            }
            std::vector<int> ordered_values;
            treap->iterate_ordered([&](const BaseSortedTreap &x) {ordered_values.push_back(std::stoi(x.key));},
                                   std::to_string(id_query++));
            EXPECT_EQ(
                std::make_pair(test.test_id, ordered_values), 
                std::make_pair(test.test_id, treap_request.second)
            );
        }

        delete treap;
    }
}

TEST(DsTreap, InsertEraseElementString) {
    ds::PS_Treap *treap = new ds::PS_Treap([&](Tnode *, const BaseSortedTreap*) {},
                                            create_new_test_tnode,
                                            copy_test_tnode);
    EXPECT_EQ(treap->static_data.size(), 0);

    std::vector<std::unique_ptr<BaseSortedTreap>> init_list;
    init_list.push_back(std::make_unique<BaseSortedTreap>("t", 0));
    init_list.push_back(std::make_unique<BaseSortedTreap>("abc", 0));
    init_list.push_back(std::make_unique<BaseSortedTreap>("aba", 0));
    init_list.push_back(std::make_unique<BaseSortedTreap>("ab0ccde", 0));
    init_list.push_back(std::make_unique<BaseSortedTreap>("ab0caade", 0));

    std::vector<std::string> expected1 = {"ab0caade", "ab0ccde", "aba", "abc", "t"};
    std::vector<std::string> ordered_values;
    treap->insert(init_list);
    treap->iterate_ordered([&](const BaseSortedTreap &x) {ordered_values.push_back(x.key);});
    EXPECT_EQ(ordered_values, expected1);

    std::vector<uint32_t> erase_list;
    erase_list.push_back(treap->find(BaseSortedTreap{"t", 0})->data_id);
    erase_list.push_back(treap->find(BaseSortedTreap{"ab0ccde", 0})->data_id);

    // std::vector<BaseSortedTreap> erase_list = {{"t", 0}, {"ab0ccde", 0}};
    std::vector<std::string> expected2 = {"ab0caade", "aba", "abc"};
    treap->erase(erase_list);
    ordered_values.clear();
    treap->iterate_ordered([&](const BaseSortedTreap &x) {ordered_values.push_back(x.key);});
    EXPECT_EQ(ordered_values, expected2);

    std::vector<std::unique_ptr<BaseSortedTreap>> insert_list;
    insert_list.push_back(std::make_unique<BaseSortedTreap>("t", 0));
    insert_list.push_back(std::make_unique<BaseSortedTreap>("p", 0));
    insert_list.push_back(std::make_unique<BaseSortedTreap>("aba00", 0));
    insert_list.push_back(std::make_unique<BaseSortedTreap>("aaa", 0));
    std::vector<std::string> expected3 = {"aaa", "ab0caade", "aba", "aba00", "abc", "p", "t"};
    treap->insert(insert_list);
    ordered_values.clear();
    treap->iterate_ordered([&](const BaseSortedTreap &x) {ordered_values.push_back(x.key);});
    EXPECT_EQ(ordered_values, expected3);

    delete treap;
}

TEST(DsTreap, EraseNonExistentElement) {
    ds::PS_Treap *treap = new ds::PS_Treap([&](Tnode *, const BaseSortedTreap*) {},
                                            create_new_test_tnode,
                                            copy_test_tnode);
    EXPECT_EQ(treap->static_data.size(), 0);

    std::vector<std::unique_ptr<BaseSortedTreap>> init_list;
    init_list.push_back(std::make_unique<BaseSortedTreap>("t", 0));
    init_list.push_back(std::make_unique<BaseSortedTreap>("abc", 0));
    init_list.push_back(std::make_unique<BaseSortedTreap>("aba", 0));
    init_list.push_back(std::make_unique<BaseSortedTreap>("ab0ccde", 0));
    init_list.push_back(std::make_unique<BaseSortedTreap>("ab0caade", 0));

    std::vector<std::string> expected1 = { "ab0caade", "ab0ccde", "aba", "abc", "t"};
    std::vector<std::string> ordered_values;
    treap->insert(init_list);
    treap->iterate_ordered([&](const BaseSortedTreap &x) {ordered_values.push_back(x.key);});
    EXPECT_EQ(ordered_values, expected1);
    EXPECT_TRUE(treap->find(BaseSortedTreap{"p", 0}) == NULL);

    delete treap;
}

TEST(DsTreap, GetDifferences) {
    ds::PS_Treap *first_treap = new ds::PS_Treap(recompute_acc_id_statistics,
                                                 treap_types::AccIdTnode::create_new_specialized_tnode,
                                                 treap_types::AccIdTnode::copy_specialized_tnode);
    EXPECT_EQ(first_treap->static_data.size(), 0);

    ds::PS_Treap *second_treap = new ds::PS_Treap(recompute_acc_id_statistics,
                                                  treap_types::AccIdTnode::create_new_specialized_tnode,
                                                  treap_types::AccIdTnode::copy_specialized_tnode);
    EXPECT_EQ(second_treap->static_data.size(), 0);

    std::vector<std::unique_ptr<BaseSortedTreap>> first_seq_list;
    first_seq_list.push_back(std::make_unique<AccessionIdSorted>("aaa", 0, 100, 100));
    first_seq_list.push_back(std::make_unique<AccessionIdSorted>("aab", 10, 200, 200));
    first_seq_list.push_back(std::make_unique<AccessionIdSorted>("aac", 20, 300, 300));
    first_seq_list.push_back(std::make_unique<AccessionIdSorted>("acd", 30, 400, 400));
    first_seq_list.push_back(std::make_unique<AccessionIdSorted>("db0", 40, 500, 500));
    uint32_t first_seq_list_sz = first_seq_list.size();

    first_treap->insert(first_seq_list);
    uint32_t cnt = 0;
    first_treap->iterate_ordered([&](const BaseSortedTreap &x) {cnt++;});
    EXPECT_EQ(cnt, first_seq_list_sz);

    std::vector<std::unique_ptr<BaseSortedTreap>> second_seq_list;

    second_seq_list.push_back(std::make_unique<AccessionIdSorted>("aac", 20, 307, 300));
    second_seq_list.push_back(std::make_unique<AccessionIdSorted>("aaaG", 50, 700, 700));
    second_seq_list.push_back(std::make_unique<AccessionIdSorted>("acd", 30, 400, 409));
    second_seq_list.push_back(std::make_unique<AccessionIdSorted>("db0", 40, 510, 510));
    second_seq_list.push_back(std::make_unique<AccessionIdSorted>("aaa", 0, 100, 100));
    uint32_t second_seq_list_sz = second_seq_list.size();
 
    second_treap->insert(second_seq_list);
    cnt = 0;
    second_treap->iterate_ordered([&](const BaseSortedTreap &x) {cnt++;});
    EXPECT_EQ(cnt, second_seq_list_sz);

    std::vector<uint32_t> insertions_db_ids;
    std::vector<uint32_t> deletions_db_ids;
    std::vector<std::pair<uint32_t, uint32_t>> updates_db_ids;
    ds::PS_Treap::get_differences(first_treap, second_treap, insertions_db_ids, deletions_db_ids, updates_db_ids);

    EXPECT_EQ(insertions_db_ids.size(), 1);
    EXPECT_EQ(insertions_db_ids[0], 1);

    EXPECT_EQ(deletions_db_ids.size(), 1);
    EXPECT_EQ(deletions_db_ids[0], 1);

    EXPECT_EQ(updates_db_ids.size(), 3);
    EXPECT_EQ(updates_db_ids[0].first, 2);
    EXPECT_EQ(updates_db_ids[0].second, 0);
    EXPECT_EQ(updates_db_ids[1].first, 3);
    EXPECT_EQ(updates_db_ids[1].second, 2);
    EXPECT_EQ(updates_db_ids[2].first, 4);
    EXPECT_EQ(updates_db_ids[2].second, 3);

    delete first_treap;
    delete second_treap;
}

// // Analyse if there is any issue if we have equal priorities for different nodes.
// TEST(DsTreap, InsertEqKeyAndPrio) {
//     ds::PS_Treap<std::string> *treap = new ds::PS_Treap<std::string>();
//     EXPECT_EQ(treap->data.size(), 0);
//     delete treap;
// }