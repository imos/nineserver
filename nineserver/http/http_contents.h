#pragma once
#include <time.h>
#include <string>
#include <map>
#include <unordered_map>

#include "base/base.h"
#include "nineserver/arena/arena.h"
#include "nineserver/arena/arena_interface.h"
#include "nineserver/common.h"
#include "nineserver/io/generic_io.h"
#include "nineserver/json.h"
#include "nineserver/http/http_parameters.h"

class HttpContents : public ArenaInterface {
 public:
  // TODO(P2): Make this case insensitive.
  typedef map<StringPiece, vector<StringPiece>> Headers;

  const StringPiece kContentLengthHeader = StringPiece("Content-Length");
  const StringPiece kTransferEncodingHeader = StringPiece("Transfer-Encoding");

  HttpContents() : arena_(nullptr) {}
  ~HttpContents() {}

  // Initializes the object.
  void Init(Arena* arena);
  void Clear();

  bool ReadHeader(GenericIO* io);
  bool ReadContent(GenericIO* io);
  // ignore_content should be enabled for a reponse for HEAD.
  bool Read(GenericIO* io, bool ignore_content = false);

  bool Write(GenericIO* io, StringPiece hostname = "");

  // Appends a content.
  inline void Print() {}
  template<typename T, typename... Args>
  inline void Print(T&& content, Args&&... args) {
    if (DVLOG_IS_ON(15)) {
      LOG(INFO) << "HttpContents::Print(\"" << content << "\")";
    } else if (DVLOG_IS_ON(5)) {
      LOG(INFO) << "HttpContents::Print(" << StringPiece(content).size() << ")";
    }
    contents_.push_back(ToArenaStringPiece(std::forward<T>(content)));
    Print(std::forward<Args>(args)...);
  }

  // Sets header.
  template<typename K, typename V>
  void SetHeader(K&& key, V&& value, bool overwrite = true) {
    DVLOG(5) << "HttpContents::SetHeader(\"" << key << "\", \""
             << value << "\", " << overwrite << ")";
    if (overwrite) {
      headers_.Set(ToArenaStringPiece(std::forward<K>(key)),
                   ToArenaStringPiece(std::forward<V>(value)));
    } else {
      headers_.Add(ToArenaStringPiece(std::forward<K>(key)),
                   ToArenaStringPiece(std::forward<V>(value)));
    }
  }

  ArenaStringPiece GetHeader(
      StringPiece key,
      ArenaStringPiece default_value = ArenaStringPiece()) const {
    return ArenaStringPiece(headers_.Get(key, default_value));
  }

  const HttpParameters& GetEnvironments() const { return environments_; }
  HttpParameters* MutableEnvironments() { return &environments_; }

  ArenaStringPiece GetFirstContent() const {
    if (contents_.empty()) {
      DVLOG(5) << "Empty contents.";
      return ""_a;
    }
    return ArenaStringPiece(contents_[0]);
  }

  // Returns contents.  This is not const because this may join chunked
  // contents.
  ArenaStringPiece GetContents() {
    if (contents_.empty()) {
      return ""_a;
    }
    if (contents_.size() > 1) {
      int64 content_length = 0;
      for (StringPiece content : contents_) {
        content_length += content.size();
      }
      ArenaBuffer* buffer = arena_->NextBuffer();
      buffer->reserve(content_length);
      for (StringPiece content : contents_) {
        buffer->append(content);
      }
      contents_.clear();
      contents_.push_back(*buffer);
    }
    return ArenaStringPiece(contents_[0]);
  }

  void ClearContents() { return contents_.clear(); }

  void Destroy() { is_destroyed_ = true; }
  bool IsDestroyed() const { return is_destroyed_; }

  Json ToJson() const {
    return Json::object({
        {"environments", environments_},
        {"headers", headers_}});
  }

  Arena* MutableArena() override { return arena_; }

 private:

  Arena* arena_;
  bool is_destroyed_ = false;

  HttpParameters headers_;
  HttpParameters environments_;
  vector<StringPiece> output_buffer_;
  vector<StringPiece> contents_;
};
