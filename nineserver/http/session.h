#pragma once
#include <functional>
#include <memory>
#include <mutex>

#include "base/base.h"

class Session {
 public:
  Session(StringPiece session_key) : session_key_(session_key.ToString())
      { Touch(); }
  Session() : Session(StringPiece("", 0)) {}
  ~Session() { Clear(); }

  const string& session_key() const { return session_key_; }
  int64 session_access() const {
    std::lock_guard<std::mutex> access_guard(access_mutex_);
    return access_;
  }

  template<typename T>
  const T* GetOrNull() const {
    TypeId type_id = GetTypeId<T>();
    std::lock_guard<std::mutex> data_guard(data_mutex_);
    return type_id < data_.size() ? static_cast<T*>(data_[type_id]) : nullptr;
  }

  template<typename T>
  const T& Get() const {
    static T kEmpty = T();
    const T* value = GetOrNull<T>();
    return value == nullptr ? kEmpty : *value;
  }

  template<typename T>
  T* Mutable() {
    TypeId type_id = GetTypeId<T>();
    std::lock_guard<std::mutex> data_guard(data_mutex_);
    if (type_id >= data_.size()) {
      data_.resize(type_id + 1, nullptr);
    }
    if (data_[type_id] == nullptr) {
      data_[type_id] = new T();
    }
    return static_cast<T*>(data_[type_id]);
  }

  template<typename T>
  void Set(std::unique_ptr<T> value) {
    TypeId type_id = GetTypeId<T>();
    std::lock_guard<std::mutex> data_guard(data_mutex_);
    data_.resize(type_id + 1, nullptr);
    data_[type_id] = value.release();
  }

  std::unique_lock<std::mutex> Lock() const {
    Touch();
    return std::unique_lock<std::mutex>(mutex_);
  }

  void Clear() {
    vector<void*> old_data;
    {
      std::lock_guard<std::mutex> data_guard(data_mutex_);
      old_data.swap(data_);
    }
    std::lock_guard<std::mutex> deleters_guard(deleters_mutex_);
    for (int i = 0; i < old_data.size(); i++) {
      (*deleters_)[i](old_data[i]);
    }
  }

  bool Empty() const {
    std::lock_guard<std::mutex> data_guard(data_mutex_);
    return data_.empty();
  }

  void Touch() const {
    static int64 access_id = 1;
    static std::mutex access_id_mutex;
    int new_access_id;
    {
      std::lock_guard<std::mutex> access_id_guard(access_id_mutex);
      new_access_id = access_id;
    }
    {
      std::lock_guard<std::mutex> access_guard(access_mutex_);
      access_ = new_access_id;
    }
  }

 private:
  typedef int TypeId;
  typedef std::function<void(void*)> Deleter;

  static TypeId IncrementTypeId(const Deleter& deleter) {
    std::lock_guard<std::mutex> deleters_guard(deleters_mutex_);
    if (deleters_ == nullptr) {
      deleters_ = new vector<Deleter>();
    }
    deleters_->push_back(deleter);
    return deleters_->size() - 1;
  }

  template<typename T>
  static TypeId GetTypeId() {
    static TypeId type_id = IncrementTypeId(
        [](void* ptr) { delete static_cast<T*>(ptr); });
    return type_id;
  }

  string session_key_;
  mutable int64 access_ = 0;
  mutable std::mutex access_mutex_;
  vector<void*> data_;
  mutable std::mutex data_mutex_;
  mutable std::mutex mutex_;
  static vector<Deleter>* deleters_;
  static std::mutex deleters_mutex_;
};

std::shared_ptr<Session> GetSession(StringPiece session_key);
bool SetSession(std::shared_ptr<Session> session);
bool RemoveSession(StringPiece session_key);
