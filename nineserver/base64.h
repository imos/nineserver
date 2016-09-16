#pragma once
#include "base/base.h"
#include "nineserver/arena/arena_buffer.h"

class Base64 {
 public:
  enum Option {
    NONE = 0,
    URL = 1,
    PADDING = 2,
  };

  static void Encode(StringPiece data, int option, BufferInterface* buffer);

  static string Encode(StringPiece data, int option = PADDING) {
    StringBuffer string_buffer;
    Encode(data, option, &string_buffer);
    return std::move(*string_buffer.get());
  }

  static string UrlEncode(StringPiece data, int option = URL) {
    StringBuffer string_buffer;
    Encode(data, option, &string_buffer);
    return std::move(*string_buffer.get());
  }

  static void Decode(StringPiece data, int option, BufferInterface* buffer);

  static string Decode(StringPiece data, int option = NONE) {
    StringBuffer string_buffer;
    Decode(data, option, &string_buffer);
    return std::move(*string_buffer.get());
  }
};
