#pragma once
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

#include "base/base.h"
#include "nineserver/arena/arena_buffer.h"
#include "nineserver/common.h"

class Arena {
 public:
  Arena() {}
  Arena(Arena&& arena) { *this = std::move(arena); }
  Arena& operator=(Arena&& arena) {
    string_deleters_ = std::move(arena.string_deleters_);
    deleters_ = std::move(arena.deleters_);
    return *this;
  }
  ~Arena();

  void DeleteAll();

  template<class T, class... Args>
  T* New(Args... args) {
    T* pointer = new T(std::forward<Args>(args)...);
    AddDeleter(pointer);
    return pointer;
  }

  ArenaBuffer* NextBuffer() {
    arena_buffer_.Init();
    return &arena_buffer_;
  }

 private:
  template<class T>
  std::enable_if<!std::is_same<T, string>::value, void> AddDeleter(T* pointer) {
    deleters_.push_back([pointer]() { delete pointer; });
  }

  void AddDeleter(string* pointer) {
    string_deleters_.push_back(std::unique_ptr<string>(pointer));
  }

  vector<std::unique_ptr<string>> string_deleters_;
  vector<std::function<void()>> deleters_;
  ArenaBuffer arena_buffer_;

  DISALLOW_COPY_AND_ASSIGN(Arena);
};
