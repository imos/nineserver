#include "nineserver/arena/arena_string_piece.h"

#include "base/base.h"

ArenaStringPiece operator"" _a(const char* str, std::size_t length) {
  return ArenaStringPiece(str, length);
}
