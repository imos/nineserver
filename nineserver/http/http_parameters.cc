#include "nineserver/http/http_parameters.h"

DEFINE_int32(http_parameters_binary_search_threshold, 20,
             "Minimum number of elements to binary-search.");

void HttpParameters::Add(ArenaStringPiece key, ArenaStringPiece value) {
  DCHECK(!key.empty() || !value.empty());
  if (is_sorted_ && !parameters_.empty() && parameters_.back().first > key) {
    is_sorted_ = false;
  }
  parameters_.emplace_back(key, value);
}

void HttpParameters::Set(ArenaStringPiece key, ArenaStringPiece value) {
  DCHECK(!key.empty() || !value.empty());
  bool found = false;
  for (auto& key_and_value : parameters_) {
    if (key_and_value.first == key) {
      if (found) {
        key_and_value.first = ""_a;
        key_and_value.second = ""_a;
        is_sorted_ = false;
      } else {
        key_and_value.second = value;
        found = true;
      }
    }
  }
  if (!found) {
    Add(key, value);
  }
}

void HttpParameters::Remove(StringPiece key) {
  for (auto& key_and_value : parameters_) {
    if (key_and_value.first == key) {
      key_and_value.first = "";
      key_and_value.second = "";
      is_sorted_ = false;
    }
  }
}

ArenaStringPiece HttpParameters::Get(
    StringPiece key, ArenaStringPiece default_value) const {
  if (is_sorted_ &&
      FLAGS_http_parameters_binary_search_threshold < parameters_.size()) {
    auto lower = key.empty() ? parameters_.begin() : std::lower_bound(
        parameters_.begin(), parameters_.end(),
        pair<StringPiece, StringPiece>(key, ""_a));
    if (lower != parameters_.end() && lower->first == key) {
      return ArenaStringPiece(lower->second);
    }
    return default_value;
  }
  for (const auto& key_and_value : parameters_) {
    if (key_and_value.first == key) {
      if (key.empty() && key_and_value.second.empty()) continue;
      return ArenaStringPiece(key_and_value.second);
    }
  }
  return default_value;
}

void HttpParameters::Clear() {
  InitializeVectorWithCapacity(&parameters_, 0, 64);
  is_sorted_ = true;
}

void HttpParameters::Sort() {
  if (is_sorted_) {
    return;
  }
  std::stable_sort(
      parameters_.begin(), parameters_.end(),
      [](const pair<StringPiece, StringPiece>& lhs,
         const pair<StringPiece, StringPiece>& rhs) {
        if (lhs.first.empty() && lhs.second.empty()) { return false; }
        if (rhs.first.empty() && rhs.second.empty()) { return true; }
        return lhs.first < rhs.first;
      });
  while (parameters_.back().first.empty() &&
         parameters_.back().second.empty()) {
    parameters_.resize(parameters_.size() - 1);
  }
  is_sorted_ = true;
}

bool HttpParameters::IsSorted() const { return is_sorted_; }

Json HttpParameters::ToJson() const {
  map<string, vector<string>> parameters;
  for (const auto& key_and_value : parameters_) {
    parameters[key_and_value.first.ToString()].push_back(
        key_and_value.second.ToString());
  }
  return parameters;
}
