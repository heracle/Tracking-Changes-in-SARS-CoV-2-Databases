#include "gtest/gtest.h"

#include <vector>
#define private public
#define protected public

#include "../ds/static_types/static_location.hpp"
#include "../ds/tnode_types/tnode_location.hpp"

TEST(TnodeLocation, RecomputeStatistics) {
    Tnode *left = new LocationTnode(2000, 100, 0, 0, 0);
    Tnode *right = new LocationTnode(4000, 200, 0, 0, 0);

    LocationSorted *curr_static = new LocationSorted("key", 0, 8, std::vector<uint64_t>(), 0);

    Tnode *tnode = new LocationTnode(0, 0, 0, 0, 0);

    treap_types::recompute_location_statistics(tnode, curr_static);
    EXPECT_EQ(static_cast<LocationTnode*>(tnode)->total_versions_in_subtree, 8);
    EXPECT_EQ(static_cast<LocationTnode*>(tnode)->total_nodes_in_subtree, 1);

    tnode->l = left;
    tnode->r = NULL;
    treap_types::recompute_location_statistics(tnode, curr_static);
    EXPECT_EQ(static_cast<LocationTnode*>(tnode)->total_versions_in_subtree, 2008);
    EXPECT_EQ(static_cast<LocationTnode*>(tnode)->total_nodes_in_subtree, 101);

    tnode->l = NULL;
    tnode->r = right;
    treap_types::recompute_location_statistics(tnode, curr_static);
    EXPECT_EQ(static_cast<LocationTnode*>(tnode)->total_versions_in_subtree, 4008);
    EXPECT_EQ(static_cast<LocationTnode*>(tnode)->total_nodes_in_subtree, 201);

    tnode->l = left;
    tnode->r = right;
    treap_types::recompute_location_statistics(tnode, curr_static);
    EXPECT_EQ(static_cast<LocationTnode*>(tnode)->total_versions_in_subtree, 6008);
    EXPECT_EQ(static_cast<LocationTnode*>(tnode)->total_nodes_in_subtree, 301);
}