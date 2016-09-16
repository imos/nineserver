#pragma once
#include <string>

#include "base/base.h"
#include "nineserver/arena/arena_buffer.h"

// I/O interface.
//
// Life-cycle:
// 1. Call a custom Init.
// 2. Do Read/Write.
// 3. Call Destroy.
//
// CAVEAT: all methods are not thread-safe.
class GenericIO {
 public:
  GenericIO() : write_buffer_(1) {}
  virtual ~GenericIO() {
    DCHECK(IsDestroyed())
        << "Implementation must call Destroy() in its destructor.";
  }

  // Returns true iff character(s) are appended.
  bool ReadLine(BufferInterface* output);
  int ReadN(int length, BufferInterface* output);
  int ReadCharacter();
  bool IsEof();
  bool Write(StringPiece data);
  bool Write(const vector<StringPiece>& data);
  void Destroy();
  bool IsDestroyed() const;
  bool Empty() const;

 protected:
  // CAVEAT: Implementation must implement custom Init(s).  They must call Init.
  void Init();

  virtual bool WriteIO(const vector<StringPiece>& data) = 0;
  virtual int ReadIO(char* buffer, int length) = 0;
  virtual void CloseIO() = 0;

 private:
  bool ReadBuffer();

  bool is_destroyed_ = true;
  int buffer_start_;
  int buffer_end_;
  char buffer_[4096];
  // Buffer for Write(StringPiece) not to allocate vector.
  vector<StringPiece> write_buffer_;

  DISALLOW_COPY_AND_ASSIGN(GenericIO);
};
