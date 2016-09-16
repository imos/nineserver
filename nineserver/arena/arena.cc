#include "nineserver/arena/arena.h"

#include "base/base.h"

Arena::~Arena() { DeleteAll(); }

void Arena::DeleteAll() {
  string_deleters_.clear();
  for (const auto& deleter : deleters_) {
    deleter();
  }
  deleters_.clear();
  arena_buffer_.DeleteAll();
}
