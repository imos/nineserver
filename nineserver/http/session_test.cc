#include "base/testing.h"
#include "nineserver/common.h"
#include "nineserver/http/session.h"

DECLARE_int32(num_sessions);

class SessionTest : public testing::Test {
 public:
  SessionTest() {
    FLAGS_num_sessions = 10000;
  }
};

struct TestData1 {
  TestData1() : value("") {}
  TestData1(const string& v) : value(v) {}

  string value;
};

struct TestData2 {
  TestData2() {}
  TestData2(const string& v) : value(v) {}

  string value;
};

TEST_F(SessionTest, SimpleSenario) {
  Session session;
  auto lock = session.Lock();
  ASSERT_EQ(nullptr, session.GetOrNull<TestData1>());
  EXPECT_EQ("", session.Get<TestData1>().value);
  session.Set(MakeUnique<TestData1>("test1a"));
  EXPECT_EQ("test1a", session.Get<TestData1>().value);
  {
    auto* test_data = session.Mutable<TestData1>();
    ASSERT_NE(nullptr, test_data);
    EXPECT_EQ("test1a", test_data->value);
    test_data->value = "test1b";
  }
  {
    auto* test_data = session.GetOrNull<TestData1>();
    ASSERT_NE(nullptr, test_data);
    EXPECT_EQ("test1b", test_data->value);
  }

  ASSERT_EQ(nullptr, session.GetOrNull<TestData2>());
  session.Set(MakeUnique<TestData2>("test2a"));
  {
    auto* test_data = session.GetOrNull<TestData1>();
    ASSERT_NE(nullptr, test_data);
    EXPECT_EQ("test1b", test_data->value);
  }
  {
    auto* test_data = session.GetOrNull<TestData2>();
    ASSERT_NE(nullptr, test_data);
    EXPECT_EQ("test2a", test_data->value);
  }
  {
    auto* test_data = session.Mutable<TestData2>();
    ASSERT_NE(nullptr, test_data);
    EXPECT_EQ("test2a", test_data->value);
    test_data->value = "test2b";
  }
  {
    auto* test_data = session.GetOrNull<TestData2>();
    ASSERT_NE(nullptr, test_data);
    EXPECT_EQ("test2b", test_data->value);
  }
  {
    auto* test_data = session.GetOrNull<TestData1>();
    ASSERT_NE(nullptr, test_data);
    EXPECT_EQ("test1b", test_data->value);
  }
}

TEST_F(SessionTest, Sessions) {
  FLAGS_num_sessions = 3;
  std::shared_ptr<Session> session = GetSession("example");
  session->Set(MakeUnique<string>("test1"));
  ASSERT_EQ("test1", GetSession("example")->Get<string>());
  for (int i = 0; i < 10; i++) {
    GetSession(StrCat(i));
  }
  // Empty sessions should be removed in advance.
  ASSERT_EQ("test1", GetSession("example")->Get<string>());

  for (int i = 0; i < 10; i++) {
    GetSession(StrCat(i))->Set(MakeUnique<string>("dummy"));
    RemoveSession(StrCat(i));
  }
  // The other sessions are removed, so this session should still be alive.
  ASSERT_EQ("test1", GetSession("example")->Get<string>());

  for (int i = 0; i < 10; i++) {
    GetSession(StrCat(i))->Set(MakeUnique<string>("dummy"));
  }
  // The session should be kicked out.
  ASSERT_EQ("", GetSession("example")->Get<string>());
}
