# How a Query works

Usually, a query receives two lists: a set of snapshot labels and another set of filtering criteria. Then, it returns one answer for each query pair: snapshot and input filter criterion (i.e. if x snapshots and y filter criteria are given, then all
the x × y queries will be computed).

# Implementation

To implement a new query, a new query class that inherits `BaseQuery` must be created. The abstract methods required to be defined are the following:

- `get_treap_to_use()` points out what treap sorting criteria to use in that query.
- `reset()` that is run after each processed filter criteria.
- `set_deletion_mode(useDeletionTreap)` used for differentiating between the processing on the normal treap and the treap for deletions.
- `first_enter_into_tnode(key, tnode, staticData, DBData)`
- `second_enter_into_tnode(key, tnode, staticData, DBData)`
- `print_results()` that is run to print results for each filter criteria.

# Additional Mentions

Before implementing a query, the user should think where to store the necessarily data for getting the best performance. The are 3 ways to store data:

- [database](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/database.md) ideal for unlimited storage
- [static data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data.md) for fast available data that can be accessed without restrictions to the sorting criteria
- [tnode data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data.md) for data that can be summed up for consecutive elements in the treap sorted order. This type of access doesn't imply step by step iteration through the data, but O(log) time access to the precomputed sums.