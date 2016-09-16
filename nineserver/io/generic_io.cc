#include "nineserver/io/generic_io.h"

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "base/base.h"
#include "nineserver/arena/arena_buffer.h"

void GenericIO::Init() {
  DVLOG(6) << "GenericIO::Init()";
  Destroy();
  is_destroyed_ = false;
  buffer_start_ = 0;
  buffer_end_ = 0;
}

bool GenericIO::ReadLine(BufferInterface* output) {
  DVLOG(6) << "GenericIO::ReadLine()";
  if (buffer_end_ < 0) {
    return false;
  }
  auto original_length = output->size();
  while (true) {
    StringPiece buffer(buffer_ + buffer_start_, buffer_end_ - buffer_start_);
    auto position = buffer.find('\n');
    if (position != StringPiece::npos) {
      output->append(buffer.substr(0, position + 1));
      buffer_start_ += position + 1;
      break;
    }
    output->append(buffer);
    if (!ReadBuffer()) {
      return false;
    } else if (buffer_end_ == 0) {
      break;
    }
  }
  return original_length != output->size();
}

int GenericIO::ReadN(int length, BufferInterface* output) {
  DVLOG(6) << "GenericIO::ReadN()";
  if (buffer_end_ < 0) {
    return false;
  }
  auto original_length = output->size();
  while (true) {
    StringPiece buffer(buffer_ + buffer_start_, buffer_end_ - buffer_start_);
    int remaining = original_length + length - output->size();
    if (remaining <= buffer.size()) {
      output->append(buffer.substr(0, remaining));
      buffer_start_ += remaining;
      break;
    }
    output->append(buffer);
    if (!ReadBuffer()) {
      return false;
    } else if (buffer_end_ == 0) {
      break;
    }
  }
  return output->size() - original_length;
}

int GenericIO::ReadCharacter() {
  DVLOG(6) << "GenericIO::ReadCharacter()";
  if (IsEof()) { return -1; }
  return buffer_[buffer_start_++];
}

bool GenericIO::IsEof() {
  DVLOG(6) << "GenericIO::IsEof()";
  if (buffer_end_ < 0) {
    return true;
  }
  if (buffer_start_ >= buffer_end_) {
    if (IsDestroyed()) { return true; }
    if (!ReadBuffer()) {
      // NOTE: This is not warning because IsEof is usually called to check if
      // the response finishes.
      DVLOG(5) << "Failed to read: " << strerror(errno);
      return true;
    // If it reaches EOF.
    } else if (buffer_end_ == 0) {
      DVLOG(6) << "Reaches EOF.";
      buffer_end_ = -1;
      return true;
    }
  }
  return false;
}

bool GenericIO::ReadBuffer() {
  DVLOG(6) << "GenericIO::ReadBuffer()";
  buffer_start_ = 0;
  buffer_end_ = ReadIO(buffer_, sizeof(buffer_));
  DVLOG(6) << "New buffer: " << buffer_end_;
  return buffer_end_ >= 0;
}

bool GenericIO::Write(StringPiece data) {
  if (data.empty()) { return true; }
  write_buffer_[0] = data;
  return WriteIO(write_buffer_);
}

bool GenericIO::Write(const vector<StringPiece>& data) {
  if (DVLOG_IS_ON(15)) {
    for (StringPiece piece : data) {
      LOG(INFO) << "GenericIO::Write(\"" << piece << "\")";
    }
  } else if (DVLOG_IS_ON(5)) {
    for (StringPiece piece : data) {
      LOG(INFO) << "GenericIO::Write(" << piece.size() << ")";
    }
  }
  return WriteIO(data);
}

void GenericIO::Destroy() {
  if (IsDestroyed()) { return; }
  DVLOG(6) << "GenericIO::Destroy()";
  is_destroyed_ = true;
  CloseIO();
}

bool GenericIO::IsDestroyed() const {
  return is_destroyed_;
}

bool GenericIO::Empty() const {
  return buffer_start_ == buffer_end_;
}
