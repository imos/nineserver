#pragma once
#include <map>

#include "base/base.h"

class Logging {
 public:
  struct Field {
    string key;
    string name;
    string unit;
    int divisor;
  };

  static void Record(StringPiece path, StringPiece key, int64 value);
  static map<string, map<string, int64>> GetStats();

  static void RegisterField(
      StringPiece key, StringPiece name,
      StringPiece unit = StringPiece(), int divisor = 1);
  static vector<Field> GetFields();
};
