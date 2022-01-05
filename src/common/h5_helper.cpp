#include "h5_helper.hpp"

#include "constants.hpp"
#include <iostream>

namespace H5Helper {

H5::Attribute attr;
H5::IntType uint32_datatype(H5::PredType::STD_U32LE);
H5::IntType int32_datatype(H5::PredType::STD_I32LE);
H5::IntType uint64_datatype(H5::PredType::STD_U64LE);
H5::StrType str_datatype(H5::PredType::C_S1, H5T_VARIABLE);
H5::DataSpace simple_dataspace(H5S_SCALAR);

uint32_t get_uint32_attr_from(const H5::Group &h5, const std::string &attr_name) {
    uint32_t value;
    attr = h5.openAttribute(attr_name);
    attr.read(uint32_datatype, &value);
    return value;
}

int32_t get_int32_attr_from(const H5::Group &h5, const std::string &attr_name) {
    int32_t value;
    attr = h5.openAttribute(attr_name);
    attr.read(int32_datatype, &value);
    return value;
}

uint64_t get_uint64_attr_from(const H5::Group &h5, const std::string &attr_name) {
    uint64_t value;
    attr = h5.openAttribute(attr_name);
    attr.read(uint64_datatype, &value);
    return value;
}

std::string get_string_attr_from(const H5::Group &h5, const std::string &attr_name) {
    std::string value;
    attr = h5.openAttribute(attr_name);
    attr.read(str_datatype, value);
    return value;
}

void set_int32_hdf5_attr(const int32_t value, H5::Group *h5, const std::string &attr_name) {
    attr = h5->createAttribute(attr_name, int32_datatype, simple_dataspace);
    attr.write(int32_datatype, &value);
}

void set_uint64_hdf5_attr(const uint64_t value, H5::Group *h5, const std::string &attr_name) {
    attr = h5->createAttribute(attr_name, uint64_datatype, simple_dataspace);
    attr.write(uint64_datatype, &value);
}

void set_uint32_hdf5_attr(const uint32_t value, H5::Group *h5, const std::string &attr_name) {
    attr = h5->createAttribute(attr_name, uint32_datatype, simple_dataspace);
    attr.write(uint32_datatype, &value);
}

void set_string_hdf5_attr(const std::string value, H5::Group *h5, const std::string &attr_name) {
    attr = h5->createAttribute(attr_name, str_datatype, simple_dataspace);
    attr.write(str_datatype, value); 
}

} // namespace H5Helper