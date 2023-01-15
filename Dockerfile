# This composes a docker image and installs a ready to run tracking changes tool.

FROM debian:bullseye as builder

RUN apt-get update --fix-missing
# If this installation step fails because of docker internet access, the problem could be caused by an active VPN on the host machine.
# Please check the solutions described in https://stackoverflow.com/questions/61567404/docker-temporary-failure-resolving-deb-debian-org.
RUN apt-get install -y curl libtool cmake gcc-10 gcc-10-base g++-10 libcurl4-openssl-dev wget python3 xz-utils python3-pandas python3-requests

COPY . /app
RUN mkdir build_docker

# HDF5 download.
RUN mkdir /soft
WORKDIR /soft
RUN wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.2/src/CMake-hdf5-1.12.2.tar.gz
RUN tar -zxvf CMake-hdf5-1.12.2.tar.gz

# Build the HDF5 library in /soft/CMake-hdf5-1.12.2/HDF_Group/HDF5/1.12.2/.
WORKDIR /soft/CMake-hdf5-1.12.2
RUN CC="gcc-10" CXX="g++-10" ctest -S HDF5config.cmake,BUILD_GENERATOR=Unix -C Release -VV -O hdf5.log
RUN ./HDF5-1.12.2-Linux.sh --skip-license

# Build the tracking changes tool.
WORKDIR /app/build_docker
RUN CC="gcc-10" CXX="g++-10" HDF5_ROOT="/soft/CMake-hdf5-1.12.2/HDF_Group/HDF5/1.12.2/" cmake -D CMAKE_BUILD_TYPE=Release ..
RUN make

# Run the available tests for the tracking changes tool.
RUN ./unit_tests --gtest_brief=0 --gtest_filter="*"
RUN python3 ../integration_tests/main.py
