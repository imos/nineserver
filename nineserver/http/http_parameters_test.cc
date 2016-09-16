#include <sys/time.h>

#include "base/testing.h"
#include "nineserver/benchmark/benchmark.h"
#include "nineserver/http/http_parameters.h"

DECLARE_int32(http_parameters_binary_search_threshold);

TEST(HttpParametersBenchmark, HttpParameters) {
  HttpParameters parameters;
  Benchmark("Add10Parameters", [&]() {
    parameters.Clear();
    parameters.Add("Host"_a, "1"_a);
    parameters.Add("Accept"_a, "2"_a);
    parameters.Add("Accept-Encoding"_a, "3"_a);
    parameters.Add("Accept-Language"_a, "4"_a);
    parameters.Add("Cache-Control"_a, "5"_a);
    parameters.Add("Connection"_a, "6"_a);
    parameters.Add("Cookie"_a, "7"_a);
    parameters.Add("If-None-Match"_a, "8"_a);
    parameters.Add("Upgrade-Insecure-Requests"_a, "9"_a);
    parameters.Add("User-Agent"_a, "10"_a);
  });
  ASSERT_FALSE(parameters.IsSorted());
  Benchmark("GetThirdParameter", [&]() {
    ASSERT_EQ("3"_a, parameters.Get("Accept-Encoding"_a));
  });
  Benchmark("GetLastParameter", [&]() {
    ASSERT_EQ("10"_a, parameters.Get("User-Agent"_a));
  });
}

TEST(HttpParametersBenchmark, SortedHttpParameters) {
  FLAGS_http_parameters_binary_search_threshold = 0;
  HttpParameters parameters;
  Benchmark("Add10Parameters", [&]() {
    parameters.Clear();
    parameters.Add("Host"_a, "1"_a);
    parameters.Add("Accept"_a, "2"_a);
    parameters.Add("Accept-Encoding"_a, "3"_a);
    parameters.Add("Accept-Language"_a, "4"_a);
    parameters.Add("Cache-Control"_a, "5"_a);
    parameters.Add("Connection"_a, "6"_a);
    parameters.Add("Cookie"_a, "7"_a);
    parameters.Add("If-None-Match"_a, "8"_a);
    parameters.Add("Upgrade-Insecure-Requests"_a, "9"_a);
    parameters.Add("User-Agent"_a, "10"_a);
    parameters.Sort();
  });
  ASSERT_TRUE(parameters.IsSorted());
  Benchmark("GetThirdParameter", [&]() {
    ASSERT_EQ("3"_a, parameters.Get("Accept-Encoding"_a));
  });
  Benchmark("GetLastParameter", [&]() {
    ASSERT_EQ("10"_a, parameters.Get("User-Agent"_a));
  });
}

TEST(HttpParametersBenchmark, Map) {
  map<StringPiece, StringPiece> parameters;
  Benchmark("Add10Parameters", [&]() {
    parameters.clear();
    parameters.emplace("Host"_a, "1"_a);
    parameters.emplace("Accept"_a, "2"_a);
    parameters.emplace("Accept-Encoding"_a, "3"_a);
    parameters.emplace("Accept-Language"_a, "4"_a);
    parameters.emplace("Cache-Control"_a, "5"_a);
    parameters.emplace("Connection"_a, "6"_a);
    parameters.emplace("Cookie"_a, "7"_a);
    parameters.emplace("If-None-Match"_a, "8"_a);
    parameters.emplace("Upgrade-Insecure-Requests"_a, "9"_a);
    parameters.emplace("User-Agent"_a, "10"_a);
  });
  Benchmark("GetThirdParameter", [&]() {
    ASSERT_EQ("3"_a, parameters.find("Accept-Encoding"_a)->second);
  });
  Benchmark("GetLastParameter", [&]() {
    ASSERT_EQ("10"_a, parameters.find("User-Agent"_a)->second);
  });
}

TEST(HttpParametersBenchmark, Vector) {
  vector<pair<StringPiece, StringPiece>> parameters;
  Benchmark("Add10Parameters", [&]() {
    parameters.clear();
    parameters.emplace_back("Host"_a, "1"_a);
    parameters.emplace_back("Accept"_a, "2"_a);
    parameters.emplace_back("Accept-Encoding"_a, "3"_a);
    parameters.emplace_back("Accept-Language"_a, "4"_a);
    parameters.emplace_back("Cache-Control"_a, "5"_a);
    parameters.emplace_back("Connection"_a, "6"_a);
    parameters.emplace_back("Cookie"_a, "7"_a);
    parameters.emplace_back("If-None-Match"_a, "8"_a);
    parameters.emplace_back("Upgrade-Insecure-Requests"_a, "9"_a);
    parameters.emplace_back("User-Agent"_a, "10"_a);
    sort(parameters.begin(), parameters.end());
  });
  Benchmark("GetThirdParameter", [&]() {
    ASSERT_EQ(
        "3"_a,
        std::lower_bound(
            parameters.begin(), parameters.end(),
            pair<StringPiece, StringPiece>("Accept-Encoding"_a, ""_a))->second);
  });
  Benchmark("GetLastParameter", [&]() {
    ASSERT_EQ(
        "10"_a,
        std::lower_bound(
            parameters.begin(), parameters.end(),
            pair<StringPiece, StringPiece>("User-Agent"_a, ""_a))->second);
  });
}
