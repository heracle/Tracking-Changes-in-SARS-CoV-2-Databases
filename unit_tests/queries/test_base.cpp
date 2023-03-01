#include "gtest/gtest.h"

#include <vector>
#include <iostream>

#define private public
#define protected public

#include "../queries/base_query.hpp"

TEST(BaseQuery, TestValInt) {
    query_ns::DataQueryObj data;
    data("test")("constant").SetValInt(40);
    data("test")("counter").SetValInt(
        data("test")("counter").GetValInt() + 1
    );
    data("test")("counter").SetValInt(
        data("test")("counter").GetValInt() + 2
    );
    data("test")("counter").SetValInt(
        data("test")("counter").GetValInt() + 4
    );
    data("test")("to_replace").SetValInt(40);
    data("test")("to_replace").SetValInt(20);
    data("test")("negative_value").SetValInt(-30);
    data("test")("zero_value").SetValInt(0);

    // A string label ("2") and an int label (2) are one and the same thing.
    data("int_vs_str")(2).SetValInt(100);
    data("int_vs_str")("2").SetValInt(data("int_vs_str")("2").GetValInt() + 11);
    
    const char *expected_output = R"V0G0N({
  "int_vs_str": {
    "2": 111,
  },
  "test": {
    "constant": 40,
    "counter": 7,
    "negative_value": -30,
    "to_replace": 20,
    "zero_value": 0,
  },
},
)V0G0N";
    EXPECT_EQ(data.GetJsonStr(), expected_output);
}

TEST(BaseQuery, TestValStr) {
    query_ns::DataQueryObj data;
    data("test")("constant").SetValStr("aaa");
    data("test")("append").SetValStr(
        data("test")("append").GetValStr() + "_app"
    );
    data("test")("append").SetValStr(
        data("test")("append").GetValStr() + "_app2"
    );
    data("test")("append").SetValStr(
        data("test")("append").GetValStr() + "_app4"
    );
    data("test")("to_replace").SetValStr("bb");
    data("test")("to_replace").SetValStr("cc");
    data("test")("empty_value").SetValStr("");

    const char *expected_output = R"V0G0N({
  "test": {
    "append": "_app_app2_app4",
    "constant": "aaa",
    "empty_value": "",
    "to_replace": "cc",
  },
},
)V0G0N";
    EXPECT_EQ(data.GetJsonStr(), expected_output);
}

TEST(BaseQuery, TestEmptyDataQueryObj) {
    query_ns::DataQueryObj data;
    const char *expected_output = R"V0G0N("",
)V0G0N";
    EXPECT_EQ(data.GetJsonStr(), expected_output);
}

TEST(BaseQuery, TestJsonTreeStructure) {
    query_ns::DataQueryObj data;
    data("A")("str").SetValStr("aaa");
    data("B")("int").SetValInt(8);
    data("CCc")("dde")("ffe")("str").SetValStr("aaa");
    data("CCc")("dde")("ffe")(32).SetValInt(77);
    data("CCc")(100).SetValInt(77);
    const char *expected_output = R"V0G0N({
  "A": {
    "str": "aaa",
  },
  "B": {
    "int": 8,
  },
  "CCc": {
    "100": 77,
    "dde": {
      "ffe": {
        "32": 77,
        "str": "aaa",
      },
    },
  },
},
)V0G0N";
    EXPECT_EQ(data.GetJsonStr(), expected_output);
    EXPECT_EQ(data.GetNumChildren(), 3);
    EXPECT_EQ(data("A").GetNumChildren(), 1);
    EXPECT_EQ(data("CCc").GetNumChildren(), 2);
    EXPECT_EQ(data("CCc")("dde").GetNumChildren(), 1);
    EXPECT_EQ(data("CCc")("dde")("ffe").GetNumChildren(), 2);
    EXPECT_EQ(data("CCc")("dde")("ffe")("32").GetNumChildren(), 0);
}

TEST(BaseQuery, TestValReset) {
    query_ns::DataQueryObj data;
    data("test_before_clear")("constant_str").SetValStr("aaa");
    data("test_before_clear")("constant_int").SetValInt(77);
    data.ClearData();

    data("test_after_clear")("constant_str").SetValStr("bbb");
    data("test_after_clear")("constant_int").SetValInt(88);

    const char *expected_output = R"V0G0N({
  "test_after_clear": {
    "constant_int": 88,
    "constant_str": "bbb",
  },
},
)V0G0N";
    EXPECT_EQ(data.GetJsonStr(), expected_output);
}

TEST(BaseQuery, TestInvalidGetSetValues) {
    query_ns::DataQueryObj data;
    data("test")("constant_str").SetValStr("aaa");
    // Cannot get an int value from a node of type kLEAF_STR.
    EXPECT_THROW(data("test")("constant_str").GetValInt(), std::runtime_error);
    EXPECT_EQ(data("test")("constant_str").GetValStr(), "aaa");
    // Cannot create a child for a kLEAF_STR.
    EXPECT_THROW(data("test")("constant_str")("something").SetValStr("smh"), std::runtime_error);
    // Cannot assign an int value to a node of type kLEAF_STR.
    EXPECT_THROW(data("test")("constant_str").SetValInt(30), std::runtime_error);
    // Cannot assign an int value to a node of type kNON_LEAF.
    EXPECT_THROW(data("test").SetValInt(30), std::runtime_error);
    // Cannot assign a str value to a node of type kNON_LEAF.
    EXPECT_THROW(data("test").SetValStr("30"), std::runtime_error);

    EXPECT_NO_THROW(data("test")("constant_int").SetValInt(20));
    EXPECT_EQ(data("test")("constant_int").GetValInt(), 20);
    // Cannot get an str value from a node of type kLEAF_INT.
    EXPECT_THROW(data("test")("constant_int").GetValStr(), std::runtime_error);
    // Cannot assign a str value to a node of type kLEAF_INT.
    EXPECT_THROW(data("test")("constant_int").SetValStr("30"), std::runtime_error);
    // Cannot create a child for a kLEAF_INT.
    EXPECT_THROW(data("test")("constant_int")("something").SetValStr("smh"), std::runtime_error);
}
