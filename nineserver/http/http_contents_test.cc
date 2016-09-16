#include <sys/time.h>

#include "base/testing.h"
#include "nineserver/benchmark/benchmark.h"
#include "nineserver/http/http_contents.h"
#include "nineserver/io/string_io.h"

TEST(HttpContentsTest, Read) {
  StringIO string_io;
  Arena arena;
  HttpContents http_contents;
  Benchmark("ParseRequest", [&]() {
    string_io.InitStringIO(
        "GET / HTTP/1.1\r\n"
        "User-Agent: foo\r\n"
        "Host: example.com\r\n"
        "Accept: */*\r\n"
        "\r\n"_a);
    http_contents.Init(&arena);
    ASSERT_TRUE(http_contents.Read(&string_io));
  });
  EXPECT_EQ("GET / HTTP/1.1", http_contents.GetHeader(""));
  EXPECT_EQ("example.com", http_contents.GetHeader("Host"));
  EXPECT_EQ("foo", http_contents.GetHeader("User-Agent"));
  http_contents.Init(&arena);
  EXPECT_EQ("", http_contents.GetHeader("Host"));
}

TEST(HttpContentsTest, ReadPostRequest) {
  StringIO string_io;
  Arena arena;
  HttpContents http_contents;
  Benchmark("ParseRequest", [&]() {
    string_io.InitStringIO(
        "POST / HTTP/1.1\r\n"
        "User-Agent: foo\r\n"
        "Host: example.com\r\n"
        "Accept: */*\r\n"
        "Content-Length: 7\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "\r\n"
        "foo=bar"_a);
    http_contents.Init(&arena);
    ASSERT_TRUE(http_contents.Read(&string_io));
  });
  EXPECT_EQ("POST / HTTP/1.1", http_contents.GetHeader(""));
  EXPECT_EQ("example.com", http_contents.GetHeader("Host"));
  EXPECT_EQ("7", http_contents.GetHeader("Content-Length"));
  EXPECT_EQ("foo=bar", http_contents.GetFirstContent());
  http_contents.Init(&arena);
  EXPECT_EQ("", http_contents.GetHeader("Host"));
  EXPECT_EQ("", http_contents.GetFirstContent());
}

TEST(HttpContentsTest, Write) {
  StringIO string_io;
  Arena arena;
  HttpContents http_contents;
  http_contents.Init(&arena);
  http_contents.SetHeader(""_a, "HTTP/1.1 200 OK"_a);
  http_contents.SetHeader("Content-Type"_a, "text/html; charset=UTF-8"_a);
  http_contents.Print("foo"_a);
  http_contents.Print("bar"_a);
  Benchmark("BuildResponse", [&]() {
    string_io.InitStringIO("");
    ASSERT_TRUE(http_contents.Write(&string_io));
    ASSERT_EQ(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: 6\r\n"
        "\r\n"
        "foobar"_a,
        string_io.output());
  });
}
