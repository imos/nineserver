#include "base/testing.h"
#include "nineserver/json.h"

struct TestObject {
  string string_value = "";
  int int_value = 0;

  Json ToJson() const {
    return Json::object({
        {"string_value", string_value},
        {"int_value", int_value}});
  }

  // string FromJson(const Json& json) {
  //   if (!json.is_object()) {
  //     return "object is expected";
  //   }
  //   auto string_value_ptr = json.find("string_value");
  //   if (string_value_ptr == json.end()) {
  //     string_value = "";
  //   } else if (string_value_ptr->is_string()) {
  //     string_value = string_value_ptr->get<string>();
  //   } else {
  //     return "string_value expects a string";
  //   }
  //   auto int_value_ptr = json.find("int_value");
  //   if (int_value_ptr == json.end()) {
  //     int_value = 0;
  //   } else if (int_value_ptr->is_number()) {
  //     int_value = int_value_ptr->get<int>();
  //   } else {
  //     return "int_value expects a number";
  //   }
  //   return "";
  // }
};

TEST(JsonTest, JsonEncode) {
  EXPECT_EQ("[3,2,1]", JsonEncode(vector<int>({3, 2 ,1})));
  EXPECT_EQ("[0.5,1,1.5]", JsonEncode(vector<double>({0.5, 1, 1.5})));
  EXPECT_EQ("[1,2,3]", JsonEncode(set<int>({3, 2, 1})));
  EXPECT_EQ(R"({"key":"value"})",
            JsonEncode(map<string, string>({{"key", "value"}})));
  EXPECT_EQ(R"({"key1":"value1","key2":"value2"})",
            JsonEncode(
                map<string, string>({{"key1", "value1"}, {"key2", "value2"}})));
  TestObject test_object;
  test_object.string_value = "foo";
  test_object.int_value = 12345;
  EXPECT_EQ(R"({"int_value":12345,"string_value":"foo"})",
            JsonEncode(test_object));
}

/*
TEST(JsonTest, JsonDecode) {
  {
    vector<int> value;
    EXPECT_EQ("", JsonDecode("[1,2,3]", &value));
    EXPECT_THAT(value, testing::ElementsAre(1, 2, 3));
  }

  {
    vector<int> value;
    EXPECT_EQ("", JsonDecode("[1,2,3]", &value));
    EXPECT_THAT(value, testing::ElementsAre(1, 2, 3));
  }

  {
    TestObject value;
    EXPECT_EQ(
        "", JsonDecode(R"({"int_value":12345,"string_value":"foo"})", &value));
    EXPECT_EQ(12345, value.int_value);
    EXPECT_EQ("foo", value.string_value);
  }

  {
    vector<int> value;
    EXPECT_NE("", JsonDecode("[1,2,\"3\"]", &value));
  }

  {
    TestObject value;
    EXPECT_NE("", JsonDecode(R"({"int_value":"foo"})", &value));
  }
}
*/