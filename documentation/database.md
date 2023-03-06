# Database

## Code Filepaths

[src/ds/database.hpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/ds/database.hpp)

[src/ds/database.cpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/ds/database.cpp)

[src/common/constants.cpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/common/constants.cpp) SEQ_FIELDS_SZ, SEQ_FIELDS, SEQ_FIELDS_TO_ID objects.

## Description

Database is one of the three data sources for a query. The other two are [static data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data.md) and [tnode data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data.md).

Database is the slowest among those three, but the only one with unlimited storage. When accessed, HDF5 does a local serialization and loads only a small part of it to RAM.

## Available fields

| Index          | Name        | Description|
| ------------- | ----------- | ---------- |
| 0  | covv_accession_id    | TODO.|
| 1   | covv_collection_date    | TODO. |
| 2 | covv_location | TODO. |
| 3 | sequence | TODO.|
| 4 | owner | TODO.|
| 5 | covv_host | TODO.|

## How to add a new field and use it for queries

Increase `SEQ_FIELDS_SZ` in common/constants.hpp.

Add the new field to both `SEQ_FIELDS` and `SEQ_FIELDS_TO_ID`.

This changes are enough to update `SeqElem` in (common/json_helper) with the new field. Then, `./run create` and `./run update` will save the new field in the h5 output if available in the `.provision.json` provided.

Inside a query we can access any database field via `database_id` index inside `BaseSortedTreap` class.

The user needs to recompute all the h5 files to gain data for a new field.

## Additional mentions

- There is no upper limit on the amount of data stored as database. For example, we store the entire DNA sequence as a database field.
- Accessing database data is much slower than for static or tnode data. It implies hdf5 deserialization. For best performance, minimize database requests inside a query.