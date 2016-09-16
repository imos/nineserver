#include "nineserver/http/stream/http_stream.h"

#include "nineserver/base64.h"
#include "nineserver/common.h"
#include "nineserver/http/http_contents.h"
#include "nineserver/http/http_parameters.h"
#include "nineserver/http/url.h"
#include "nineserver/http/util.h"
#include "nineserver/json.h"

#include <memory>

const string* Stream::kEmptyString = new string("");
const ArenaStringPiece Stream::kHttpOk = "HTTP/1.1 200 OK"_a;
const ArenaStringPiece Stream::kHttpBadRequest = "HTTP/1.1 400 Bad Request"_a;

bool Stream::Init() {
  DVLOG(5) << "Stream::Init()";
  is_destroyed_ = false;
  allow_keep_alive_ = false;
  Clear();
  return true;
}

void Stream::Clear() {
  arena_.DeleteAll();
  request_.Init(&arena_);
  response_.Init(&arena_);
  get_parameters_.Clear();
  post_parameters_.Clear();
  cookie_parameters_.Clear();
  file_parameters_.clear();
  if (session_parameters_.type() != Json::NUL) {
    session_parameters_ = Json(nullptr);
  }
}

bool Stream::Start() {
  DVLOG(5) << "Stream::Start()";
  Finish();
  if (IsDestroyed()) { return false; }
  if (MutableIO()->IsEof()) {
    Destroy();
    return false;
  }

  is_started_ = true;
  Clear();

  if (!Read() || !ParseParameters()) {
    response_.SetHeader(""_a, kHttpBadRequest);
    response_.SetHeader("Content-Type"_a, "text/plain; charset=UTF-8"_a);
    response_.Print("Failed to parse request:\n"_a);
    response_.Print(DebugString());
    Finish();
    Destroy();
    return false;
  }

  response_.SetHeader(""_a, "HTTP/1.1 200 OK"_a, false /* overwrite */);
  response_.SetHeader(
      "Content-Type"_a, "text/html; charset=UTF-8"_a, false /* overwrite */);
  return true;
}

bool Stream::Read() {
  DVLOG(5) << "Stream::Read()";
  return MutableRequest()->Read(MutableIO());
}

void Stream::Finish() {
  if (!is_started_) { return; }

  DVLOG(5) << "Stream::Finish()";
  is_started_ = false;

  bool keep_alive = CanKeepAlive();
  DVLOG(5) << "KeepAlive: " << keep_alive;
  if (keep_alive) {
    MutableResponse()->SetHeader("Connection"_a, "keep-alive"_a);
    MutableResponse()->SetHeader("Keep-Alive"_a, "timeout=1"_a);
  } else {
    MutableResponse()->SetHeader("Connection"_a, "close"_a);
  }
  if (!MutableResponse()->Write(MutableIO())) {
    keep_alive = false;
  }
  Flush();
  if (!keep_alive) { Destroy(); }
}

void Stream::Destroy() {
  DVLOG(5) << "Stream::Destroy()";
  if (is_destroyed_) { return; }
  SetKeepAlive(false);
  Finish();
  is_destroyed_ = true;
  if (!GetIO().IsDestroyed()) {
    MutableIO()->Destroy();
  }
  Clear();
}

bool Stream::IsDestroyed() const {
  return GetIO().IsDestroyed() || is_destroyed_;
}

bool Stream::CanKeepAlive() const {
  if (!allow_keep_alive_) { return false; }
  if (IsDestroyed()) { return false; }
  StringPiece protocol =
      GetRequest().GetEnvironments().Get("REQUEST_PROTOCOL"_a);
  if (protocol == "HTTP/1.1"_a) {
    return !CaseInsensitiveEqual(
        GetRequest().GetHeader("Connection"_a), "close"_a);
  } else if (protocol == "HTTP/1.0"_a) {
    return CaseInsensitiveEqual(
        GetRequest().GetHeader("Connection"_a), "keep-alive"_a);
  }
  LOG_FIRST_N(WARNING, 10) << "Unknown protocol: " << protocol;
  return false;
}

Json Stream::ToJson() const {
  map<string, Json> json_object;
  json_object["query_parameters"] = GetQueryParameters().ToJson();
  json_object["post_parameters"] = GetPostParameters().ToJson();
  json_object["cookie_parameters"] = GetCookieParameters().ToJson();
  json_object["file_parameters"] = GetFileParameters();
  json_object["request"] = request_.ToJson();
  json_object["response"] = response_.ToJson();
  return json_object;
}

string Stream::DebugString() const {
  return JsonEncode(ToJson(), true);
}

namespace {

template<typename Delimiter>
bool DecodeQueryMap(
    StringPiece data, Delimiter delimiter,
    Arena* arena, HttpParameters* parameters) {
  DCHECK(parameters->IsEmpty());
  while (true) {
    StringPiece key_and_value;
    StringPiece trailing;
    if (!SplitToStringPieces(data, delimiter, &key_and_value, &trailing)) {
      key_and_value = data;
    }

    StringPiece key;
    StringPiece value;
    if (!SplitToStringPieces(key_and_value, '=', &key, &value)) {
      return true;
    }
    ArenaBuffer* buffer = arena->NextBuffer();
    UrlDecode(key, buffer);
    ArenaStringPiece decoded_key = *buffer;
    buffer = arena->NextBuffer();
    UrlDecode(value, buffer);
    ArenaStringPiece decoded_value = *buffer;
    parameters->Add(decoded_key, decoded_value);

    if (trailing.empty()) return true;
    data = trailing;
  }
}

}  // namespace

