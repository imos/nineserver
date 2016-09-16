#include "nineserver/http/util.h"

#include <chrono>

#include "time_zone.h"

string ToHttpDateTime(time_t time) {
  char buffer[100];
  struct tm tm;
  strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT",
           gmtime_r(&time, &tm));
  return buffer;
}

time_t FromHttpDateTime(const string& http_date_time) {
  string formatted_time;
  cctz::time_zone lax;
  if (TryStripSuffixString(http_date_time, " UTC", &formatted_time) ||
      TryStripSuffixString(http_date_time, " GMT", &formatted_time)) {
    load_time_zone("UTC", &lax);
  } else if (TryStripSuffixString(http_date_time, " JST", &formatted_time)) {
    load_time_zone("Asia/Tokyo", &lax);
  } else if (TryStripSuffixString(http_date_time, " PST", &formatted_time) ||
             TryStripSuffixString(http_date_time, " PDT", &formatted_time)) {
    load_time_zone("America/Los_Angeles", &lax);
  } else if (TryStripSuffixString(http_date_time, " EST", &formatted_time) ||
             TryStripSuffixString(http_date_time, " EDT", &formatted_time)) {
    load_time_zone("America/New_York", &lax);
  } else if (TryStripSuffixString(http_date_time, " CST", &formatted_time) ||
             TryStripSuffixString(http_date_time, " CDT", &formatted_time)) {
    load_time_zone("America/Chicago", &lax);
  } else {
    return static_cast<time_t>(0);
  }

  std::chrono::system_clock::time_point tp;
  if (!cctz::parse("%a, %d %b %Y %H:%M:%S", formatted_time, lax, &tp)) {
    return static_cast<time_t>(0);
  }

  return std::chrono::system_clock::to_time_t(tp);
}

