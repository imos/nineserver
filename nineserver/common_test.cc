#include "nineserver/common.h"

#include <unordered_set>

#include "base/testing.h"

TEST(CommonTest, CaseInsensitiveEqual) {
  EXPECT_TRUE(CaseInsensitiveEqual("abc", "abc"));
  EXPECT_TRUE(CaseInsensitiveEqual("abc", "Abc"));
  EXPECT_FALSE(CaseInsensitiveEqual("abc", ""));
  EXPECT_FALSE(CaseInsensitiveEqual("", "abc"));
  EXPECT_TRUE(CaseInsensitiveEqual("", ""));
  EXPECT_FALSE(CaseInsensitiveEqual("abc", "cde"));
}

TEST(CommonTest, CaseInsensitiveStringPiece) {
  std::unordered_set<StringPiece, CaseInsensitiveStringPiece,
                     CaseInsensitiveStringPiece> values;
  values.insert("abc");
  EXPECT_EQ(1, values.size());
  values.insert("cdf");
  EXPECT_EQ(2, values.size());
  values.insert("ABC");
  EXPECT_EQ(2, values.size());
}

TEST(CommonTest, SplitToStringPieces) {
  StringPiece output[3];
  EXPECT_TRUE(SplitToStringPieces(
      "foo, bar,, baz", ", ", &output[0], &output[1], &output[2]));
  EXPECT_EQ("foo", output[0]);
  EXPECT_EQ("bar,", output[1]);
  EXPECT_EQ("baz", output[2]);

  EXPECT_FALSE(SplitToStringPieces(
      "foo, bar", ", ", &output[0], &output[1], &output[2]));
}

TEST(CommonTest, ReverseSplitToStringPieces) {
  StringPiece output[3];
  EXPECT_TRUE(ReverseSplitToStringPieces(
      "hoge, foo, bar,, baz", ", ", &output[0], &output[1], &output[2]));
  EXPECT_EQ("hoge, foo", output[0]);
  EXPECT_EQ("bar,", output[1]);
  EXPECT_EQ("baz", output[2]);
}
