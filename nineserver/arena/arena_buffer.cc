#include "nineserver/arena/arena_buffer.h"

#include "base/base.h"

DEFINE_int32(arena_buffer_size, 16 * 1024, "Initial arena buffer size.");

ArenaBuffer::ArenaBuffer() {
  buffers_.emplace_back(FLAGS_arena_buffer_size);
}

void ArenaBuffer::DeleteAll() {
  buffers_.erase(buffers_.begin() + 1, buffers_.end());
  buffers_[0].size_ = 0;
  usage_ = 0;
}

void ArenaBuffer::Init() { size_ = 0; }

void ArenaBuffer::clear() { resize(0); }

void ArenaBuffer::push_back(char c) {
  resize(size() + 1);
  buffers_.back().data_[buffers_.back().size_ - 1] = c;
}

void ArenaBuffer::resize(int64 size) {
  reserve(size);
  buffers_.back().size_ += size - size_;
  size_ = size;
}

void ArenaBuffer::append(StringPiece str) {
  resize(size() + str.size());
  memcpy(buffers_.back().data_.get() + buffers_.back().size_ - str.size(),
         str.data(), str.size());
}

void ArenaBuffer::reserve(int64 size) {
  if (capacity() < size) {
    AddBuffer(size);
  }
}

int64 ArenaBuffer::capacity() const {
  return buffers_.back().capacity_ - buffers_.back().size_ + size_;
}

int64 ArenaBuffer::usage() const {
  return usage_ + buffers_.back().size_;
}

void ArenaBuffer::AddBuffer(int length) {
  DVLOG(10) << "ArenaBuffer::AddBuffer(" << length << ")";
  if (length < buffers_.back().capacity_ * 2) {
    length = buffers_.back().capacity_ * 2;
  }
  usage_ += buffers_.back().capacity_;
  length = (length + 8191) / 8192 * 8192;
  StringPiece original = *this;
  buffers_.emplace_back(length);
  DCHECK_LE(original.size(), length);
  buffers_.back().size_ = original.size();
  memcpy(buffers_.back().data_.get(), original.data(), original.size());
}
