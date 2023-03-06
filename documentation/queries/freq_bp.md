# Query: Count Indels

## Code Filepaths

[src/queries/count_indels.hpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/queries/freq_bp.hpp)

[src/queries/count_indels.cpp](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/src/queries/freq_bp.cpp)

## Description

Returns a table with the most frequently altered base-pair indices among all base-pair sequence indices.
Furthermore, it mentions which laboratory was doing the change and the
specific characters that were replaced.

## Query Sample

`./run query -q bp_freq -i data.h5 --snapshot seq_v1.provision.json "Oceania"`

## Flags available

| Flag          | Type        | Description|
| ------------- | ----------- | ---------- |
| `--snapshot`  | [string]    | Indicate the list of snapshots to use as targets for the query.|
| `--compute-total-owner-cnt`   | bool    | Print how many uploads each owner group has. |
| `--top-to-print ` | int | Set a maximum number of results to print. |
| `--exclude-deleted` | bool | Exclude all bp alterations for the sequences that were later deleted.|


## JSON Output Format

```
OUTPUT := [LOCATION_ANSWER] 

LOCATION_ANSWER := {
    "name": [SNAPSHOT_ANSWER]
}

SNAPSHOT_ANSWER := {
    "bp": [BP_DATA],
    "list only top X bp": int,
    "list only top X owners": int,
    "owners": [OWNER_DATA]
}

OWNER_DATA := {
    "name": string,
    "number of edits": int,
    "number of uploads": int
}

BP_DATA := {
    "bp index": int,
    "number edits": int,
    "number owners": int,
    "owner": [BP_OWNER_DATA]
}

BP_OWNER_DATA := {
    "number of edits": int,
    "owner name": string,
    "per bp distribution": [PER_BP_DISTRIBUTION],
    "ratio of current bp edits": double
}

PER_BP_DISTRIBUTION := {
    "kind": string,
    "number of edits": int,
    "ratio of current owner edits": double
}
```

Example
```
{
  "Oceania": {
    "modified_sequence_v2.provision.json": {
      "bp": [
        {
          "bp index": 112,
          "number edits": 2,
          "number owners": 2,
          "owner": [
            {
              "number of edits": 1,
              "owner name": "ow3",
              "per bp distribution": [
                {
                  "kind": "A>C",
                  "number of edits": 1,
                  "ratio of current owner edits": "1.00"
                }
              ],
              "ratio of current bp edits": "0.50"
            },
            {
              "number of edits": 1,
              "owner name": "ow1",
              "per bp distribution": [
                {
                  "kind": "A>G",
                  "number of edits": 1,
                  "ratio of current owner edits": "1.00"
                }
              ],
              "ratio of current bp edits": "0.50"
            }
          ]
        },
        {
          "bp index": 42,
          "number edits": 1,
          "number owners": 1,
          "owner": [
            {
              "number of edits": 1,
              "owner name": "ow3",
              "per bp distribution": [
                {
                  "kind": "T>C",
                  "number of edits": 1,
                  "ratio of current owner edits": "1.00"
                }
              ],
              "ratio of current bp edits": "1.00"
            }
          ]
        },
        {
          "bp index": 157,
          "number edits": 1,
          "number owners": 1,
          "owner": [
            {
              "number of edits": 1,
              "owner name": "ow3",
              "per bp distribution": [
                {
                  "kind": "A>C",
                  "number of edits": 1,
                  "ratio of current owner edits": "1.00"
                }
              ],
              "ratio of current bp edits": "1.00"
            }
          ]
        }
      ],
      "list only top X bp": 50,
      "list only top X owners": 50,
      "owner": [
        {
          "name": "ow1",
          "number of edits": 1,
          "number of uploads": 1
        },
        {
          "name": "ow3",
          "number of edits": 1,
          "number of uploads": 1
        }
      ]
    }
  }
}
```

## Database fields

More details about database [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/database.md).

| Index          | Name        | Description|
| ------------- | ----------- | ---------- |
| 0  | `owner`    | TODO.|

## Static Data fields

More details about Static Data [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data.md).

This query uses [Static Data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/static_data/location.md) optimized for sorting the location in lexicographic order.

| Index          | Name        | Description|
| ------------- | ----------- | ---------- |
| 0  | `tnode_key`    | TODO.|
| 1  | `bp_alterations`    | TODO.|


## Tnode Data fields

More details about Tnode Data [here](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data.md).

This query uses [Tnode Data](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/tree/main/documentation/tnode_data/location.md) optimized for sorting the location in lexicographic order.

Nothing used from Tnode Data.


## Time complexity

This query has a time complexity of O(N) per snapshot id per input location. To get the result for a given location, all sequences that matches the given geographic location must be iterated.