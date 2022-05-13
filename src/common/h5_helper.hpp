#pragma once

#include "H5Cpp.h"

#include <vector>

namespace H5Helper {

int64_t get_int64_attr_from(const H5::Group &h5, const std::string &attr_name);
uint64_t get_uint64_attr_from(const H5::Group &h5, const std::string &attr_name);
std::string get_string_attr_from(const H5::Group &h5, const std::string &attr_name);

void set_int64_hdf5_attr(const int64_t value, H5::Group *h5, const std::string &attr_name);
void set_uint64_hdf5_attr(const uint64_t value, H5::Group *h5, const std::string &attr_name);
void set_string_hdf5_attr(const std::string value, H5::Group *h5, const std::string &attr_name);

// write_h5_dataset stores an entire list of strings.
void write_h5_dataset(const std::vector<std::string> &elems, H5::Group *h5, const std::string &dataset_name);
// read_h5_dataset deserialize a list of strings (stored by 'write_h5_dataset' function).
std::vector<std::string> read_h5_dataset(const H5::Group &h5, const std::string &dataset_name);

// create_extendable_h5_dataset creates a space in the h5 file for storing an extendable "std::vector<std::string> v".
void create_extendable_h5_dataset(H5::Group &h5, const std::string &dataset_name);
// append_extendable_h5_dataset appends a list of strings to the existent extendable "std::vector<std::string> v".
void append_extendable_h5_dataset(const std::vector<std::string> &elems, H5::Group &h5, const std::string &dataset_name);
// get_from_extendable_h5_dataset returns the string with index 'id' stored as an extendable string array.
std::string get_from_extendable_h5_dataset(uint64_t id, const H5::Group &h5, const std::string &dataset_name);

template <typename T>
// write_h5_int_to_dataset stores an entire list of integers.
void write_h5_int_to_dataset(const std::vector<T> elems, H5::Group *h5, const std::string &dataset_name);

template <typename T>
// read_h5_int_to_dataset deserialize an entire list of integers.
std::vector<T> read_h5_int_to_dataset(const H5::Group &h5, const std::string &dataset_name);

} // namespace H5Helper