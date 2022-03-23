#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <stdio.h>

#define private public
#define protected public

#include "../common/h5_helper.hpp"

const std::string H5_FILENAME = "__tmp_h5_for_unit_tests.h5";
const std::string H5_GROUPNAME = "/unit_test";

TEST(H5Helper, GetSetAttr_uint32) {
    std::string test_filename = "uint32" + H5_FILENAME; 
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    H5::Group tmp_group = h5_file.createGroup(H5_GROUPNAME);

    H5Helper::set_uint32_hdf5_attr((uint32_t)50, &tmp_group, "first");
    H5Helper::set_uint32_hdf5_attr(150, &tmp_group, "second");
    H5Helper::set_uint32_hdf5_attr((1<<31) + 500, &tmp_group, "third");

    tmp_group.close();
    h5_file.close();

    h5_file = H5::H5File(test_filename, H5F_ACC_RDONLY);
    tmp_group = H5Gopen(h5_file.getLocId(), H5_GROUPNAME.c_str(), H5P_DEFAULT);

    EXPECT_EQ(H5Helper::get_uint32_attr_from(tmp_group, "second"), 150);
    EXPECT_EQ(H5Helper::get_uint32_attr_from(tmp_group, "third"), (1<<31) + 500);
    EXPECT_EQ(H5Helper::get_uint32_attr_from(tmp_group, "first"), 50);

    tmp_group.close();
    h5_file.close();

    EXPECT_EQ(remove(test_filename.c_str()), 0);
}

TEST(H5Helper, GetSetAttr_int32) {
    std::string test_filename = "int32" + H5_FILENAME;
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    H5::Group tmp_group = h5_file.createGroup(H5_GROUPNAME);

    H5Helper::set_int32_hdf5_attr((int32_t)50, &tmp_group, "first");
    H5Helper::set_int32_hdf5_attr(150, &tmp_group, "second");
    H5Helper::set_int32_hdf5_attr((1<<31) + 500, &tmp_group, "third");

    tmp_group.close();
    h5_file.close();

    h5_file = H5::H5File(test_filename, H5F_ACC_RDONLY);
    tmp_group = H5Gopen(h5_file.getLocId(), H5_GROUPNAME.c_str(), H5P_DEFAULT);

    EXPECT_EQ(H5Helper::get_int32_attr_from(tmp_group, "second"), 150);
    EXPECT_EQ(H5Helper::get_int32_attr_from(tmp_group, "third"), (1<<31) + 500);
    EXPECT_EQ(H5Helper::get_int32_attr_from(tmp_group, "first"), 50);

    tmp_group.close();
    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}

TEST(H5Helper, GetSetAttr_uint64) {
    std::string test_filename = "uint64" + H5_FILENAME;

    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    H5::Group tmp_group = h5_file.createGroup(H5_GROUPNAME);

    H5Helper::set_uint64_hdf5_attr((uint64_t)50, &tmp_group, "first");
    H5Helper::set_uint64_hdf5_attr(150, &tmp_group, "second");
    H5Helper::set_uint64_hdf5_attr((1ULL<<63) + 500, &tmp_group, "third");

    tmp_group.close();
    h5_file.close();

    h5_file = H5::H5File(test_filename, H5F_ACC_RDONLY);
    tmp_group = H5Gopen(h5_file.getLocId(), H5_GROUPNAME.c_str(), H5P_DEFAULT);

    EXPECT_EQ(H5Helper::get_uint64_attr_from(tmp_group, "second"), 150);
    EXPECT_EQ(H5Helper::get_uint64_attr_from(tmp_group, "third"), (1ULL<<63) + 500);
    EXPECT_EQ(H5Helper::get_uint64_attr_from(tmp_group, "first"), 50);

    tmp_group.close();
    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}

