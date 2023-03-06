# Query: Count Hosts

## Code Filepaths

[src/queries/count_hosts.hpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/queries/count_hosts.hpp)

[src/queries/count_hosts.cpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/queries/count_hosts.cpp)

## Description

Counts the number of sequences with different hosts for a certain set of geographic locations.

## Query Sample

`./run query -q cnt_hosts -i data.h5 --snapshot seq_v1.provision.json,seq_v2.provision.json,seq_v2.provision.json "Void" "Oceania" "Europe" "" `

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
    "host": map<string, int>.
}
```

Example
```
{
  "Europe": {
    "cnt_host_test_v2.provision.json": {
      "Dog": 1,
      "Human": 1
    }
  }
},
{
  "Oceania": {
    "cnt_host_test_v1.provision.json": {
      "Cat": 1,
      "Human": 2
    },
    "cnt_host_test_v2.provision.json": {
      "Cat": 1,
      "Human": 2
    }
  }
}
```


## Database fields

More details about database [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/database.md).

| Index          | Name        | Description|
| ------------- | ----------- | ---------- |
| 0  | `covv_host`    | TODO.|

## Static Data fields

More details about Static Data [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data.md).

This query uses [Static Data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data/location.md) optimized for sorting the location in lexicographic order.

| Index          | Name        | Description|
| ------------- | ----------- | ---------- |
| 0  | `tnode_key`    | TODO.|
| 1  | `is_human_host`    | TODO.|


## Tnode Data fields

More details about Tnode Data [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data.md).

This query uses [Tnode Data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data/location.md) optimized for sorting the location in lexicographic order.

Nothing used from Tnode Data.


## Time complexity

This query has a time complexity of O(N) per snapshot id per input location. To get the result for a given location, all sequences that matches the given geographic location must be iterated.
