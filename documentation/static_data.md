# Static Data

## Code Filepaths

[src/ds/static_types/...](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/ds/static_types)

## Description

Static Data is one of the three data sources for a query. The other two are [database](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/database.md) and [tnode data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data.md).

Static Data is much faster than Database, but has the disadvantage of not having serialization and local access via HDF5. The entire content of static data is loaded to RAM at all the time, therefore we should avoid storing any expensive field here.

Each query uses only one binary search tree (i.e. treap) that stores the data according to a certain sorting criteria. Therefore, each treap is optimized in a different way to be suitable for a set of specific queries. For a treap we need to define a type of static data and a type of tnode data.

The focus in this doc is on static data, that is static inside the given treap: once set, it will never need to be updated while the treap evolves in time.

## Available Options

A list with all the current available options for static data in correspondence with their treap sorting criteria is the following

- optimized for [accession version](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data/acc_id.md) ascending order.

- optimized for [geographic location](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data/location.md) lexicographic order.

## How to add a new option and use it for queries

A static data object is linked to a particular treap. Therefore, it makes sense to discuss about implementing a new treap structure where to implement a particular static data object.

To create a new treap:

- Create new `hpp` and `cpp` files in src/ds/static_types/.
- Define a new class that inherits `BaseSortedTreap`.
- Create new `hpp` and `cpp` files in src/ds/tnode_types/.
- Define a new class that inherits `Tnode`.
- Implement the following functions:
    - create_new_specialized_tnode()
    - copy_specialized_tnode()
    - get_h5_tnode()
    - reset_get_h5_tnode()
    - append_tnode_data()
    - reset_append_tnode_data()
    - write_tnode_data_to_h5()
    - recompute_statistics()
    - copy_specialized_static_field()
    - get_new_BaseSortedTreap()
    - reset_get_new_BaseSortedTreap()
    - get_unique_from_snapshot_line()
    - reset_get_unique_from_snapshot_line()
    - serialize_acc_id_elem_to_hdf5()
    - Some of these functions are related to static data and others to tnode data.All are required to define a new treap. Please see the [full text](https://doi.org/10.3929/ethz-b-000555485) for more details about these functions.
- Add a new treap key in src/ds/ctc.cpp `CTC::ctc_init()` using this set of functions in a similar way as the existent ones.

Finally, all the fields created inside the new class that inherits `BaseSortedTreap` will compose the static data.