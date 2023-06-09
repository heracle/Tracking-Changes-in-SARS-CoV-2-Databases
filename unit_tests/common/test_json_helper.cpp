#include "gtest/gtest.h"

#include <string>
#include <vector>

#define private public
#define protected public

#include "../common/json_helper.hpp"

const std::string PROVISION_TEST_FILEPATH = "../test_data/0000-01-01.provision.json";

bool validate_SeqElem(common::SeqElem seq_elem, int64_t id) {
    if (seq_elem.covv_data[0] != "EPI_ISL_" + std::to_string(id)) {
        return false;
    }

    if (seq_elem.covv_data[1].size() != 10 || seq_elem.covv_data[1][4] != '-' || seq_elem.covv_data[1][7] != '-') {
        return false;
    }
    return true;
}

TEST(JsonHelper, SeqElemEqOperator) {
    common::SeqElem x, y;

    for (uint64_t i = 0; i < common::SEQ_FIELDS_SZ; ++i) {
        x.covv_data[i] = "test_" + std::to_string(i);
    }

    y = x;

    for (uint64_t i = 0; i < common::SEQ_FIELDS_SZ; ++i) {
        EXPECT_EQ(y.covv_data[i], "test_" + std::to_string(i));
    }
}

TEST(JsonHelper, GetSeqElemFromJson) {
    rapidjson::Document document;
    std::ifstream f(PROVISION_TEST_FILEPATH);
    std::string json_line;
    std::getline(f, json_line);
    document.Parse(json_line.c_str());

    common::SeqElem seq_elem = common::get_SeqElem_from_json(document);
    EXPECT_EQ(validate_SeqElem(seq_elem, 0), true);
}

TEST(JsonHelper, SeqElemReaderAlignedElems) {
    common::SeqElemReader *reader = new common::SeqElemReader(PROVISION_TEST_FILEPATH);
    const uint64_t num_append_cst = 6;
    uint64_t num_seq_read = 0;
    while(! reader->end_of_file()) {
        std::vector<common::SeqElem> sequences = reader->get_aligned_seq_elements(num_append_cst);

        if (sequences.size() != num_append_cst) {
            EXPECT_EQ(reader->end_of_file(), true);   
        }

        for (const common::SeqElem &seq : sequences) {
            EXPECT_EQ(validate_SeqElem(seq, num_seq_read), true);
            ++num_seq_read;
        }
    }
    EXPECT_EQ(num_seq_read, 20);
    EXPECT_EQ(reader->end_of_file(), true);
}

TEST(JsonHelper, SeqElemReaderGetElem) {
    common::SeqElemReader *reader = new common::SeqElemReader(PROVISION_TEST_FILEPATH);

    for (int64_t id = 0; id < 20; ++id) {
        EXPECT_EQ(reader->end_of_file(), false);
        common::SeqElem seq = reader->get_elem(id);
        EXPECT_EQ(validate_SeqElem(seq, id), true);
    }
    // common::SeqElem seq = reader->get_elem(20);
    EXPECT_EQ(reader->end_of_file(), true);

    delete reader;

    reader = new common::SeqElemReader(PROVISION_TEST_FILEPATH);
    common::SeqElem seq = reader->get_elem(7);
    EXPECT_EQ(validate_SeqElem(seq, 7), true);
    EXPECT_EQ(reader->end_of_file(), false);
    seq = reader->get_elem(15);
    EXPECT_EQ(validate_SeqElem(seq, 15), true);
    EXPECT_EQ(reader->end_of_file(), false);
    seq = reader->get_elem(19);
    EXPECT_EQ(validate_SeqElem(seq, 19), true);
    EXPECT_EQ(reader->end_of_file(), true);

    delete reader;
}

TEST(JsonHelper, SeqElemReaderGetElemInorder) {
    common::SeqElemReader *reader = new common::SeqElemReader(PROVISION_TEST_FILEPATH);
    
    common::SeqElem seq = reader->get_elem(15);
    EXPECT_EQ(validate_SeqElem(seq, 15), true);

    EXPECT_THROW({
        try
        {
            reader->get_elem(7);
        }
        catch( const std::runtime_error& e )
        {
            EXPECT_STREQ( "ERROR -> Elem reader requests elems in not ascending order.", e.what() );
            throw;
        }
    }, std::runtime_error);
}