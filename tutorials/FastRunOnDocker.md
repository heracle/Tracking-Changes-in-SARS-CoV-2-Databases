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

For having this tutorial run fast, we will take into account only a very limited amount of sequences that are not representative for any viral analysis. For analysis, please see the other [RunFull_Amazonaws Tutorial](RunFull_Amazonaws.md) which will take much longer to run. 

From 3 terminal tabs run the following 3 commands:

```
cd <git_root>/scripts
mkdir /tmp/aws_data

python3 aws_download_aligned_and_metadata.py -d 2021-08-11 --limit 2700 --region Europe --tmp_dir /tmp/aws_data -o <git_root>/data/aws_europe_aligned_lim2700

python3 aws_download_aligned_and_metadata.py -d 2021-11-11 --limit 3000 --region Europe --tmp_dir /tmp/aws_data -o <git_root>/data/aws_europe_aligned_lim3000

python3 aws_download_aligned_and_metadata.py -d 2022-01-11 --limit 3100 --region Europe --tmp_dir /tmp/aws_data -o <git_root>/data/aws_europe_aligned_lim3100
```

Now, we have three files in the required JSON viral format that can be used by the tracking changes tool.

## Append all the snapshots to one Tool Data Structure

Running the following commands:

```
cd <git_root>/build_docker
./run create -o ../data/aws1_2021-08-11.h5 ../data/aws_europe_aligned_lim2700_2021-08-11.json
./run append -i ../data/aws1_2021-08-11.h5 -o ../data/aws2_2021-11-11.h5 ../data/aws_europe_aligned_lim3000_2021-11-11.json
./run append -i ../data/aws2_2021-11-11.h5 -o ../data/aws3_2022-01-11.h5 ../data/aws_europe_aligned_lim3100_2022-01-11.json
```

Will create one file `<git_root>/data/aws3_2022-01-11.h5` that contains non-duplicateded data from all the three given snapshots. To see how many nodes each snapshot has, run:

```
tracking-changes/build$ ./run stats ../data/aws3_2022-01-11.h5 

Total number of saved snapshots: 3
Size of 'data' field:3253
Snapshot 'aws_europe_aligned_lim2700_2021-08-11.json' contains 2700 treap nodes.
Snapshot 'aws_europe_aligned_lim3000_2021-11-11.json' contains 3000 treap nodes.
Snapshot 'aws_europe_aligned_lim3100_2022-01-11.json' contains 3100 treap nodes.
```

We can run queries like "which base-pair was most often edited between these 3 snapshot versions?" with the following command:

```
./run query -q bp_freq -i ../data/aws3_2022-01-11.h5 --snapshot "aws_europe_aligned_lim3100_2022-01-11.json" ""
```

This will return that base-pairs with aligned index `27404` and `27461` have been edited 139 times and, a couple of other base-pairs were edited less than 10 times.

Another available query returns "how many bp insertions, replacements and deletions occured between these versions considering a certain region prefix?". For example, we can compare the data for Switzerland, Germany and UK with the following query:

```
build$ ./run query -q cnt_indels -i ../data/aws3_2022-01-11.h5 --snapshot "aws_europe_aligned_lim3100_2022-01-11.json" "Europe / Switzerland" "Europe / Germany" "Europe / United Kingdom"
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

<!-- We need to run several preprocessing steps to add the `owner` and the `aligned sequence` fields to the standard GISAID SARS-CoV-2 snapshot. To facilitate the usage in the `Euler` cluster environment, a couple of scripts are already available to generate a TCVD snapshot file.

In the Docker container, we need to run the following commands:

```
mkdir -p /tmp/gisaid_data/gisaid_preproc_results
cd /tmp/gisaid_data/gisaid_preproc_results
wget --content-disposition https://polybox.ethz.ch/index.php/s/YhiY3ptw5LF6OzJ/download # lookup table: sequence hash to align sequence
wget --content-disposition https://polybox.ethz.ch/index.php/s/okkZtnCqhBinPUy/download # submitter information
bsub unxz lookup_2022-03-13.json.xz
<TCVD git root>/src/external_libraries/nextclade-Linux-x86_64 dataset get --name 'sars-cov-2' --output-dir 'ref_sars-cov-2'
```

We need to use a smaller database. -->