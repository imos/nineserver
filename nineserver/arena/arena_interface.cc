#include "nineserver/arena/arena_interface.h"

#include "base/base.h"
#include "nineserver/arena/arena.h"

ArenaStringPiece ArenaInterface::ArenaStringPrintf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  ArenaBuffer* buffer = MutableArena()->NextBuffer();
  while (true) {
    buffer->resize(buffer->capacity());
    va_list backup_ap;
    va_copy(backup_ap, ap);
    int output = vsnprintf(buffer->data(), buffer->size(), format, backup_ap);
    va_end(backup_ap);
    if (output < 0) {
      buffer->clear();
      LOG(ERROR) << "Failed to call vsnprintf: " << strerror(errno);
      break;
    }
    if (output < buffer->size()) {
      buffer->resize(output);
      break;
    }
    buffer->clear();
    if (buffer->capacity() > 128 * 1024) {
      LOG(ERROR) << "Too big to ArenaStringPrintf.";
      break;
    }
    buffer->reserve(buffer->capacity() * 2);
  }
  va_end(ap);
  return *buffer;
}
