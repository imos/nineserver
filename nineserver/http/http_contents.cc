#include "nineserver/arena/arena_buffer.h"
#include "nineserver/arena/arena_string_piece.h"
#include "nineserver/common.h"
#include "nineserver/http/http_contents.h"

void HttpContents::Init(Arena* arena) {
  DCHECK(arena != nullptr);
  arena_ = arena;
  Clear();
}

void HttpContents::Clear() {
  headers_.Clear();
  environments_.Clear();
  InitializeVectorWithCapacity(&output_buffer_, 0, 4096 / 16);
  InitializeVectorWithCapacity(&contents_, 0, 4096 / 16);
}

namespace {

bool ReadHeaderBlock(GenericIO* io, ArenaBuffer* header_buffer) {
  for (header_buffer->resize(0); io->ReadLine(header_buffer);) {
    if (HasSuffixString(*header_buffer, "\r\n\r\n"_a)) {
      return true;
    }
    DLOG_IF(ERROR, !HasSuffixString(*header_buffer, "\r\n"_a))
        << "Header must terminate with CRLF.";
  }
  if (*header_buffer == ""_a) {
    DVLOG(5) << "Closed buffer.";
    return false;
  }
  VLOG(1) << "Invalid read buffer: " << *header_buffer;
  return false;
}

}  // namespace

bool HttpContents::ReadHeader(GenericIO* io) {
  DCHECK(arena_ != nullptr);
  DCHECK(headers_.IsEmpty());

  ArenaBuffer* header_buffer = arena_->NextBuffer();
  if (!ReadHeaderBlock(io, header_buffer)) { return false; }
  StringPiece headers = *header_buffer;
  for (size_t i = 0; i < headers.size();) {
    StringPiece buffer(
        headers.data() + i, headers.size() - i);
    auto newline_position = buffer.find("\r\n"_a);
    if (newline_position == StringPiece::npos) {
      break;
    }
    StringPiece header = buffer.substr(0, newline_position);
    if (header.empty()) break;
    auto colon_position = header.find(": "_a);
    if (colon_position == StringPiece::npos) {
      SetHeader(""_a, ArenaStringPiece(header));
    } else {
      for (int j = 0; j < colon_position; j++) {
        (*header_buffer)[i + j] = (j == 0 || header[j - 1] == '-')
            ? toupper(header[j]) : tolower(header[j]);
      }
      StringPiece key = StringPiece(header.data(), colon_position);
      StringPiece value = header.substr(colon_position + 2);
      SetHeader(ArenaStringPiece(key), ArenaStringPiece(value));
    }
    i += newline_position + 2;
  }
  return true;
}

bool HttpContents::ReadContent(GenericIO* io) {
  DCHECK(arena_ != nullptr);

  StringPiece content_length_buffer = GetHeader(kContentLengthHeader);
  if (!content_length_buffer.empty()) {
    int32 content_length = 0;
    if (!safe_strto32(content_length_buffer.data(),
                      content_length_buffer.size(), &content_length)) {
      VLOG(1) << "Invalid content length: " << content_length_buffer;
      return false;
    }
    if (content_length == 0) { return true; }
    ArenaBuffer* content_buffer = arena_->NextBuffer();
    content_buffer->reserve(content_length);
    if (io->ReadN(content_length, content_buffer) != content_length) {
      VLOG(1) << "Invalid content length: " << content_length;
      return false;
    }
    contents_.push_back(*content_buffer);
    DVLOG(5) << "ReadContent is done.";
    return true;
  }

  StringPiece transfer_encoding_buffer = GetHeader(kTransferEncodingHeader);
  if (!CaseInsensitiveEqual(transfer_encoding_buffer, "chunked"_a)) {
    if (CaseInsensitiveEqual(GetHeader("Connection"_a), "keep-alive"_a)) {
      return true;
    }
    ArenaBuffer* content_buffer = arena_->NextBuffer();
    while (io->ReadN(128 * 1024, content_buffer) > 0);
    contents_.push_back(*content_buffer);
    return true;
  }

  StringBuffer chunk_length_buffer;
  chunk_length_buffer.get()->reserve(10);
  ArenaBuffer* content_buffer = arena_->NextBuffer();
  for (; io->ReadLine(&chunk_length_buffer);
         chunk_length_buffer.get()->resize(0)) {
    if (!HasSuffixString(chunk_length_buffer, "\r\n")) {
      return false;
    }
    // TODO(P3): Support chunk-extension.
    uint32 chunk_length = 0;
    if (!safe_strtou32_base(chunk_length_buffer, &chunk_length, 16)) {
      VLOG(1) << "Invalid chunk length: " << chunk_length_buffer;
      return false;
    }
    if (io->ReadN(chunk_length, content_buffer) != chunk_length) {
      VLOG(1) << "Invalid chunk length: " << chunk_length;
      return false;
    }
    if (io->ReadCharacter() != '\r' || io->ReadCharacter() != '\n') {
      VLOG(1) << "Invalid chunk termination.";
      return false;
    }
    if (chunk_length == 0) {
      contents_.push_back(ArenaStringPiece(StringPiece(*content_buffer)));
      return true;
    }
  }
  return false;
}

