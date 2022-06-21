# Tool for Tracking Changes in Viral Databases (TCVD)

## How to install

### Clone this repository

### Install all git submodules

```
git clone https://gitlab.ethz.ch/rmuntean/tracking-changes.git
```

### Install all the external libraries: 
 - curl
 - libtool
 - hdf5

 For the Euler cluster

 ```
module load cmake/3.11.0 libtool/2.4
module load gcc/8.2.0
module load python/3.8.5
module load hdf5
```

### Make the TCVD binary:

```
mkdir <project root path>/build
cd <project root path>/build
cmake -D CMAKE_BUILD_TYPE=Release ..
make
```

## Run unit tests:

```
./unit_tests --gtest_brief=0 --gtest_filter="*"
```

## Run integration tests:
```
python ../integration_tests/main.py
```

## TCVD Tutorial

A [tutorial](Tutorial.md) is available for getting an overview on how to use the tool.