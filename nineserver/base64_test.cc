#include "nineserver/base64.h"

#include "base/testing.h"

TEST(Base64Test, Encode) {
  EXPECT_EQ("", Base64::Encode(""));
  EXPECT_EQ("YQ==", Base64::Encode("a"));
  EXPECT_EQ("YWI=", Base64::Encode("ab"));
  EXPECT_EQ("YWJj", Base64::Encode("abc"));
  EXPECT_EQ("AA==", Base64::Encode("\0"_a));
  EXPECT_EQ("AAA=", Base64::Encode("\0\0"_a));
  EXPECT_EQ("AAAA", Base64::Encode("\0\0\0"_a));
  EXPECT_EQ("YWJjZA==", Base64::Encode("abcd"));
  EXPECT_EQ("YWI+", Base64::Encode("ab>"));
  EXPECT_EQ("YWI/", Base64::Encode("ab?"));
}

TEST(Base64Test, UrlEncode) {
  EXPECT_EQ("", Base64::UrlEncode(""));
  EXPECT_EQ("YQ", Base64::UrlEncode("a"));
  EXPECT_EQ("YWI", Base64::UrlEncode("ab"));
  EXPECT_EQ("YWJj", Base64::UrlEncode("abc"));
  EXPECT_EQ("YWJjZA", Base64::UrlEncode("abcd"));
  EXPECT_EQ("YWI-", Base64::UrlEncode("ab>"));
  EXPECT_EQ("YWI_", Base64::UrlEncode("ab?"));
}

TEST(Base64Test, UrlDecode) {
  EXPECT_EQ("", Base64::Decode(""));
  EXPECT_EQ("a", Base64::Decode("YQ=="));
  EXPECT_EQ("ab", Base64::Decode("YWI="));
  EXPECT_EQ("abc", Base64::Decode("YWJj"));
  EXPECT_EQ("abcd", Base64::Decode("YWJjZA=="));

  EXPECT_EQ("a", Base64::Decode("YQ"));
  EXPECT_EQ("ab", Base64::Decode("YWI"));
  EXPECT_EQ("abcd", Base64::Decode("YWJjZA"));

  EXPECT_EQ("\0"_a, Base64::Decode("AA=="));
  EXPECT_EQ("\0\0"_a, Base64::Decode("AAA="));
  EXPECT_EQ("\0\0\0"_a, Base64::Decode("AAAA"));

  EXPECT_EQ("ab>", Base64::Decode("YWI+"));
  EXPECT_EQ("ab?", Base64::Decode("YWI/"));
  EXPECT_EQ("ab>", Base64::Decode("YWI-"));
  EXPECT_EQ("ab?", Base64::Decode("YWI_"));
}
