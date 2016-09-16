#pragma once
#include <time.h>
#include <string>
#include <map>
#include <unordered_map>

#include "base/base.h"
#include "nineserver/arena/arena.h"
#include "nineserver/common.h"
#include "nineserver/io/generic_io.h"
#include "nineserver/json.h"

// Container for a small number of key-value pairs.  This accepts multiple keys.
class HttpParameters {
 public:
  typedef vector<pair<StringPiece, StringPiece>> Parameters;

  HttpParameters() {}

  // Adds key=value even if duplicated.
  void Add(ArenaStringPiece key, ArenaStringPiece value);
  // Sets key to value.  Removes existings.
  void Set(ArenaStringPiece key, ArenaStringPiece value);
  // Removes all values for key.
  void Remove(StringPiece key);
  // Gets a value with default_value.
  ArenaStringPiece Get(
      StringPiece key,
      ArenaStringPiece default_value = ArenaStringPiece()) const;
  // Returns all values.
  const vector<pair<StringPiece, StringPiece>>& GetValues() const;
  // Removes all values.
  void Clear();
  // Returns true iff empty.
  bool IsEmpty() const { return parameters_.empty(); }
  // Sorts values.
  void Sort();
  bool IsSorted() const;
  // Returns JSON object.
  Json ToJson() const;

  Parameters::iterator begin() { return parameters_.begin(); }
  Parameters::iterator end() { return parameters_.end(); }
  Parameters::const_iterator begin() const { return parameters_.begin(); }
  Parameters::const_iterator end() const { return parameters_.end(); }


 private:
  bool is_sorted_ = true;
  vector<pair<StringPiece, StringPiece>> parameters_;

  DISALLOW_COPY_AND_ASSIGN(HttpParameters);
};
