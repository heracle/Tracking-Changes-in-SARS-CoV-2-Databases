#include "gtest/gtest.h"

#include <string>
#include <vector>

#define private public
#define protected public

#include "../ds/database.hpp"
#include "../common/json_helper.hpp"
#include "../common/h5_helper.hpp"

const std::string H5_FILENAME = "__tmp_h5_for_unit_tests.h5";
const std::string PROVISION_TEST_FILEPATH = "../test_data/0000-01-01.provision.json";

TEST(Database, DBConstructor) {
    std::string test_filename = "uint32" + H5_FILENAME; 
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    uint32_t flush_size = 15;
    ds::DB *db = new ds::DB(&h5_file, flush_size);
    EXPECT_EQ(db->flush_size, flush_size);
    EXPECT_EQ(db->db_str_fields.size(), common::SEQ_FIELDS_SZ);

    for (uint32_t i = 0; i < db->db_str_fields.size(); ++i) {
        EXPECT_EQ(db->db_str_fields[i], common::SEQ_FIELDS[i]);
    }

    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}

TEST(Database, WriteBuffData) {
    std::string test_filename = "uint32" + H5_FILENAME; 
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);
    ds::DB *db = new ds::DB(&h5_file);

    common::SeqElemReader *reader = new common::SeqElemReader(PROVISION_TEST_FILEPATH);
    std::vector<common::SeqElem> file_SeqElem = reader->get_aligned_seq_elements(20);

    db->buff_data = file_SeqElem;
    db->write_buff_data();
    EXPECT_EQ(db->buff_data.size(), 0);

    H5::Group tmp_group = H5Gopen(h5_file.getLocId(), "/database", H5P_DEFAULT);
    for (uint32_t field_id = 0; field_id < db->db_str_fields.size(); ++field_id) {
        for (uint32_t i = 0; i < 20; ++i) {
            std::string x = H5Helper::get_from_extendable_h5_dataset(i, tmp_group, db->db_str_fields[field_id]);
            EXPECT_EQ(x, file_SeqElem[i].covv_data[field_id]);
        }
    }

    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}

TEST(Database, InsertGetElement) {   

    for (uint32_t flush_size : std::vector<uint32_t>{4, 3, 1, 9, 10, 18}) {
        std::string test_filename = std::to_string(flush_size) + H5_FILENAME; 
        H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);

        ds::DB *db = new ds::DB(&h5_file, flush_size);

        common::SeqElemReader *reader = new common::SeqElemReader(PROVISION_TEST_FILEPATH);
        std::vector<common::SeqElem> file_SeqElem = reader->get_aligned_seq_elements(20);

        for (uint32_t i = 0; i < file_SeqElem.size(); ++i) {
            file_SeqElem[i].prv_db_id = i + 1050;

            db->insert_element(file_SeqElem[i]);
            EXPECT_EQ(db->buff_data.size(), (i + 1) % flush_size);
            EXPECT_EQ(db->data_size, i + 1);
        }
        EXPECT_EQ(db->data_size, file_SeqElem.size());
        db->write_buff_data();
        delete db;

        // open again db for making sure that the elements are stored in the h5 file.

        ds::DB *db2 = new ds::DB(&h5_file, flush_size);

        for (int32_t i = file_SeqElem.size() - 1; i >= 0; --i) {
            for (uint32_t field_id = 0; field_id < common::SEQ_FIELDS_SZ; ++field_id) {
                EXPECT_EQ(db2->get_element(i).covv_data[field_id], file_SeqElem[i].covv_data[field_id]);
            }
            EXPECT_EQ(db2->get_element(i).prv_db_id, file_SeqElem[i].prv_db_id);
        }

        delete db2;
        h5_file.close();
        EXPECT_EQ(remove(test_filename.c_str()), 0);
    }
}

TEST(Database, CloneDB) {
    uint32_t flush_size = 4;

    std::string test_filename = std::to_string(flush_size) + H5_FILENAME; 
    H5::H5File h5_file(test_filename, H5F_ACC_TRUNC);

    ds::DB *db = new ds::DB(&h5_file, flush_size);

    common::SeqElemReader *reader = new common::SeqElemReader(PROVISION_TEST_FILEPATH);
    std::vector<common::SeqElem> file_SeqElem = reader->get_aligned_seq_elements(20);

    for (uint32_t i = 0; i < file_SeqElem.size(); ++i) {
        file_SeqElem[i].prv_db_id = i + 1050;

        db->insert_element(file_SeqElem[i]);
        EXPECT_EQ(db->buff_data.size(), (i + 1) % flush_size);
        EXPECT_EQ(db->data_size, i + 1);
    }
    EXPECT_EQ(db->data_size, file_SeqElem.size());
    db->write_buff_data();

    H5::H5File h5_file2( "second" + test_filename, H5F_ACC_TRUNC);
    ds::DB *db2 = new ds::DB(&h5_file2, flush_size);

    db2->clone_db(*db);
    delete db;

    // open again db for making sure that the elements are stored in the h5 file.
    for (int32_t i = file_SeqElem.size() - 1; i >= 0; --i) {
        for (uint32_t field_id = 0; field_id < common::SEQ_FIELDS_SZ; ++field_id) {
            EXPECT_EQ(db2->get_element(i).covv_data[field_id], file_SeqElem[i].covv_data[field_id]);
        }
        EXPECT_EQ(db2->get_element(i).prv_db_id, file_SeqElem[i].prv_db_id);
    }

    delete db2;
    h5_file.close();
    EXPECT_EQ(remove(test_filename.c_str()), 0);
}