#include "nineserver/arena/arena.h"

#include "base/testing.h"

TEST(ArenaTest, NewString) {
  Arena arena;
  string* value = arena.New<string>("foo");
  EXPECT_EQ("foo", *value);
}
