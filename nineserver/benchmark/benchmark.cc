#include "nineserver/benchmark/benchmark.h"

#include <sys/time.h>

DEFINE_double(benchmark_duration, 0, "Duration to benchmark.");

bool BenchmarkState::KeepRunning() {
  iteration_++;
  if (iteration_to_check_ == iteration_) {
    if (iteration_to_check_ == 1) {
      iteration_to_check_ = 2;
      start_time_ = GetTime();
      results_.reserve(100);
      results_.emplace_back(iteration_, start_time_);
      return true;
    }
    double current_time = GetTime();
    if (current_time - start_time_ < 0.1 &&
        current_time - start_time_ < FLAGS_benchmark_duration) {
      if (current_time - start_time_ < 0.01) {
        iteration_to_check_ = iteration_ * 8;
      } else {
        iteration_to_check_ = iteration_ * 2;
      }
    } else {
      results_.emplace_back(iteration_, current_time);
      iteration_to_check_ =
          iteration_ + round(iteration_ / (current_time - start_time_) / 10);
      if (iteration_to_check_ <= iteration_) {
        iteration_to_check_ = iteration_ + 1;
      }
      if (current_time - start_time_ > FLAGS_benchmark_duration) {
        PrintStats();
        return false;
      }
    }
  }
  return true;
}

void BenchmarkState::PrintStats() {
  double best_qps = 0;
  for (int i = 1; i < results_.size(); i++) {
    best_qps = max(best_qps, (results_[i].first - results_[i - 1].first) /
                             (results_[i].second - results_[i - 1].second));
  }
  printf("%32s: % 16.2f ns\n", name_.c_str(), 1e9 / best_qps);
}

double BenchmarkState::GetTime() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}

void Benchmark(const string& name, std::function<void()> target) {
  BenchmarkState state(name);
  while (state.KeepRunning()) {
    target();
  }
}