const map<StringPiece, string>& GetMimeTypes() {
  static map<StringPiece, string>* mime_types = new map<StringPiece, string>({
      {"html", "text/html"},
      {"htm", "text/html"},
      {"shtml", "text/html"},
      {"css", "text/css"},
      {"xml", "text/xml"},
      {"gif", "image/gif"},
      {"jpeg", "image/jpeg"},
      {"jpg", "image/jpeg"},
      {"js", "application/javascript"},
      {"atom", "application/atom+xml"},
      {"rss", "application/rss+xml"},
      {"mml", "text/mathml"},
      {"txt", "text/plain"},
      {"jad", "text/vnd.sun.j2me.app-descriptor"},
      {"wml", "text/vnd.wap.wml"},
      {"htc", "text/x-component"},
      {"png", "image/png"},
      {"tif", "image/tiff"},
      {"tiff", "image/tiff"},
      {"wbmp", "image/vnd.wap.wbmp"},
      {"ico", "image/x-icon"},
      {"jng", "image/x-jng"},
      {"bmp", "image/x-ms-bmp"},
      {"svgz", "image/svg+xml svg"},
      {"webp", "image/webp"},
      {"woff", "application/font-woff"},
      {"jar", "application/java-archive"},
      {"war", "application/java-archive"},
      {"ear", "application/java-archive"},
      {"json", "application/json"},
      {"hqx", "application/mac-binhex40"},
      {"doc", "application/msword"},
      {"pdf", "application/pdf"},
      {"ps", "application/postscript"},
      {"eps", "application/postscript"},
      {"ai", "application/postscript"},
      {"rtf", "application/rtf"},
      {"m3u8", "application/vnd.apple.mpegurl"},
      {"xls", "application/vnd.ms-excel"},
      {"eot", "application/vnd.ms-fontobject"},
      {"ppt", "application/vnd.ms-powerpoint"},
      {"wmlc", "application/vnd.wap.wmlc"},
      {"kml", "application/vnd.google-earth.kml+xml"},
      {"kmz", "application/vnd.google-earth.kmz"},
      {"7z", "application/x-7z-compressed"},
      {"cco", "application/x-cocoa"},
      {"jardiff", "application/x-java-archive-diff"},
      {"jnlp", "application/x-java-jnlp-file"},
      {"run", "application/x-makeself"},
      {"pl", "application/x-perl"},
      {"pm", "application/x-perl"},
      {"prc", "application/x-pilot"},
      {"pdb", "application/x-pilot"},
      {"rar", "application/x-rar-compressed"},
      {"rpm", "application/x-redhat-package-manager"},
      {"sea", "application/x-sea"},
      {"swf", "application/x-shockwave-flash"},
      {"sit", "application/x-stuffit"},
      {"tcl", "application/x-tcl"},
      {"tk", "application/x-tcl"},
      {"der", "application/x-x509-ca-cert"},
      {"pem", "application/x-x509-ca-cert"},
      {"crt", "application/x-x509-ca-cert"},
      {"xpi", "application/x-xpinstall"},
      {"xhtml", "application/xhtml+xml"},
      {"xspf", "application/xspf+xml"},
      {"zip", "application/zip"},
      {"bin", "application/octet-stream"},
      {"exe", "application/octet-stream"},
      {"dll", "application/octet-stream"},
      {"deb", "application/octet-stream"},
      {"dmg", "application/octet-stream"},
      {"iso", "application/octet-stream"},
      {"img", "application/octet-stream"},
      {"msi", "application/octet-stream"},
      {"msp", "application/octet-stream"},
      {"msm", "application/octet-stream"},
      {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
      {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
      {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
      {"mid", "audio/midi"},
      {"midi", "audio/midi"},
      {"kar", "audio/midi"},
      {"mp3", "audio/mpeg"},
      {"ogg", "audio/ogg"},
      {"m4a", "audio/x-m4a"},
      {"ra", "audio/x-realaudio"},
      {"3gp", "video/3gpp 3gpp"},
      {"ts", "video/mp2t"},
      {"mp4", "video/mp4"},
      {"mpeg", "video/mpeg"},
      {"mpg", "video/mpeg"},
      {"mov", "video/quicktime"},
      {"webm", "video/webm"},
      {"flv", "video/x-flv"},
      {"m4v", "video/x-m4v"},
      {"mng", "video/x-mng"},
      {"asx", "video/x-ms-asf"},
      {"asf", "video/x-ms-asf"},
      {"wmv", "video/x-ms-wmv"},
      {"avi", "video/x-msvideo"},
  });
  return *mime_types;
}

ArenaStringPiece GetMimeType(StringPiece extension) {
  const string* mime_type = FindOrNull(GetMimeTypes(), extension);
  if (mime_type == nullptr) {
    return ArenaStringPiece();
  }
  return ArenaStringPiece(StringPiece(*mime_type));
}

void SplitHeader(StringPiece header, StringPiece* key, StringPiece* value,
                 StringPiece* trailing) {
  if (header.starts_with(" "_a)) {
    header.remove_prefix(1);
  }

  int64 position = 0;
  while (position < header.size()) {
    auto semicolon_position = header.find(';', position);
    if (semicolon_position == StringPiece::npos) {
      position = header.size();
      break;
    }
    auto quote_position =
        header.substr(0, semicolon_position).find('"', position);
    if (quote_position == StringPiece::npos) {
      position = semicolon_position;
      break;
    }
    quote_position = header.find('"', quote_position + 1);
    if (quote_position == StringPiece::npos) {
      position = header.size();
      break;
    }
    position = quote_position + 1;
  }
  StringPiece piece;
  if (position < header.size()) {
    DCHECK_EQ(header[position], ';');
    piece = header.substr(0, position);
    *trailing = header.substr(position + 1);
  } else {
    piece = header;
    *trailing = ""_a;
  }

  if (!SplitToStringPieces(piece, '=', key, value)) {
    *key = ""_a;
    *value = piece;
  }
  if (value->size() >= 2 && *value->begin() == '"' && *value->rbegin() == '"') {
    value->remove_prefix(1);
    value->remove_suffix(1);
  }
}
