#pragma once

#include "H5Cpp.h"

namespace H5Helper {

uint32_t get_uint32_attr_from(const H5::Group &h5, const std::string &attr_name);
int32_t get_int32_attr_from(const H5::Group &h5, const std::string &attr_name);
uint64_t get_uint64_attr_from(const H5::Group &h5, const std::string &attr_name);
std::string get_string_attr_from(const H5::Group &h5, const std::string &attr_name);

void set_uint32_hdf5_attr(const uint32_t value, H5::Group *h5, const std::string &attr_name);
void set_uint64_hdf5_attr(const uint64_t value, H5::Group *h5, const std::string &attr_name);
void set_int32_hdf5_attr(const int32_t value, H5::Group *h5, const std::string &attr_name);
void set_string_hdf5_attr(const std::string value, H5::Group *h5, const std::string &attr_name);

} // namespace H5Helper