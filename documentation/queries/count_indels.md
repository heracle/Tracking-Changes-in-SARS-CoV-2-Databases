# Query: Count Indels

## Code Filepaths

[src/queries/count_indels.hpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/queries/count_indels.hpp)

[src/queries/count_indels.cpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/queries/count_indels.cpp)

## Description

Counts the total number of insertions, deletions, bp alterations that have happened before a given snapshot ID for a certain set of geographic locations.

## Query Sample

`./run query -q cnt_indels -i data.h5 --snapshot seq_v1.provision.json,seq_v2.provision.json,seq_v2.provision.json "Void" "Oceania" "Europe" "" `

## Flags available

| Flag          | Type        | Description|
| ------------- | ----------- | ---------- |
| `--snapshot`  | [string]    | Indicate the list of snapshots to use as targets for the query.|


## JSON Output Format

```
OUTPUT := [LOCATION_ANSWER] 

LOCATION_ANSWER := {
    "name": [SNAPSHOT_ANSWER]
}

SNAPSHOT_ANSWER := {
    "inserted": int,
    "deleted": int,
    "modified": int
}
```

Example
```
{
  "Oceania": {
    "modified_sequence_v1.provision.json": {
      "inserted": 3,
      "modified": 0
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 3,
      "modified": 2
    },
    "modified_sequence_v3.provision.json": {
      "inserted": 3,
      "modified": 4
    }
  }
},
{
  "Europe": {
    "modified_sequence_v1.provision.json": {
      "inserted": 2,
      "modified": 0
    },
    "modified_sequence_v2.provision.json": {
      "inserted": 2,
      "modified": 2
    },
    "modified_sequence_v3.provision.json": {
      "deleted": 1,
      "inserted": 1,
      "modified": 3
    }
  }
},
```

## Database fields

More details about database [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/database.md).

Nothing used from database.

## Static Data fields

More details about Static Data [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data.md).

This query uses [Static Data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data/location.md) optimized for sorting the location in lexicographic order.

| Index          | Name        | Description|
| ------------- | ----------- | ---------- |
| 0  | `tnode_key`    | TODO.|
| 1  | `num_sequence_versions`    | TODO.|


## Tnode Data fields

More details about Tnode Data [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data.md).

This query uses [Tnode Data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data/location.md) optimized for sorting the location in lexicographic order.

| Index          | Name        | Description|
| ------------- | ----------- | ---------- |
| 0  | `total_nodes_in_subtree`    | TODO.|
| 1  | `total_versions_in_subtree`    | TODO.|

## Time complexity

This query has a time complexity of O(log(N)) per snapshot id per input location. This query is mainly using subtree precalculation from tnode data and, thus, doesn't need to iterate all the sequences stored in the treap.
