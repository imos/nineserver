#pragma once
#include <ctype.h>
#include <string.h>
#include <algorithm>

#include "base/base.h"

#ifndef DVLOG_IS_ON
#ifdef NDEBUG
#define DVLOG_IS_ON(level) false
#else
#define DVLOG_IS_ON(level) VLOG_IS_ON(level)
#endif
#endif

// Enables to prioritize functions.
template<unsigned priority>
struct overload_priority : overload_priority<priority - 1> {};
template<> struct overload_priority<0> {};

template<typename T>
void InitializeVectorWithCapacity(
    vector<T>* v, int64 minimum, int64 maximum = 0) {
  if (minimum > maximum) { maximum = minimum; }
  v->clear();
  if (v->capacity() > maximum) {
    v->shrink_to_fit();
    v->reserve(maximum);
  } else if (v->capacity() < minimum) {
    v->reserve(minimum);
  }
}

template<typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

bool CaseInsensitiveEqual(StringPiece l, StringPiece r);

struct StringPieceHash {
  std::size_t operator()(const StringPiece& key) const {
    return key.size();
  }
};

struct CaseInsensitiveStringPiece {
  std::size_t operator()(const StringPiece& x) const {
    if (x.empty()) { return 0; }
    std::size_t hash = 0;
    for (int i = 0; i < sizeof(hash) && i < x.size(); i++) {
      hash = (hash << 8) | static_cast<uint8>(tolower(x[i]));
    }
    return hash ^ x.size();
  }

  bool operator()(const StringPiece& l, const StringPiece& r) const
      { return CaseInsensitiveEqual(l, r); }
};

namespace internal {

inline size_t Length(char c) { return 1; }
inline size_t Length(StringPiece s) { return s.size(); }

}  // namespace

template<typename Delimiter>
inline bool SplitToStringPieces(
    StringPiece input, Delimiter delimiter,
    StringPiece* output1, StringPiece* output2) {
  auto position = input.find(delimiter);
  if (position == StringPiece::npos) {
    *output1 = input;
    return false;
  }
  if (output1 != nullptr) {
    output1->set(input.data(), position);
  }
  if (output2 != nullptr) {
    output2->set(input.data() + position + internal::Length(delimiter),
                 input.size() - position - internal::Length(delimiter));
  }
  return true;
}

template<typename Delimiter, typename... Args>
inline bool SplitToStringPieces(
    StringPiece input, Delimiter delimiter,
    StringPiece* output1, StringPiece* output2,
    StringPiece* output3, Args*... args) {
  StringPiece trailing;
  if (!SplitToStringPieces(input, delimiter, output1, &trailing)) {
    return false;
  }
  return SplitToStringPieces(trailing, delimiter, output2, output3, args...);
}

template<typename Delimiter>
inline bool ReverseSplitToStringPieces(
    StringPiece input, Delimiter delimiter,
    StringPiece* output1, StringPiece* output2) {
  auto position = input.rfind(delimiter);
  if (position == StringPiece::npos) {
    if (output2 != nullptr) {
      *output2 = input;
    }
    return false;
  }
  if (output1 != nullptr) {
    output1->set(input.data(), position);
  }
  if (output2 != nullptr) {
    output2->set(input.data() + position + internal::Length(delimiter),
                 input.size() - position - internal::Length(delimiter));
  }
  return true;
}

template<typename Delimiter, typename... Args>
inline bool ReverseSplitToStringPieces(
    StringPiece input, Delimiter delimiter, Args*... args,
    StringPiece* output3, StringPiece* output2, StringPiece* output1) {
  StringPiece leading;
  if (!ReverseSplitToStringPieces(input, delimiter, &leading, output1)) {
    return false;
  }
  return ReverseSplitToStringPieces(
      leading, delimiter, args..., output3, output2);
}

double GetThreadClock();
double GetCurrentTime();

int64 Rand();