bool HttpContents::Read(GenericIO* io, bool ignore_content) {
  DVLOG(5) << "HttpContents::Read(" << io << ", " << ignore_content << ")";
  DCHECK(arena_ != nullptr);
  DCHECK(environments_.IsEmpty());

  if (!ReadHeader(io)) { return false; }
  if (ignore_content) {
    return true;
  }

  ArenaStringPiece primary_header = GetHeader(""_a);
  // For a HTTP request.
  if (!HasPrefixString(primary_header, "HTTP/"_a)) {
    ArenaStringPiece method, uri, protocol;
    if (!SplitToArenaStringPieces(
            primary_header, ' ', &method, &uri, &protocol)) {
      VLOG(1) << "Invalid primary header: " << primary_header;
      return false;
    }

    DVLOG(5) << "Request URI: " << uri;
    environments_.Set("REQUEST_METHOD"_a, method);
    environments_.Set("REQUEST_URI"_a, uri);
    environments_.Set("REQUEST_PROTOCOL"_a, protocol);
    ArenaStringPiece script_name, query_string;
    if (SplitToArenaStringPieces(uri, '?', &script_name, &query_string)) {
      environments_.Set("SCRIPT_NAME"_a, script_name);
      environments_.Set("QUERY_STRING"_a, query_string);
    } else {
      environments_.Set("SCRIPT_NAME"_a, uri);
    }

    if (DVLOG_IS_ON(3)) {
      for (const auto& key_and_value : environments_) {
        LOG(INFO) << key_and_value.first << " => " << key_and_value.second;
      }
    }

    // Early skip reading contents.
    if (method == "GET"_a || method == "HEAD"_a) {
      return true;
    }
  }

  return ReadContent(io);
}

bool HttpContents::Write(GenericIO* io, StringPiece hostname) {
  DVLOG(5) << "HttpContents::Write(" << io << ", \"" << hostname << "\")";
  DCHECK(arena_ != nullptr);

  StringPiece primary_header = GetHeader(""_a);
  if (primary_header.empty()) {
    VLOG(1) << "Primary header is empty";
    return false;
  }
  output_buffer_.resize(0);
  output_buffer_.push_back(primary_header);
  output_buffer_.push_back("\r\n"_a);

  for (const auto& header : headers_) {
    if (header.first.empty()) continue;
    if (header.first == "Content-Length"_a) continue;
    if (!hostname.empty() && header.first == "Host"_a) continue;
    // TODO(P3): Make this case-insensitive.
    if (header.first == "Transfer-Encoding"_a &&
        header.second == "chunked"_a) continue;
    output_buffer_.push_back(header.first);
    output_buffer_.push_back(": "_a);
    output_buffer_.push_back(header.second);
    output_buffer_.push_back("\r\n"_a);
  }

  if (!hostname.empty()) {
    output_buffer_.push_back("Host: "_a);
    output_buffer_.push_back(hostname);
    output_buffer_.push_back("\r\n"_a);
  }

  int content_length = 0;
  for (StringPiece content : contents_) {
    content_length += content.size();
  }

  output_buffer_.push_back("Content-Length: "_a);
  char content_length_buffer[21];
  sprintf(content_length_buffer, "%d", content_length);
  output_buffer_.push_back(content_length_buffer);
  output_buffer_.push_back("\r\n\r\n"_a);
  DVLOG(5) << "Content-Length: " << content_length_buffer;
  for (StringPiece content : contents_) {
    output_buffer_.push_back(content);
  }

  return io->Write(output_buffer_);
}
