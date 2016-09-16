#pragma once
#include <memory>
#include <string>
#include <vector>

#include "base/base.h"
#include "nineserver/common.h"
#include "nineserver/arena/arena_string_piece.h"

// Interface to access to both of ArenaBuffer and string.
class BufferInterface {
 public:
  BufferInterface() {}
  virtual ~BufferInterface() {}

  virtual void resize(int64 size) = 0;
  virtual void reserve(int64 size) = 0;
  virtual void append(StringPiece str) = 0;
  // Falls back to append.
  virtual void push_back(char c) {
    char buffer[1] = {c};
    append(StringPiece(buffer, 1));
  }

  virtual const char* data() const = 0;
  virtual int64 size() const = 0;

  operator StringPiece() const { return StringPiece(data(), size()); }

  template<typename V, typename... Args>
  inline void StrCat(const V& value, const Args&... args) {
    AlphaNum alpha_num(value);
    append(StringPiece(alpha_num.data(), alpha_num.size()));
    StrCat(args...);
  }
  inline void StrCat() {}
};

// ArenaBuffer manages long memory areas, and provides the last piece as a
// buffer.  Caller can resize the last piece.  Once the piece's size is
// finalized, it can be converted into StringPiece safely while ArenaBuffer
// is alive.
//
// How to create a buffer:
// 1. Init() allocates a new 0-byte buffer.
// 2. resize methods (incl. push_back/append) resizes the buffer.
// 3. ToStringPiece() returns the current buffer as StringPiece.
// 4. Next Init() call finalizes the memory location.
class ArenaBuffer : public BufferInterface {
 public:
  ArenaBuffer();

  // Initialize the ArenaBuffer.  This may release memories, so any StringPiece
  // created by this object must be invalidated.
  void DeleteAll();

  // Prepares a new buffer.
  void Init();

  //////////////////////////////////////////////////////////////////////////////
  // Resize functions
  // CAVEAT: Resize function may change the current buffer's memory location.
  //////////////////////////////////////////////////////////////////////////////

  // Resizes the buffer into `size`.
  void resize(int64 size) override;
  // An alias of resize(0).
  void clear();
  // Add a character `c` at the end.
  void push_back(char c) override;
  // Appends a string `str` at the end.
  void append(StringPiece str) override;
  // Reserves capacity.
  void reserve(int64 size) override;
  // Returns capacity.
  int64 capacity() const;
  // Returns usage.
  int64 usage() const;

  //////////////////////////////////////////////////////////////////////////////
  // Accessors
  //////////////////////////////////////////////////////////////////////////////
  char& operator[](int64 index) { return data()[index]; }
  char operator[](int64 index) const { return data()[index]; }

  inline char* data()
      { return buffers_.back().data_.get() + buffers_.back().size_ - size_; }
  inline const char* data() const override
      { return buffers_.back().data_.get() + buffers_.back().size_ - size_; }
  inline int64 size() const override { return size_; }

  operator StringPiece() const { return StringPiece(data(), size()); }
  operator ArenaStringPiece() const { return ToArenaStringPiece(); }

  ArenaStringPiece ToArenaStringPiece() const {
    return ArenaStringPiece(StringPiece(*this));
  }

 private:
  class Buffer {
   public:
    explicit Buffer(int capacity)
        : data_(new char[capacity]), capacity_(capacity), size_(0) {}
    Buffer(Buffer&& buffer) { *this = std::move(buffer); }
    ~Buffer() noexcept {}

    void operator=(Buffer&& buffer) {
      data_ = std::move(buffer.data_);
      capacity_ = buffer.capacity_;
      size_ = buffer.size_;
      buffer.capacity_ = 0;
      buffer.size_ = 0;
    }

    std::unique_ptr<char[]> data_;
    int64 capacity_;
    int64 size_;
  };

  // Prepares a new memory area.
  void AddBuffer(int length = 0);

  vector<Buffer> buffers_;
  int64 size_ = 0;
  int64 usage_ = 0;
};

class StringBuffer : public BufferInterface {
 public:
  StringBuffer(string* buffer) : buffer_(buffer) {}
  StringBuffer() : buffer_(new string), buffer_is_owned_(true) {}
  ~StringBuffer() override {
    if (buffer_is_owned_) {
      delete buffer_;
    }
  }

  string* get() const { return buffer_; }

  const char* data() const override { return buffer_->data(); }
  int64 size() const override { return static_cast<int64>(buffer_->size()); }
  void resize(int64 size) override { buffer_->resize(size); }
  void reserve(int64 size) override { buffer_->reserve(size); }
  void append(StringPiece str) override
      { buffer_->append(str.data(), str.size()); }

  operator const string&() const { return *buffer_; }
  operator string&&() { return std::move(*buffer_); }

 private:
  string* buffer_;
  bool buffer_is_owned_ = false;
};