TEST(H5Helper, GetSetAttr_str) {
    std::string test_filename = "str" + H5_FILENAME;
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    H5::Group tmp_group = h5_file.createGroup(H5_GROUPNAME);

    H5Helper::set_string_hdf5_attr("AAA", &tmp_group, "first");
    H5Helper::set_string_hdf5_attr("BBB", &tmp_group, "second");
    H5Helper::set_string_hdf5_attr("DEC", &tmp_group, "third");

    tmp_group.close();
    h5_file.close();

    h5_file = H5::H5File(test_filename, H5F_ACC_RDONLY);
    tmp_group = H5Gopen(h5_file.getLocId(), H5_GROUPNAME.c_str(), H5P_DEFAULT);

    EXPECT_EQ(H5Helper::get_string_attr_from(tmp_group, "second"), "BBB");
    EXPECT_EQ(H5Helper::get_string_attr_from(tmp_group, "third"), "DEC");
    EXPECT_EQ(H5Helper::get_string_attr_from(tmp_group, "first"), "AAA");

    tmp_group.close();
    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}

TEST(H5Helper, GetSetAttr_h5dataset) {
    std::string test_filename = "h5dataset" + H5_FILENAME;
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    H5::Group tmp_group = h5_file.createGroup(H5_GROUPNAME);

    std::vector<std::string> v_tmp = {"AAA", "BBBB", "CCCCC", "D"};
    H5Helper::write_h5_dataset(v_tmp, &tmp_group, "first");

    tmp_group.close();
    h5_file.close();

    h5_file = H5::H5File(test_filename, H5F_ACC_RDONLY);
    tmp_group = H5Gopen(h5_file.getLocId(), H5_GROUPNAME.c_str(), H5P_DEFAULT);

    EXPECT_EQ(H5Helper::read_h5_dataset(tmp_group, "first"), v_tmp);

    tmp_group.close();
    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}

TEST(H5Helper, GetSetAttr_int_dataset) {
    std::string test_filename = "int_dataset" + H5_FILENAME;
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    H5::Group tmp_group = h5_file.createGroup(H5_GROUPNAME);

    std::vector<uint64_t> v_tmp = {(1ULL<<63) + 500, (1<<30) + 10, 20, 56};
    H5Helper::write_h5_int_to_dataset(v_tmp, &tmp_group, "first");

    tmp_group.close();
    h5_file.close();

    h5_file = H5::H5File(test_filename, H5F_ACC_RDONLY);
    tmp_group = H5Gopen(h5_file.getLocId(), H5_GROUPNAME.c_str(), H5P_DEFAULT);

    EXPECT_EQ(H5Helper::read_h5_int_to_dataset<uint64_t>(tmp_group, "first"), v_tmp);

    tmp_group.close();
    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}

TEST(H5Helper, GetSetAttr_ext_h5dataset) {
    std::string test_filename = "ext_h5dataset" + H5_FILENAME;
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    H5::Group tmp_group = h5_file.createGroup(H5_GROUPNAME);

    std::string dataset_name = "test";
    H5Helper::create_extendable_h5_dataset(tmp_group, dataset_name);

    std::vector<std::string> v_tmp = {"AAA", "BBBB", "CCCCC", "D", "E"};
    H5Helper::append_extendable_h5_dataset(std::vector<std::string>(v_tmp.begin(), v_tmp.begin() + 2), tmp_group, dataset_name);
    H5Helper::append_extendable_h5_dataset(std::vector<std::string>(v_tmp.begin() + 2, v_tmp.begin() + 3), tmp_group, dataset_name);
    H5Helper::append_extendable_h5_dataset(std::vector<std::string>(v_tmp.begin() + 3, v_tmp.begin() + v_tmp.size()), tmp_group, dataset_name);

    tmp_group.close();
    h5_file.close();

    h5_file = H5::H5File(test_filename, H5F_ACC_RDONLY);
    tmp_group = H5Gopen(h5_file.getLocId(), H5_GROUPNAME.c_str(), H5P_DEFAULT);

    EXPECT_EQ(H5Helper::get_from_extendable_h5_dataset(2, tmp_group, dataset_name), v_tmp[2]);
    EXPECT_EQ(H5Helper::get_from_extendable_h5_dataset(4, tmp_group, dataset_name), v_tmp[4]);
    EXPECT_EQ(H5Helper::get_from_extendable_h5_dataset(1, tmp_group, dataset_name), v_tmp[1]);
    EXPECT_EQ(H5Helper::get_from_extendable_h5_dataset(3, tmp_group, dataset_name), v_tmp[3]);
    EXPECT_EQ(H5Helper::get_from_extendable_h5_dataset(0, tmp_group, dataset_name), v_tmp[0]);

    tmp_group.close();
    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}