#include "nineserver/handler/base_handler.h"

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>

#include "base/base.h"
#include "nineserver/common.h"
#include "nineserver/handler/logging.h"
#include "nineserver/handler/resource_pool.h"
#include "nineserver/http/stream/http_stream.h"
#include "nineserver/http/stream/standalone_stream.h"
#include "nineserver/http/util.h"
#include "nineserver/signal.h"

DEFINE_bool(profile_header, true, "add X-Profile header.");
DEFINE_bool(handler_profile, true, "enable profiling in BaseHandler.");

namespace {

const ArenaStringPiece kLoggingCpuKey = "cpu"_a;
const ArenaStringPiece kLoggingRealKey = "real"_a;
const ArenaStringPiece kLoggingResponseKey = "response"_a;

void RegisterLoggingKeys() {
  Logging::RegisterField(kLoggingCpuKey, "CPU", "ms", 1000);
  Logging::RegisterField(kLoggingRealKey, "Real", "ms", 1000);
  Logging::RegisterField(kLoggingResponseKey, "Response", "KB", 1000);
}

}  // namespace

bool BaseHandler::Process() {
  double start_cpu = GetThreadClock();
  double start_time = GetCurrentTime();
  bool result = SendResource(ArenaStrCat("www"_a, ENV("SCRIPT_NAME"_a))) ||
                GetHandlerPool()->Run(this) || SystemHandler() || Run() ||
                SendResource(ArenaStrCat("default"_a, ENV("SCRIPT_NAME"_a)));
  if (result) {
    GetFilterPool()->Run(this);
    PostProcess();
  }
  if (!result) {
    MutableResponse()->Clear();
    MutableResponse()->SetHeader(""_a, "404 Not Found"_a);
    MutableResponse()->SetHeader("Content-Type"_a, "text/plain"_a);
    MutableResponse()->Print("No handler processed."_a);
  }
  if (FLAGS_profile_header) {
    SetHeader("X-Profile"_a,
              ArenaStringPrintf(
                  "%.3f ms / %.3f ms",
                  (GetThreadClock() - start_cpu) * 1e3,
                  (GetCurrentTime() - start_time) * 1e3));
  }
  Profile(GetThreadClock() - start_cpu, GetCurrentTime() - start_time);
  Finish();
  return result;
}

void BaseHandler::Finish() {
  if (session_ != nullptr &&
      session_->session_key() != COOKIE("nine_session"_a)) {
    SetCookie("nine_session"_a, session_->session_key(), 0, "/"_a);
  }
  stream_->Finish();
  if (session_ != nullptr) {
    SetSession(session_);
    session_.reset();
  }
}

bool BaseHandler::SendResource(StringPiece file, bool cache) {
  DVLOG(5) << "BaseHandler::SendResource(\"" << file << "\")";
  const StringPiece* data = ResourcePool::GetOrNull(file);
  if (data == nullptr) {
    if (HasSuffixString(file, "/")) {
      return SendResource(ArenaStrCat(file, "index.html")) ||
             SendResource(ArenaStrCat(file, "index.htm"));
    }
    return false;
  }
  ArenaStringPiece etag = ArenaStrCat("\"", file, "\"");
  if (NotModified(etag)) { return true; }
  StringPiece extension;
  ReverseSplitToStringPieces(file, '.', nullptr, &extension);
  ArenaStringPiece content_type = GetMimeType(extension);
  SetHeader(""_a, "HTTP/1.1 200 OK"_a);
  SetHeader("Etag"_a, etag);
  if (cache) {
    SetHeader("Cache-Control"_a, "public, max-age=600"_a);
  }
  if (content_type.empty()) {
    SetHeader("Content-Type"_a, "application/octet-stream"_a);
  } else {
    SetHeader("Content-Type"_a, content_type);
  }
  Print(ArenaStringPiece(*data));
  return true;
}

bool BaseHandler::NotModified(StringPiece expected_etag) {
  ArenaStringPiece etag = MutableRequest()->GetHeader("If-None-Match"_a);
  if (etag.empty()) { return false; }
  if (!expected_etag.empty() && etag != expected_etag) { return false; }
  SetHeader(""_a, "HTTP/1.1 304 Not Modified"_a);
  SetHeader("Etag"_a, etag);
  return true;
}

std::shared_ptr<Session> BaseHandler::MutableSession() {
  if (session_ != nullptr) { return session_; }
  StringPiece session_key = COOKIE("nine_session"_a);
  if (!session_key.empty()) {
    session_ = GetSession(session_key);
    return session_;
  }
  char buffer[16];
  for (int i = 0; i < sizeof(buffer) / sizeof(buffer[0]); i++) {
    buffer[i] = 'A' + (Rand() % 26);
  }
  session_.reset(new Session(StringPiece(buffer, sizeof(buffer))));
  return session_;
}

string BaseHandler::ProfileName() {
  return ENV("SCRIPT_NAME"_a).ToString();
}

void BaseHandler::Profile(double cpu_time, double real_time) {
  if (!FLAGS_handler_profile) { return; }
  DVLOG(10) << "Recording profile: cpu_time="
            << cpu_time << ", real_time=" << real_time;
  ArenaStringPiece response_code;
  if (!SplitToArenaStringPieces(MutableResponse()->GetHeader(""_a),
                                ' ', nullptr, &response_code, nullptr)) {
    response_code = "???"_a;
  }
  string path = StrCat(
      ENV("REQUEST_METHOD").ToStringPiece(), " ", ProfileName(), " ",
      response_code.ToStringPiece());
  Logging::Record(path, ""_a, 1);
  Logging::Record(path, kLoggingCpuKey, round(cpu_time * 1e6));
  Logging::Record(path, kLoggingRealKey, round(real_time * 1e6));
  Logging::Record(path, kLoggingResponseKey,
                  MutableResponse()->GetContents().size());
}

