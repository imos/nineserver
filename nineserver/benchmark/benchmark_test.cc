#include <mutex>
#include <thread>

#include "base/testing.h"
#include "nineserver/benchmark/benchmark.h"

TEST(BenchmarkTest, DoNothing) {
  Benchmark("DoNothing", []() {});
}

TEST(BenchmarkTest, Increment) {
  int a;
  Benchmark("Increment", [&a]() {
    a++;
  });
}

TEST(BenchmarkTest, AllocateString) {
  Benchmark("AllocateString", []() {
    string s;
    s = "foobar";
  });
}

TEST(BenchmarkTest, Nanosleep) {
  struct timespec req, rem;
  req.tv_sec = 0;
  req.tv_nsec = 1;
  Benchmark("Nanosleep", [&req, &rem]() {
    nanosleep(&req, &rem);
  });
}

TEST(BenchmarkTest, ThreadYield) {
  Benchmark("Yield", []() {
    std::this_thread::yield();
  });
}

TEST(BenchmarkTest, Mutex) {
  std::mutex m;
  Benchmark("LockAndUnlock", [&m]() {
    m.lock();
    m.unlock();
  });
  Benchmark("TryLock", [&m]() {
    m.try_lock();
  });
}
