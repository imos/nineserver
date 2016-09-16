#include "nineserver/http/url.h"

namespace {

int FromHex(char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  }
  if ('a' <= c && c <= 'f') {
    return static_cast<int>(c - 'a') + 10;
  }
  if ('A' <= c && c <= 'F') {
    return static_cast<int>(c - 'A') + 10;
  }
  return 0;
}

char ToHex(int digit) {
  DCHECK_GE(digit, 0);
  DCHECK_LE(digit, 15);
  if (digit < 10) {
    return '0' + digit;
  }
  return 'A' + (digit - 10);
}

}  // namespace

string UrlEncode(StringPiece value) {
  string result;
  result.reserve(value.size() * 2);
  for (int i = 0; i < value.size(); i++) {
    // Unresrved characters in RFC 3986.
    if (isalnum(value[i]) || value[i] == '.' || value[i] == '~' ||
        value[i] == '-' || value[i] == '_') {
      result.append(1, value[i]);
    } else {
      result.append(StringPrintf("%%%02x", value[i]));
    }
  }
  return result;
}

string UrlDecode(StringPiece url) {
  StringBuffer buffer;
  UrlDecode(url, &buffer);
  return buffer;
}

void UrlDecode(StringPiece url, BufferInterface* buffer) {
  for (int i = 0; i < url.size(); i++) {
    if (url[i] == '%' && i + 2 < url.size() &&
        isxdigit(url[i + 1]) && isxdigit(url[i + 2])) {
      buffer->push_back(FromHex(url[i + 1]) * 16 + FromHex(url[i + 2]));
      i += 2;
    } else if (url[i] == '+') {
      buffer->push_back(' ');
    } else {
      buffer->push_back(url[i]);
    }
  }
}

void UrlEncode(StringPiece data, BufferInterface* buffer) {
  for (int i = 0; i < data.size(); i++) {
    if (isalnum(data[i]) ||
        data[i] == '-' || data[i] == '_' || data[i] == '.') {
      buffer->push_back(data[i]);
    } else {
      buffer->push_back('%');
      buffer->push_back(ToHex((data[i] >> 4) & 15));
      buffer->push_back(ToHex(data[i] & 15));
    }
  }
}

map<string, string> QueryDecode(StringPiece query) {
  map<string, string> result;
  for (StringPiece parameter : strings::Split(query, "&")) {
    vector<StringPiece> pieces =
        strings::Split(parameter, strings::delimiter::Limit("=", 1));
    if (pieces.size() != 2) continue;
    result.emplace(UrlDecode(pieces[0]), UrlDecode(pieces[1]));
  }
  return result;
}