bool Stream::ParseParameters() {
  DVLOG(5) << "Stream::ParseParameters is starting...";
  ArenaStringPiece query_string =
      request_.GetEnvironments().Get("QUERY_STRING"_a);
  if (!query_string.empty()) {
    DecodeQueryMap(query_string, '&', &arena_, &get_parameters_);
  }

  ArenaStringPiece cookie_string = request_.GetHeader("Cookie"_a);
  if (!cookie_string.empty()) {
    DecodeQueryMap(cookie_string, "; "_a, &arena_, &cookie_parameters_);
  }

  ArenaStringPiece session_data = cookie_parameters_.Get("session_data"_a);
  if (!session_data.empty()) {
    string error;
    session_parameters_ = Json::parse(Base64::Decode(session_data), error);
    if (!error.empty()) {
      VLOG(1) << "Invalid session: " << session_data;
    }
  }

  if (GetRequest().GetFirstContent().empty()) {
    DVLOG(5) << "Contents is empty.";
    return true;
  }

  StringPiece content_type_buffer = GetRequest().GetHeader("Content-Type"_a);
  StringPiece content_type;
  SplitToStringPieces(content_type_buffer, ';', &content_type, nullptr);
  if (content_type != "multipart/form-data"_a) {
    if (DecodeQueryMap(
            GetRequest().GetFirstContent(), '&', &arena_, &post_parameters_)) {
      return true;
    }
    VLOG(1) << "Failed to parse POST: " << GetRequest().GetFirstContent();
    return false;
  }

  StringPiece boundary;
  for (StringPiece data = content_type_buffer; !data.empty();) {
    StringPiece part;
    StringPiece trailing;
    SplitToStringPieces(data, "; "_a, &part, &trailing);
    if (HasPrefixString(part, "boundary="_a)) {
      boundary = part.substr("boundary="_a.size());
      break;
    }
    data = trailing;
  }
  if (boundary.empty()) {
    VLOG(1) << "No boundary: " << content_type_buffer;
    return false;
  }
  if (boundary.size() > 2 &&
      boundary[0] == '"' && boundary[boundary.size() - 1] == '"') {
    boundary.remove_prefix(1);
    boundary.remove_suffix(1);
  }

  ArenaStringPiece files = GetRequest().GetFirstContent();

  ArenaBuffer* buffer = arena_.NextBuffer();
  buffer->append("\r\n--"_a);
  buffer->append(boundary);
  buffer->append("--\r\n"_a);
  StringPiece separator = *buffer;
  if (!HasSuffixString(files, separator)) {
    VLOG(1) << "Content must end with boundary, but "
            << files.substr(max(files.size() - 100, 0LL));
    return false;
  }
  files.remove_suffix(separator.size());
  buffer->resize(buffer->size() - 4);
  buffer->append("\r\n"_a);
  separator = *buffer;
  if (!HasPrefixString(files, separator.substr(2))) {
    VLOG(1) << "Content must start with boundary, but " << files;
    return false;
  }
  files.remove_prefix(separator.size() - 2);

  while (!files.empty()) {
    ArenaStringPiece file;
    ArenaStringPiece trailing;
    SplitToArenaStringPieces(files, separator, &file, &trailing);
    ArenaStringPiece headers;
    ArenaStringPiece body;
    if (!SplitToArenaStringPieces(file, "\r\n\r\n"_a, &headers, &body)) {
      VLOG(1) << "Content must have header and body: " << file;
      return false;
    }
    std::unique_ptr<HttpParameters> file_parameter(new HttpParameters);
    while (!headers.empty()) {
      ArenaStringPiece header;
      ArenaStringPiece trailing;
      SplitToArenaStringPieces(headers, "\r\n"_a, &header, &trailing);
      ArenaStringPiece key;
      ArenaStringPiece value;
      if (!SplitToArenaStringPieces(header, ": "_a, &key, &value)) {
        VLOG(1) << "Invalid header: " << header;
        return false;
      }
      file_parameter->Set(key, value);
      headers = trailing;
    }
    file_parameter->Set("Content-Body"_a, body);
    StringPiece name;
    StringPiece content_disposition =
        file_parameter->Get("Content-Disposition"_a);
    while (!content_disposition.empty()) {
      StringPiece key, value, trailing;
      SplitHeader(content_disposition, &key, &value, &trailing);
      if (key == "name") {
        name = value;
        break;
      }
      content_disposition = trailing;
    }
    file_parameters_.emplace(ArenaStringPiece(name), std::move(file_parameter));
    files = trailing;
  }

  DVLOG(5) << "Stream::ParseParameters is done.";
  return true;
}
