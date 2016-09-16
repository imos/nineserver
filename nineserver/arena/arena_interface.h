#pragma once
#include <stdarg.h>

#include "base/base.h"
#include "nineserver/arena/arena.h"
#include "nineserver/arena/arena_buffer.h"

class ArenaInterface {
 public:
  template<typename... Args>
  inline ArenaStringPiece ArenaStrCat(const Args&... args) {
    ArenaBuffer* buffer = MutableArena()->NextBuffer();
    buffer->StrCat(args...);
    return *buffer;
  }

  ArenaStringPiece ArenaStringPrintf(const char* format, ...)
      // Tell the compiler to do printf format string checking.
      PRINTF_ATTRIBUTE(2, 3);

  inline ArenaStringPiece ToArenaStringPiece(string&& content) {
    return ArenaStringPiece(StringPiece(
        *MutableArena()->New<string>(std::move(content))));
  }
  inline ArenaStringPiece ToArenaStringPiece(ArenaStringPiece&& content)
      { return content; }
  inline ArenaStringPiece ToArenaStringPiece(const ArenaStringPiece& content) 
      { return content; }

  inline ArenaBuffer* NextBuffer() { return MutableArena()->NextBuffer(); }

 protected:
  virtual Arena* MutableArena() = 0;
};
