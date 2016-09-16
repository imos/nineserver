#include "base/testing.h"
#include "nineserver/benchmark/benchmark.h"
#include "nineserver/handler/base_handler.h"
#include "nineserver/http/stream/string_stream.h"

DECLARE_bool(profile_header);

map<StringPiece, StringPiece> GetResourceData() {
  return map<StringPiece, StringPiece>();
}

class BaseHandlerTest : public testing::Test {
 public:
  BaseHandlerTest() {
    FLAGS_profile_header = false;
    handler_.SetStream(&stream_);
  }

  int RunHandler() {
    int count = 0;
    for (handler_.Init(); handler_.Start();) {
      count++;
      handler_.Process();
    }
    handler_.Destroy();
    return count;
  }

  void RegisterHandler(StringPiece prefix, const string& name, bool result) {
    handler_pool_.Register(prefix, [this, name, result](BaseHandler*) {
      handler_history_.push_back(name);
      return result;
    });
  }

  StringStream stream_;
  BaseHandler handler_;
  HandlerPool handler_pool_;
  vector<string> handler_history_;
};

bool ReplaceFilter(BaseHandler* handler) {
  ArenaStringPiece data = handler->MutableResponse()->GetContents();
  ArenaStringPiece former, latter;
  if (!SplitToArenaStringPieces(data, "foo", &former, &latter)) {
    return false;
  }
  handler->MutableResponse()->ClearContents();
  handler->MutableResponse()->Print(former);
  handler->MutableResponse()->Print("baz");
  handler->MutableResponse()->Print(latter);
  return false;
}
REGISTER_FILTER(ReplaceFilter, "/", ReplaceFilter);

TEST_F(BaseHandlerTest, SimpleScenario) {
  Benchmark("NullHandler", [this]() {
    stream_.SetInput(
        "GET /_/null HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: foo\r\n"
        "\r\n"_a);
    EXPECT_EQ(1, RunHandler());
    EXPECT_EQ(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Connection: close\r\n"
        "Content-Length: 3\r\n"
        "\r\n"
        "OK\n"_a,
        stream_.GetOutput());
  });
}

TEST_F(BaseHandlerTest, ReplaceFilter) {
  Benchmark("ReplaceFilter", [this]() {
    stream_.SetInput(
        "GET /_/var?key=query_parameters&foo=bar HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: foo\r\n"
        "\r\n"_a);
    EXPECT_EQ(1, RunHandler());
    EXPECT_EQ(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Connection: close\r\n"
        "Content-Length: 42\r\n"
        "\r\n"
        "{\"baz\":[\"bar\"],\"key\":[\"query_parameters\"]}"_a,
        stream_.GetOutput());
  });
}

TEST_F(BaseHandlerTest, HandlerPool_True) {
  RegisterHandler("/a", "/a", true);
  RegisterHandler("/aa", "/aa", true);
  RegisterHandler("/abc", "/abc1", true);
  RegisterHandler("/abc", "/abc2", true);
  RegisterHandler("/abcdef", "/abcdef", true);

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/abcdefghi"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_, testing::ElementsAre("/abcdef"));

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/abcdef"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_, testing::ElementsAre("/abcdef"));

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/abcde"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_, testing::ElementsAre("/abc2"));

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_, testing::ElementsAre());
}

TEST_F(BaseHandlerTest, HandlerPool_False) {
  RegisterHandler("/a", "/a", false);
  RegisterHandler("/aa", "/aa", false);
  RegisterHandler("/abc", "/abc1", false);
  RegisterHandler("/abc", "/abc2", false);
  RegisterHandler("/abcdef", "/abcdef", false);

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/abcdefghi"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_,
              testing::ElementsAre("/abcdef", "/abc2", "/abc1", "/a"));

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/abcdef"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_,
              testing::ElementsAre("/abcdef", "/abc2", "/abc1", "/a"));

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/abcde"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_,
              testing::ElementsAre("/abc2", "/abc1", "/a"));

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/abc"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_,
              testing::ElementsAre("/abc2", "/abc1", "/a"));

  handler_history_.clear();
  stream_.MutableRequest()->MutableEnvironments()->Set(
      "SCRIPT_NAME"_a, "/"_a);
  handler_pool_.Run(&handler_);
  EXPECT_THAT(handler_history_, testing::ElementsAre());
}
