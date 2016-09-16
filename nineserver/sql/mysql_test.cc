#include "base/testing.h"
#include "nineserver/json.h"
#include "nineserver/sql/mysql.h"

using testing::ElementsAre;

class MySQLTest : public testing::Test {
 public:
  MySQLTest() {
    result_.num_rows = 2;
    result_.num_fields = 3;
    result_.data.reset(new ArenaStringPiece[9]);
    result_.data[0] = "field1"_a;
    result_.data[1] = "field2"_a;
    result_.data[2] = "field3"_a;
    result_.data[3] = "value1"_a;
    result_.data[4] = "value2"_a;
    result_.data[5] = "value3"_a;
    result_.data[6] = "0"_a;
    result_.data[7] = ""_a;
    result_.data[8] = QueryResult::Null();
  }

  QueryResult result_;
};

TEST_F(MySQLTest, QueryResult) {
  EXPECT_EQ("field1", result_.Field(0));
  EXPECT_EQ("field2", result_.Field(1));
  EXPECT_EQ("field3", result_.Field(2));
  EXPECT_EQ("value1", result_.Get(0, 0));
  EXPECT_EQ("value2", result_.Get(0, 1));
  EXPECT_EQ("value3", result_.Get(0, 2));
  EXPECT_EQ("0", result_.Get(1, 0));
  EXPECT_EQ("", result_.Get(1, 1));
  EXPECT_EQ("", result_.Get(1, 2));
  EXPECT_FALSE(QueryResult::IsNull(result_.Get(0, 0)));
  EXPECT_FALSE(QueryResult::IsNull(result_.Get(0, 1)));
  EXPECT_FALSE(QueryResult::IsNull(result_.Get(0, 2)));
  EXPECT_FALSE(QueryResult::IsNull(result_.Get(1, 0)));
  EXPECT_FALSE(QueryResult::IsNull(result_.Get(1, 1)));
  EXPECT_TRUE(QueryResult::IsNull(result_.Get(1, 2)));

  EXPECT_EQ(2, result_.size());
  EXPECT_EQ(3, result_[0].size());
  EXPECT_EQ("value1", result_[0][0]);
  EXPECT_TRUE(QueryResult::IsNull(result_[1][2]));
}

TEST_F(MySQLTest, QueryResult_ToJson) {
  EXPECT_EQ(
      R"([["field1","field2","field3"],)"
      R"(["value1","value2","value3"],)"
      R"(["0","",null]])",
      JsonEncode(result_));
}

TEST_F(MySQLTest, QueryResult_ForEach) {
  vector<vector<ArenaStringPiece>> table;
  for (const auto& result_row : result_) {
    vector<ArenaStringPiece> row;
    for (const auto& result_cell : result_row) {
      row.push_back(result_cell);
    }
    table.push_back(std::move(row));
  }
  EXPECT_THAT(table,
              ElementsAre(
                  ElementsAre("value1"_a, "value2"_a, "value3"_a),
                  ElementsAre("0"_a, ""_a, ""_a)));
}

TEST_F(MySQLTest, ToSqlTimestamp) {
  setenv("TZ", "Asia/Tokyo", 1);
  tzset();
  EXPECT_EQ("2016-01-02 03:04:05", ToSqlTimestamp(1451671445));
  EXPECT_EQ("2016-11-22 01:23:45", ToSqlTimestamp(1479745425));
  EXPECT_EQ("1970-01-01 09:00:00", ToSqlTimestamp(0));
  EXPECT_EQ("0000-00-00 00:00:00", ToSqlTimestamp(-1));
}

TEST_F(MySQLTest, FromSqlTimestamp) {
  setenv("TZ", "Asia/Tokyo", 1);
  tzset();
  EXPECT_EQ(1451671445, FromSqlTimestamp("2016-01-02 03:04:05"));
  EXPECT_EQ(1479745425, FromSqlTimestamp("2016-11-22 01:23:45"));
  EXPECT_EQ(0, FromSqlTimestamp("1970-01-01 09:00:00"));
  EXPECT_EQ(-1, FromSqlTimestamp("0000-00-00 00:00:00"));

  // Invalid format.
  EXPECT_EQ(-1, FromSqlTimestamp("2016-01-02"));
  EXPECT_EQ(-1, FromSqlTimestamp("2016-01-02 03:04"));
  EXPECT_EQ(-1, FromSqlTimestamp("2016/01/02 03:04:05"));
}
