# TCVD Introduction Tutorial for Docker

This tutorial has the purpose of presenting an overview of the Traching Changes tool capabilities. The following steps will make basic setup for the minimal experience of making the tool work.

The tool works with JSON viral snapshots that contain the following fields:

- Accession ID
- Collection Date
- Location
- Genome Sequence (in the aligned format)
- Owner Lab

Thus, we will need to download a few snapshots of a viral database ([amazonaws](https://nextstrain-data.s3.amazonaws.com) in this case) and compose a JSON that contains all the mentioned fields.  

## Get into the docker container

Build the docker image:
```
docker build -t ctc_docker:v1 .
```

And run it inside a docker container:
```
docker run -i -t ctc_docker:v1 /bin/bash
```

## Get the Amazonaws data

To ensure that the setup in this tutorial does not take too much time, we will take into account only a limited and unrepresentative viral cohort. If you prefer to do the setup for a real analysis on the entire amount of data available, just remove the `--limit` flag and, to not restrict to Europe only, set `--region=""` (that means worldwide sequences). Then, all the next steps from the tutorial will be the same for both limited and non-limited cases. 

Create the directories where the setup processes and saves the sequences:

```
mkdir /tmp/aws_data
mkdir ../data
```

From 3 terminal tabs run the following 3 commands:

```
<git_root>/scripts$ python3 aws_download_aligned_and_metadata.py -d 2021-08-11 --limit 2700 --region Europe --tmp_dir /tmp/aws_data -o ../data/aws_europe_aligned_lim2700

<git_root>/scripts$ python3 aws_download_aligned_and_metadata.py -d 2021-11-11 --limit 3000 --region Europe --tmp_dir /tmp/aws_data -o ../data/aws_europe_aligned_lim3000

<git_root>/scripts$ python3 aws_download_aligned_and_metadata.py -d 2022-01-11 --limit 3100 --region Europe --tmp_dir /tmp/aws_data -o ../data/aws_europe_aligned_lim3100
```

When these finish to run, we will have three sets/snapshots of sequences in the proper format to be used by the tracking changes tool.

## Append all the snapshots to one Tool Data Structure

Run the following commands to compose an `.h5` file that gets together all the three snapshots:

```
<git_root>/build_docker$ ./run create -o ../data/aws1_2021-08-11.h5 ../data/aws_europe_aligned_lim2700_2021-08-11.json

<git_root>/build_docker$ ./run append -i ../data/aws1_2021-08-11.h5 -o ../data/aws2_2021-11-11.h5 ../data/aws_europe_aligned_lim3000_2021-11-11.json

<git_root>/build_docker$ ./run append -i ../data/aws2_2021-11-11.h5 -o ../data/aws3_2022-01-11.h5 ../data/aws_europe_aligned_lim3100_2022-01-11.json
```

Will create one file `<git_root>/data/aws3_2022-01-11.h5` that contains non-duplicateded data from all the three given snapshots. To see how many nodes each snapshot has, run:

```
tracking-changes/build_docker$ ./run stats ../data/aws3_2022-01-11.h5 

Total number of saved snapshots: 3
Size of 'data' field:3253
Snapshot 'aws_europe_aligned_lim2700_2021-08-11.json' contains 2700 treap nodes.
Snapshot 'aws_europe_aligned_lim3000_2021-11-11.json' contains 3000 treap nodes.
Snapshot 'aws_europe_aligned_lim3100_2022-01-11.json' contains 3100 treap nodes.
```

We can run queries like "which base-pair was most often edited between these 3 snapshot versions?" with the following command:

```
<git_root>/build_docker$ ./run query -q bp_freq -i ../data/aws3_2022-01-11.h5 --snapshot "aws_europe_aligned_lim3100_2022-01-11.json" ""
```

This will return that base-pairs with aligned index `27404` and `27461` have been edited 139 times and, a couple of other base-pairs were edited less than 10 times.

Another available query returns "how many bp insertions, replacements and deletions occured between these versions considering a certain region prefix?". For example, we can compare the data for Switzerland, Germany and UK with the following query:

```
<git_root>/build_docker$ ./run query -q cnt_indels -i ../data/aws3_2022-01-11.h5 --snapshot "aws_europe_aligned_lim3100_2022-01-11.json" "Europe / Switzerland" "Europe / Germany" "Europe / United Kingdom"
```

For the 3 snapshots from this tutorial, the answer should be

```
Snapshots:
0.	aws_europe_aligned_lim3100_2022-01-11.json

Target key 'Europe / Switzerland'
Insertions:	1	
Deletions:	0	
Modified:	0	

Target key 'Europe / Germany'
Insertions:	98	
Deletions:	0	
Modified:	0	

Target key 'Europe / United Kingdom'
Insertions:	2841	
Deletions:	0	
Modified:	153
```
