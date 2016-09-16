#include "nineserver/handler/logging.h"

#include <map>
#include <mutex>
#include <string>
#include <tuple>

#include "base/base.h"

class LoggingStorage {
 public:
  static LoggingStorage* Get() {
    static LoggingStorage* logging_storage = new LoggingStorage();
    return logging_storage;
  }

  void Record(StringPiece path, StringPiece key, int64 value) {
    DVLOG(10) << "Logging " << path << ":" << key << "=" << value;
    std::lock_guard<std::mutex> data_guard(data_mutex_);
    stats_[path.ToString()][key.ToString()] += value;
  }

  map<string, map<string, int64>> GetStats() {
    std::lock_guard<std::mutex> data_guard(data_mutex_);
    return stats_;
  }

  void RegisterField(
      StringPiece key, StringPiece name, StringPiece unit, int divisor) {
    std::lock_guard<std::mutex> data_guard(data_mutex_);
    fields_.push_back({
        .key = key.ToString(),
        .name = name.ToString(),
        .unit = unit.ToString(),
        .divisor = divisor});
  }

  vector<Logging::Field> GetFields() {
    std::lock_guard<std::mutex> data_guard(data_mutex_);
    return fields_;
  }

 private:
  std::mutex data_mutex_;
  map<string, map<string, int64>> stats_;
  vector<Logging::Field> fields_;
};

void Logging::Record(StringPiece path, StringPiece key, int64 value) {
  LoggingStorage::Get()->Record(path, key, value);
}

map<string, map<string, int64>> Logging::GetStats() {
  return LoggingStorage::Get()->GetStats();
}

void Logging::RegisterField(
    StringPiece key, StringPiece name, StringPiece unit, int divisor) {
  LoggingStorage::Get()->RegisterField(key, name, unit, divisor);
}

vector<Logging::Field> Logging::GetFields() {
  return LoggingStorage::Get()->GetFields();
}
