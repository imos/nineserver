#include "base/testing.h"
#include "nineserver/http/util.h"

TEST(HttpTest, ToHttpDateTime) {
  EXPECT_EQ("Sat, 02 Jan 2016 03:04:05 GMT",
            ToHttpDateTime(static_cast<time_t>(1451703845)));
}

TEST(HttpTest, FromHttpDateTime) {
  EXPECT_EQ(1451703845, FromHttpDateTime("Sat, 02 Jan 2016 03:04:05 UTC"));
  EXPECT_EQ(1451671445, FromHttpDateTime("Sat, 02 Jan 2016 03:04:05 JST"));
  EXPECT_EQ(1451732645, FromHttpDateTime("Sat, 02 Jan 2016 03:04:05 PDT"));
  EXPECT_EQ(1451721845, FromHttpDateTime("Sat, 02 Jan 2016 03:04:05 EDT"));
  EXPECT_EQ(0, FromHttpDateTime("Sat, 02 Jan 2016 03:04:05 XXX"));
}

TEST(HttpTest, SplitHeader) {
  {
    StringPiece header = R"(attachment; filename="hogehoge.txt")";
    StringPiece key, value, trailing;
    SplitHeader(header, &key, &value, &header);
    EXPECT_EQ("", key);
    EXPECT_EQ("attachment", value);
    EXPECT_EQ(R"( filename="hogehoge.txt")", header);
  }

  {
    StringPiece header = R"(filename="hogehoge.txt"; attachment)";
    StringPiece key, value, trailing;
    SplitHeader(header, &key, &value, &header);
    EXPECT_EQ("filename", key);
    EXPECT_EQ("hogehoge.txt", value);
    EXPECT_EQ(R"( attachment)", header);
  }

  {
    StringPiece header = R"(attachment)";
    StringPiece key, value, trailing;
    SplitHeader(header, &key, &value, &header);
    EXPECT_EQ("", key);
    EXPECT_EQ("attachment", value);
    EXPECT_EQ("", header);
  }

  {
    StringPiece header = R"(filename="hogehoge.txt")";
    StringPiece key, value, trailing;
    SplitHeader(header, &key, &value, &header);
    EXPECT_EQ("filename", key);
    EXPECT_EQ("hogehoge.txt", value);
    EXPECT_EQ("", header);
  }

  {
    StringPiece header = R"(filename="hoge;hoge.txt")";
    StringPiece key, value, trailing;
    SplitHeader(header, &key, &value, &header);
    EXPECT_EQ("filename", key);
    EXPECT_EQ("hoge;hoge.txt", value);
    EXPECT_EQ("", header);
  }
}
