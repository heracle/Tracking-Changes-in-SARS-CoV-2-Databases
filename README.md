# Tracking Changes in Viral Databases (TCVD)

This project proposes a tool to highlight meaningful differences between SARS-CoV-2 viral snapshots that can be used for efficient inter-version comparison queries.

See the [full text](https://doi.org/10.3929/ethz-b-000555485) of my Master's Thesys that underlines all tool's capabilities.

# How to install

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

## Docker Available

All these steps have already been added inside a docker file to be easier to get the tool available on any machine.

Build the image:
```
sudo docker build -t ctc_docker:v1 .
```

Use the tool from the docker container:
```
sudo docker run -i -t ctc_docker:v1 /bin/bash
```

## Installation on ETH Euler Cluster 

All these dependency steps are not in case of using the Euler Cluster available to ETH staff. Instead, you would need only to load the following modules:

```
module load cmake/3.11.0 libtool/2.4
module load gcc/8.2.0
module load python/3.8.5
module load hdf5
```

## TCVD Tutorial

There are multiple tutorial available for getting an overview on how to use the tool on SARS-CoV-2 databases.

<!-- A [tutorial](Tutorial.md) is available for getting an overview on how to construct the tool HDF5 file for the GISAID SARS-CoV-2 database. -->

One example [Merge Request 52](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/merge_requests/52) is provided as a guideline for implementing a new query and adding a new field to the TCVD database.

Another example [Merge Request 53](https://gitlab.ethz.ch/rmuntean/tracking-changes/-/merge_requests/53) is a second guideline about how to use the treap static field to improve the query runtime.
