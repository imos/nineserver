#include "base/testing.h"
#include "nineserver/benchmark/benchmark.h"
#include "nineserver/http/stream/string_stream.h"

class StringStreamTest : public testing::Test {
 public:
  StringStreamTest() {}

  StringPiece GetUri() {
    return stream_.GetRequest().GetEnvironments().Get("REQUEST_URI");
  }

  StringStream stream_;
};

TEST_F(StringStreamTest, Get) {
  Benchmark("NoKeepAlive", [this]() {
    stream_.SetInput(
        "GET /uri HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"_a);
    stream_.Init();
    ASSERT_TRUE(stream_.Start());
    ASSERT_EQ("/uri", GetUri());
    ASSERT_FALSE(stream_.Start());
    stream_.Destroy();
  });
  Benchmark("KeepAliveAndCut", [this]() {
    stream_.SetInput(
        "GET /uri HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"_a);
    stream_.Init();
    stream_.SetKeepAlive();
    ASSERT_TRUE(stream_.Start());
    ASSERT_EQ("/uri", GetUri());
    ASSERT_FALSE(stream_.Start());
    stream_.Destroy();
  });
  Benchmark("KeepAlive", [this]() {
    stream_.SetInput(
        "GET /first HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "GET /second HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "GET /third HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"_a);
    stream_.Init();
    stream_.SetKeepAlive();
    ASSERT_TRUE(stream_.Start());
    ASSERT_EQ("/first", GetUri());
    ASSERT_TRUE(stream_.Start());
    ASSERT_EQ("/second", GetUri());
    ASSERT_TRUE(stream_.Start());
    ASSERT_EQ("/third", GetUri());
    ASSERT_FALSE(stream_.Start());
    stream_.Destroy();
  });
}
