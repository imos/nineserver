#pragma once
#include <thread>
#include <vector>

#include "base/base.h"
#include "nineserver/arena/arena_interface.h"
#include "nineserver/arena/arena_string_piece.h"
#include "nineserver/base64.h"
#include "nineserver/http/session.h"
#include "nineserver/http/stream/http_stream.h"
#include "third_party/glog/base/googleinit.h"

class BaseHandler : public ArenaInterface {
 public:
  BaseHandler() {}
  virtual ~BaseHandler() {}

  //////////////////////////////////////////////////////////////////////////////
  // Process functions
  //////////////////////////////////////////////////////////////////////////////
  void SetStream(Stream* stream) { stream_ = stream; }
  bool Init() { return stream_->Init(); }
  bool Start() { return stream_->Start(); }
  bool Process();
  void Destroy() { stream_->Destroy(); }
  void Finish();
  bool MayRelease() { return stream_->MayRelease(); }

  //////////////////////////////////////////////////////////////////////////////
  // Request functions
  //////////////////////////////////////////////////////////////////////////////
  inline const HttpParameters& ENV() const
      { return stream_->GetRequest().GetEnvironments(); }
  inline const ArenaStringPiece ENV(
      StringPiece key, ArenaStringPiece default_value = ""_a) const
      { return ENV().Get(key, default_value); }
  inline const HttpParameters& GET() const
      { return stream_->GetQueryParameters(); }
  inline const ArenaStringPiece GET(
      StringPiece key, ArenaStringPiece default_value = ""_a) const
      { return GET().Get(key, default_value); }
  inline const HttpParameters& POST() const
      { return stream_->GetPostParameters(); }
  inline const ArenaStringPiece POST(
      StringPiece key, ArenaStringPiece default_value = ""_a) const
      { return POST().Get(key, default_value); }
  inline const HttpParameters& COOKIE() const
      { return stream_->GetCookieParameters(); }
  inline const ArenaStringPiece COOKIE(
      StringPiece key, ArenaStringPiece default_value = ""_a) const
      { return COOKIE().Get(key, default_value); }
  inline const map<StringPiece, std::unique_ptr<HttpParameters>>& FILE() const
      { return stream_->GetFileParameters(); }
  inline const HttpParameters& FILE(StringPiece key) const {
    static HttpParameters http_parameters;
    const std::unique_ptr<HttpParameters>& file =
        FindWithDefault(FILE(), key, nullptr);
    return file != nullptr ? *file : http_parameters;
  }
  inline const Json& SESSION() const { return stream_->GetSessionParameters(); }

  //////////////////////////////////////////////////////////////////////////////
  // Response functions
  //////////////////////////////////////////////////////////////////////////////
  template<typename K, typename V>
  inline void SetHeader(K&& key, V&& value, bool overwrite = true) {
    MutableResponse()->SetHeader(key, value, overwrite);
  }
  template<typename... Args>
  inline void Print(Args&&... args) {
    MutableResponse()->Print(std::forward<Args>(args)...);
  }

  inline void SetCookie(
      StringPiece key, StringPiece value,
      int64 age = 0, StringPiece path = "/") {
    ArenaBuffer* buffer = MutableArena()->NextBuffer();
    UrlEncode(key, buffer);
    buffer->push_back('=');
    UrlEncode(value, buffer);
    if (age != 0) {
      buffer->append("; max-age="_a);
      buffer->StrCat(age);
    }
    if (!path.empty()) {
      buffer->append("; path="_a);
      buffer->append(path);
    }
    SetHeader("Set-Cookie"_a, *buffer, false);
  }

  void SetSessionData(const Json& value) {
    *stream_->MutableSessionParameters() = value;
    if (value.type() == Json::NUL) {
      SetCookie("session_data"_a, "", -1);
      return;
    }
    SetCookie("session_data"_a, Base64::UrlEncode(JsonEncode(value)));
  }

  // Sets a content-type and sends a file.
  bool SendResource(StringPiece path, bool cache = true);

  // TODO(P1): Implement cache control.

  bool NotModified(StringPiece expected_etag = StringPiece());

  //////////////////////////////////////////////////////////////////////////////
  // Accessors
  //////////////////////////////////////////////////////////////////////////////
  inline Stream* MutableStream() { return stream_; }
  inline HttpContents* MutableRequest()
      { return MutableStream()->MutableRequest(); }
  inline HttpContents* MutableResponse()
      { return MutableStream()->MutableResponse(); }
  std::shared_ptr<Session> MutableSession();

 protected:
  //////////////////////////////////////////////////////////////////////////////
  // Interfaces
  //////////////////////////////////////////////////////////////////////////////
  virtual bool Run() { return false; }
  virtual void PostProcess() {}

  virtual string ProfileName();
  virtual void Profile(double cpu_time, double real_time);

  // For ArenaInterface.
  Arena* MutableArena() override { return stream_->MutableArena(); }

 private:
  bool SystemHandler();

  std::shared_ptr<Session> session_;
  Stream* stream_;
};

class BaseHandlerFactory {
 public:
  BaseHandlerFactory();
  virtual ~BaseHandlerFactory() {}

  virtual BaseHandler* NewHandler() = 0;
  void Run();

 private:
  vector<std::thread> threads_;
};

class HandlerPool {
 public:
  void Register(StringPiece prefix,
                const std::function<bool(BaseHandler*)>& handler);

  template<typename T>
  typename std::enable_if<
      std::is_base_of<BaseHandler, T>::value &&
      !std::is_same<BaseHandler, T>::value,
      void>::type
      Register(StringPiece prefix, const std::function<bool(T*)>& handler) {
    Register(prefix, [handler](BaseHandler* base_handler) {
      T* original_handler = dynamic_cast<T*>(base_handler);
      if (original_handler == nullptr) { return false; }
      return handler(original_handler);
    });
  }

  template<typename T>
  void Register(StringPiece prefix, bool(*handler)(T*)) {
    Register(prefix, std::function<bool(T*)>(handler));
  }

  bool Run(BaseHandler* stream);

  void PrintHandlers(StringPiece purpose);

 private:
  multimap<StringPiece, std::function<bool(BaseHandler*)>> handlers_;
  std::mutex handlers_mutex_;
};

HandlerPool* GetHandlerPool();
HandlerPool* GetFilterPool();

#define REGISTER_HANDLER(name, prefix, body) \
    REGISTER_MODULE_INITIALIZER(name, { \
      GetHandlerPool()->Register(prefix, body); });
#define REGISTER_FILTER(name, prefix, body) \
    REGISTER_MODULE_INITIALIZER(name, { \
      GetFilterPool()->Register(prefix, body); });
