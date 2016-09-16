#pragma once
#include <map>
#include <string>

#include "base/base.h"
#include "nineserver/arena/arena.h"
#include "nineserver/http/http_contents.h"
#include "nineserver/http/http_parameters.h"
#include "nineserver/http/url.h"

// 共通HTTP入出力ストリーム．
class Stream {
 public:
  static const ArenaStringPiece kHttpOk;
  static const ArenaStringPiece kHttpBadRequest;

  Stream() {}
  virtual ~Stream() {
    CHECK(is_destroyed_) << "Destroy must be called before destructing the "
                         << "stream to avoid 'pure virtual function call'.";
  }

  virtual bool Init();

  // Initialize Stream object for a request.
  bool Start();
  void Finish();

  // This may release the current IO to the pool.  The released IO should be
  // handled by Init in the future.
  virtual bool MayRelease() { return false; }

  // Finish the current response.  The stream must not be resused after Destroy.
  virtual void Destroy();
  bool IsDestroyed() const;

  virtual bool Read();
  virtual void Flush() {};
  virtual const GenericIO& GetIO() const = 0;
  virtual GenericIO* MutableIO() = 0;

  void SetKeepAlive(bool allow_keep_alive = true)
      { allow_keep_alive_ = allow_keep_alive; }
  bool GetKeepAlive() const { return allow_keep_alive_; }
  // Returns true if the current request can keep alive.
  virtual bool CanKeepAlive() const;

  const HttpParameters& GetQueryParameters() const { return get_parameters_; }
  const HttpParameters& GetPostParameters() const { return post_parameters_; }
  const HttpParameters& GetCookieParameters() const
      { return cookie_parameters_; }
  const map<StringPiece, std::unique_ptr<HttpParameters>>&
      GetFileParameters() const { return file_parameters_; }
  const Json& GetSessionParameters() const { return session_parameters_; }
  Json* MutableSessionParameters() { return &session_parameters_; }

  const HttpContents& GetRequest() const { return request_; }
  inline HttpContents* MutableRequest() { return &request_; }
  inline HttpContents* MutableResponse() { return &response_; }

  Arena* MutableArena() { return &arena_; }

  Json ToJson() const;

  string DebugString() const;

 protected:
  // Fills parameters based on server parameters and data.
  bool ParseParameters();

 protected:
  static const string* kEmptyString;

  // Whether to allow keep-alive.
  bool allow_keep_alive_ = false;
  // Whether the Stream is started and not finished.
  bool is_started_ = false;
  // Whether the Stream is destroyed and Init must be called.
  bool is_destroyed_ = true;

  // Following parameters depend on a request, and they must be cleared in
  // Start.
  Arena arena_;
  HttpContents request_;
  HttpContents response_;
  HttpParameters get_parameters_;
  HttpParameters post_parameters_;
  HttpParameters cookie_parameters_;
  Json session_parameters_;
  map<StringPiece, std::unique_ptr<HttpParameters>> file_parameters_;

 private:
  void Clear();

  DISALLOW_COPY_AND_ASSIGN(Stream);
};
