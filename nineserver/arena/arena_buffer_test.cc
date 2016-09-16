#include "nineserver/arena/arena_buffer.h"

#include "base/testing.h"
#include "nineserver/benchmark/benchmark.h"

DECLARE_int32(arena_buffer_size);

class ArenaBufferTest : public testing::Test {
 public:
  ArenaBufferTest() {
    FLAGS_arena_buffer_size = 128;
  }
};

TEST(ArenaBufferTest, SimpleScenario) {
  Benchmark("SimpleScenario", []() {
    ArenaBuffer arena_buffer;
    arena_buffer.append("foo");
    arena_buffer.append("bar");
    StringPiece piece1 = arena_buffer;
    arena_buffer.Init();
    arena_buffer.append("hoge");
    arena_buffer.append("piyo");
    StringPiece piece2 = arena_buffer;
    arena_buffer.Init();
    arena_buffer.push_back('1');
    arena_buffer.append("23");
    arena_buffer.resize(2);
    arena_buffer.append("456");
    StringPiece piece3 = arena_buffer;
    EXPECT_EQ("foobar", piece1);
    EXPECT_EQ("hogepiyo", piece2);
    EXPECT_EQ("12456", piece3);
    EXPECT_EQ('4', piece3[2]);

    arena_buffer.DeleteAll();
    arena_buffer.append("bar");
    arena_buffer.append("baz");
    // NOTE: First piece should be overwritten because of DeleteAll.
    EXPECT_EQ("barbaz", piece1);
  });
}

TEST(ArenaBufferTest, LargeString) {
  Benchmark("LargeString", []() {
    ArenaBuffer arena_buffer;
    for (int i = 0; i < 10000; i++) {
      arena_buffer.append("foo");
    }
    for (int i = 0; i < 10000; i++) {
      arena_buffer.append("bar");
    }
    StringPiece piece = arena_buffer;
    EXPECT_EQ(60000, arena_buffer.size());
    arena_buffer.Init();
    EXPECT_EQ(0, arena_buffer.size());
    EXPECT_EQ(60000, piece.size());
    EXPECT_EQ("foofoo", piece.substr(0, 6));
    EXPECT_EQ("barbar", piece.substr(piece.size() - 6));
  });
}
