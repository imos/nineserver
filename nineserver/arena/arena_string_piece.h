#pragma once
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

#include "base/base.h"
#include "nineserver/common.h"

class ArenaStringPiece {
 public:
  ArenaStringPiece() {}
  ArenaStringPiece(const char* data, std::size_t length)
      : value_(data, length) {}
  ArenaStringPiece(const ArenaStringPiece& value) : value_(value.value_) {}
  ArenaStringPiece(ArenaStringPiece&& value) : value_(value.value_) {}
  // TODO(P2): Prepare a static function to convert for safety.
  explicit ArenaStringPiece(StringPiece value) : value_(value) {}
  ~ArenaStringPiece() noexcept {}

  inline ArenaStringPiece operator=(ArenaStringPiece value) {
    value_ = value.value_;
    return *this;
  }

  inline operator StringPiece() const { return value_; }
  inline StringPiece ToStringPiece() const { return value_; }
  inline string ToString() const { return value_.ToString(); }

  inline bool empty() const { return value_.empty(); }
  inline const char* data() const { return value_.data(); }
  inline int64 size() const { return value_.size(); }

  inline ArenaStringPiece substr(
      int64 position, int64 length = StringPiece::npos) const {
    return ArenaStringPiece(value_.substr(position, length));
  }
  inline void remove_suffix(int64 length) { value_.remove_suffix(length); }
  inline void remove_prefix(int64 length) { value_.remove_prefix(length); }

  inline bool operator!=(StringPiece rhs) const { return value_ != rhs; }
  inline bool operator==(StringPiece rhs) const { return value_ == rhs; }
  inline bool operator<(StringPiece rhs) const { return value_ < rhs; }
  inline bool operator>(StringPiece rhs) const { return value_ > rhs; }
  inline bool operator<=(StringPiece rhs) const { return value_ <= rhs; }
  inline bool operator>=(StringPiece rhs) const { return value_ >= rhs; }

  StringPiece* MutableStringPiece() { return &value_; }

 private:
  StringPiece value_;
};

ArenaStringPiece operator"" _a(const char* str, std::size_t length);

template<typename Delimiter>
inline bool SplitToArenaStringPieces(
    ArenaStringPiece input, Delimiter delimiter,
    ArenaStringPiece* output1, ArenaStringPiece* output2) {
  return SplitToStringPieces(
      input, delimiter,
      output1 == nullptr ? nullptr : output1->MutableStringPiece(),
      output2 == nullptr ? nullptr : output2->MutableStringPiece());
}

template<typename Delimiter, typename... Args>
inline bool SplitToArenaStringPieces(
    ArenaStringPiece input, Delimiter delimiter,
    ArenaStringPiece* output1, ArenaStringPiece* output2,
    ArenaStringPiece* output3, Args*... args) {
  ArenaStringPiece trailing;
  if (!SplitToArenaStringPieces(input, delimiter, output1, &trailing)) {
    return false;
  }
  return SplitToArenaStringPieces(
      trailing, delimiter, output2, output3, args...);
}

template<typename Delimiter>
inline bool ReverseSplitToArenaStringPieces(
    ArenaStringPiece input, Delimiter delimiter,
    ArenaStringPiece* output1, ArenaStringPiece* output2) {
  return ReverseSplitToStringPieces(
      input, delimiter,
      output1 == nullptr ? nullptr : output1->MutableStringPiece(),
      output2 == nullptr ? nullptr : output2->MutableStringPiece());
}

template<typename Delimiter, typename... Args>
inline bool ReverseSplitToArenaStringPieces(
    ArenaStringPiece input, Delimiter delimiter,
    ArenaStringPiece* output1, ArenaStringPiece* output2,
    ArenaStringPiece* output3, Args*... args) {
  ArenaStringPiece leading;
  if (!ReverseSplitToStringPieces(input, delimiter, &leading, output1)) {
    return false;
  }
  return ReverseSplitToStringPieces(
      leading, delimiter, args..., output3, output2);
}
