# Tracking Changes in Viral Databases (TCVD)

This project proposes a tool to highlight meaningful differences between SARS-CoV-2 viral snapshots that can be used for efficient inter-version comparison queries.

See the [full text](https://doi.org/10.3929/ethz-b-000555485) of my Master's Thesys that underlines all tool's capabilities.

# How to install

<details>
  <summary markdown="span"> Manual install of all dependencies </summary>

## Manual install of all dependencies

Ensure that all the following packages are installed:

```
apt-get install -y curl libtool cmake gcc-10 gcc-10-base g++-10 libcurl4-openssl-dev wget python3 xz-utils python3-pandas python3-requests
```

Download and build the HDF5 library:

```
cd <local_libraries>
wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.2/src/CMake-hdf5-1.12.2.tar.gz
tar -zxvf CMake-hdf5-1.12.2.tar.gz
cd CMake-hdf5-1.12.2
CC="gcc-10" CXX="g++-10" ctest -S HDF5config.cmake,BUILD_GENERATOR=Unix -C Release -VV -O hdf5.log
./HDF5-1.12.2-Linux.sh
```

Clone and build this git repository with all submodules:

```
git clone --recurse-submodules https://gitlab.ethz.ch/rmuntean/tracking-changes.git
mkdir tracking-changes/build
cd tracking-changes/build
CC="gcc-10" CXX="g++-10" HDF5_ROOT="<local_libraries>/CMake-hdf5-1.12.2/HDF_Group/HDF5/1.12.2/" cmake -D CMAKE_BUILD_TYPE=Release ..
make
```

Run the available tests and check if everything works well:

```
./unit_tests --gtest_brief=0 --gtest_filter="*"
python3 ../integration_tests/main.py

```
</details>

<details>
  <summary markdown="span"> Build the docker container </summary>

## Build the docker container

All the manual steps to install and link the dependencies have already been added inside a docker file. This facilitates the process to get the tool ready to run on any machine.


Build the docker image:
```
docker build -t ctc_docker:v1 .
```

And run it inside a docker container:
```
docker run -i -t ctc_docker:v1 /bin/bash
```
</details>

<details>
  <summary markdown="span"> Installation on ETH Euler Cluster  </summary> 

## Installation on ETH Euler Cluster

All these dependency steps are not needed in case of using the Euler Cluster available to ETH staff. Instead, you would need only to load the following modules:

```
module load cmake/3.11.0
module load libtool/2.4.6
module load gcc/8.2.0
module load python/3.8.5
module load hdf5
```

Clone and build this git repository with all submodules:

```
git clone --recurse-submodules https://gitlab.ethz.ch/rmuntean/tracking-changes.git
mkdir tracking-changes/build
cd tracking-changes/build
EULER_LDAP="rmuntean" cmake -D CMAKE_BUILD_TYPE=Release ..
make
```

Run the available tests and check if everything works well:

```
./unit_tests --gtest_brief=0 --gtest_filter="*"
python3 ../integration_tests/main.py

```
</details>

# TCVD Tutorial

This tutorial has the purpose of presenting an overview of the Traching Changes tool capabilities. The following steps will make basic setup for the minimal experience of making the tool work.

The tool works with JSON viral snapshots that contain the following fields:

- Accession ID
- Collection Date
- Location
- Genome Sequence (in the aligned format)
- Owner Lab


## Get the AmazonAWS data

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

```
tcvd_snapshot1="../data/aws_europe_aligned_lim2700_2021-08-11.json"
tcvd_snapshot2="../data/aws_europe_aligned_lim3000_2021-11-11.json"
tcvd_snapshot3="../data/aws_europe_aligned_lim3100_2022-01-11.json"
```


## Append all the snapshots to one Tool Data Structure

Run the following commands to compose an `.h5` file that gets together all the three snapshots:

```
<git_root>/build_docker$ ./run create -o ../data/tcvd1_2021-08-11.h5 "${tcvd_snapshot1}"

<git_root>/build_docker$ ./run append -i ../data/tcvd1_2021-08-11.h5 -o ../data/tcvd2_2021-11-11.h5 "${tcvd_snapshot2}"

<git_root>/build_docker$ ./run append -i ../data/tcvd2_2021-11-11.h5 -o ../data/tcvd3_2022-01-11.h5 "${tcvd_snapshot2}"
```

Will create one file `<git_root>/data/tcvd3_2022-01-11.h5` that contains non-duplicateded data from all the three given snapshots. To see how many nodes each snapshot has, run:

```
tracking-changes/build_docker$ ./run stats ../data/tcvd3_2022-01-11.h5 

Total number of saved snapshots: 3
Size of 'data' field:3253
Snapshot 'aws_europe_aligned_lim2700_2021-08-11.json' contains 2700 treap nodes.
Snapshot 'aws_europe_aligned_lim3000_2021-11-11.json' contains 3000 treap nodes.
Snapshot 'aws_europe_aligned_lim3100_2022-01-11.json' contains 3100 treap nodes.
```

We can run queries like "which base-pair was most often edited between these 3 snapshot versions?" with the following command:

```
<git_root>/build_docker$ ./run query -q bp_freq -i ../data/tcvd3_2022-01-11.h5 --snapshot "aws_europe_aligned_lim3100_2022-01-11.json" ""
```

This will return that base-pairs with aligned index `27404` and `27461` have been edited 139 times and, a couple of other base-pairs were edited less than 10 times.

Another available query returns "how many bp insertions, replacements and deletions occured between these versions considering a certain region prefix?". For example, we can compare the data for Switzerland, Germany and UK with the following query:

```
<git_root>/build_docker$ ./run query -q cnt_indels -i ../data/tcvd3_2022-01-11.h5 --snapshot "aws_europe_aligned_lim3100_2022-01-11.json" "Europe / Switzerland" "Europe / Germany" "Europe / United Kingdom"
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