bool BaseHandler::SystemHandler() {
  ArenaStringPiece uri = ENV("SCRIPT_NAME"_a);
  if (!HasPrefixString(uri, "/_/"_a)) { return false; }
  SetHeader("Content-Type"_a, "text/plain; charset=UTF-8"_a);
  ArenaStringPiece path;
  // Parses URI like "/_/path".
  if (!SplitToArenaStringPieces(uri, '/', nullptr, nullptr, &path)) {
    SetHeader(""_a, "HTTP/1.1 500 Internal Error"_a);
    Print("Something wrong with URI: "_a, uri);
    return true;
  }

  if (path == "exit"_a) {
    Print("OK\n"_a);
    MutableStream()->SetKeepAlive(false);
    MutableStream()->Destroy();
    RunExitHandlers();
  } else if (path == "var"_a) {
    StringPiece key = GET("key"_a);
    Print(JsonEncode(
        key.empty() ? stream_->ToJson() : stream_->ToJson()[key.ToString()],
        GET("pretty"_a, ""_a) != ""_a));
  } else if (path == "resource"_a) {
    StringPiece key = GET("key"_a);
    if (key.empty()) {
      SetHeader("Content-Type"_a, "text/html; charset=UTF-8"_a);
      Print("<html><body>Resource<ul>"_a);
      for (const auto& name_and_body : ResourcePool::Get()) {
        Print("<li><a href=\"?key="_a, ArenaStringPiece(name_and_body.first),
              "\">"_a, ArenaStringPiece(name_and_body.first), "</a> ("_a,
              ArenaStrCat(name_and_body.second.size()), " bytes)</li>"_a);
      }
      Print("</ul></body></html>"_a);
    } else {
      SendResource(key);
    }
  } else if (path == "cookie"_a) {
    StringPiece max_age = GET("max_age"_a, "0"_a);
    int32 value;
    if (safe_strto32(max_age.data(), max_age.size(), &value)) {
      SetCookie(GET("key"_a), GET("value"_a), value, GET("path"_a));
    } else {
      Print("Invalid max-age.\n"_a);
    }
  } else {
    SetHeader(""_a, "HTTP/1.1 404 Not Found"_a);
    Print("No such system handler: "_a, path, "\n"_a);
  }
  return true;
}

BaseHandlerFactory::BaseHandlerFactory() {
  GetHandlerPool()->PrintHandlers("Handler");
  GetFilterPool()->PrintHandlers("Filter");
  RegisterLoggingKeys();
}

void BaseHandlerFactory::Run() {
  auto handler = [this](Stream* stream) {
    std::unique_ptr<BaseHandler> handler(this->NewHandler());
    handler->SetStream(stream);
    while (true) {
      if (!handler->Init()) break;
      while (handler->Start()) {
        handler->Process();
        if (handler->MayRelease()) break;
      }
      handler->Destroy();
    }
  };
  StandaloneStream::StartThreads(handler, &threads_);
  for (std::thread& thread : threads_) { thread.join(); }
  LOG(INFO) << "All threads joined.";
}

void HandlerPool::Register(StringPiece prefix,
                           const std::function<bool(BaseHandler*)>& handler) {
  std::lock_guard<std::mutex> handlers_guard(handlers_mutex_);
  handlers_.emplace(prefix, handler);
}

bool HandlerPool::Run(BaseHandler* handler) {
  if (handlers_.empty()) { return false; }
  StringPiece path =
      handler->MutableRequest()->GetEnvironments().Get("SCRIPT_NAME"_a);
  char buffer[path.size() + 1];
  memcpy(buffer, path.data(), path.size());
  buffer[path.size()] = '$';
  StringPiece search_path(buffer, sizeof(buffer));
  while (!search_path.empty()) {
    auto upper_bound = handlers_.upper_bound(search_path);
    if (upper_bound == handlers_.begin()) {
      return false;
    }
    upper_bound--;
    while (search_path.starts_with(upper_bound->first)) {
      DVLOG(5) << "Calling handler: " << search_path;
      if (upper_bound->second(handler)) { return true; }
      if (upper_bound == handlers_.begin()) {
        return false;
      }
      upper_bound--;
    }
    search_path.remove_suffix(max(
        (int64)search_path.size() - (int64)upper_bound->first.size(),
        static_cast<int64>(1LL)));
    for (int i = 0; i < search_path.size(); i++) {
      if (search_path[i] != upper_bound->first[i]) {
        search_path.remove_suffix(search_path.size() - i);
        break;
      }
    }
  }
  return false;
}

void HandlerPool::PrintHandlers(StringPiece purpose) {
  for (const auto& prefix_and_handler : handlers_) {
    DLOG(INFO) << purpose << " for \"" << prefix_and_handler.first << "\"";
  }
}

HandlerPool* GetHandlerPool() {
  static HandlerPool* pool = new HandlerPool();
  return pool;
}

HandlerPool* GetFilterPool() {
  static HandlerPool* pool = new HandlerPool();
  return pool;
}
