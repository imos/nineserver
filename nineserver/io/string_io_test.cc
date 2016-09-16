#include "base/testing.h"
#include "nineserver/arena/arena.h"
#include "nineserver/arena/arena_buffer.h"
#include "nineserver/io/string_io.h"

TEST(StringIOTest, SimpleScenario) {
  StringIO string_io;
  ASSERT_TRUE(string_io.IsDestroyed());
  string_io.InitStringIO("foo\nbar\nbaz"_a);
  StringBuffer line;
  EXPECT_TRUE(string_io.ReadLine(&line));
  ASSERT_EQ("foo\n", line);
  EXPECT_TRUE(string_io.ReadLine(&line));
  ASSERT_EQ("foo\nbar\n", line);
  EXPECT_TRUE(string_io.ReadLine(&line));
  ASSERT_EQ("foo\nbar\nbaz", line);
  EXPECT_FALSE(string_io.ReadLine(&line));

  EXPECT_TRUE(string_io.Write("hoge"));
  ASSERT_EQ("hoge", string_io.output());
  EXPECT_TRUE(string_io.Write("piyo"));
  ASSERT_EQ("hogepiyo", string_io.output());
  string_io.Destroy();
  EXPECT_FALSE(string_io.Write("fuga"));
  ASSERT_EQ("hogepiyo", string_io.output());
}

TEST(StringIOTest, ReadN) {
  {
    StringIO string_io;
    string_io.InitStringIO("12345"_a);
    StringBuffer buffer;
    // Consume exactly.
    EXPECT_TRUE(string_io.ReadN(5, &buffer));
    ASSERT_EQ("12345", buffer);
    EXPECT_FALSE(string_io.ReadN(0, &buffer));
    EXPECT_FALSE(string_io.ReadN(1, &buffer));
  }

  {
    StringIO string_io;
    string_io.InitStringIO("12345"_a);
    StringBuffer buffer;
    // Reads partially.
    EXPECT_TRUE(string_io.ReadN(3, &buffer));
    ASSERT_EQ("123", buffer);
    // Input shortage.
    EXPECT_TRUE(string_io.ReadN(3, &buffer));
    ASSERT_EQ("12345", buffer);
    EXPECT_FALSE(string_io.ReadN(0, &buffer));
    EXPECT_FALSE(string_io.ReadN(1, &buffer));
  }
}
