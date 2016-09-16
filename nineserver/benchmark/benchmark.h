#include <functional>

#include "base/base.h"

class BenchmarkState {
 public:
  BenchmarkState(const string& name) : name_(name) {}

  bool KeepRunning();

 private:
  void PrintStats();
  static double GetTime();

  string name_;
  double start_time_;
  int64 iteration_ = 0;
  int64 iteration_to_check_ = 1;
  vector<pair<int64, double>> results_;
};

void Benchmark(const string& name, std::function<void()> target);
