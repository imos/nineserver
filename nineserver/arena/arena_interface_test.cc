#include "nineserver/arena/arena_interface.h"

#include "base/testing.h"
#include "nineserver/benchmark/benchmark.h"

DECLARE_int32(arena_buffer_size);

class ArenaImplForTest : public ArenaInterface {
 public:
  int64 GetUsage() { return arena_.NextBuffer()->usage(); }

 private:
  Arena* MutableArena() override { return &arena_; }

  Arena arena_;
};

class ArenaInterfaceTest : public testing::Test {
 public:
  ArenaInterfaceTest() {
    FLAGS_arena_buffer_size = 16 * 1024;
  }
};

TEST_F(ArenaInterfaceTest, ArenaStrCat) {
  {
    FLAGS_arena_buffer_size = 4;
    ArenaImplForTest arena_impl;
    int counter = 0;
    Benchmark("4", [&]() {
      ASSERT_EQ("12foo34"_a, arena_impl.ArenaStrCat(12, "foo"_a, 34));
      counter++;
    });
    EXPECT_EQ(7, (arena_impl.GetUsage() + 4 * 7) / (counter + 4));
  }
  {
    FLAGS_arena_buffer_size = 4096;
    ArenaImplForTest arena_impl;
    int counter = 0;
    Benchmark("4096", [&]() {
      ASSERT_EQ("12foo34"_a, arena_impl.ArenaStrCat(12, "foo"_a, 34));
      counter++;
    });
    EXPECT_EQ(7, arena_impl.GetUsage() / counter);
  }
}

TEST_F(ArenaInterfaceTest, ArenaStringPrintf) {
  {
    FLAGS_arena_buffer_size = 4;
    ArenaImplForTest arena_impl;
    int counter = 0;
    Benchmark("4", [&]() {
      ASSERT_EQ("1234567"_a, arena_impl.ArenaStringPrintf("%d", 1234567));
      counter++;
    });
    EXPECT_EQ(7, (arena_impl.GetUsage() + 4 * 7) / (counter + 4));
  }
  {
    FLAGS_arena_buffer_size = 4096;
    ArenaImplForTest arena_impl;
    int counter = 0;
    Benchmark("4096", [&]() {
      ASSERT_EQ("1234567"_a, arena_impl.ArenaStringPrintf("%d", 1234567));
      counter++;
    });
    EXPECT_EQ(7, arena_impl.GetUsage() / counter);
  }
}
