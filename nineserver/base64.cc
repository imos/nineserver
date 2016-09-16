#include "nineserver/base64.h"

void Base64::Encode(StringPiece data, int option, BufferInterface* buffer) {
  constexpr int kShift[] = {10, 4, 6, 8};
  constexpr char kBase64[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  constexpr char kUrlBase64[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
  const char* pattern = (option & URL) ? kUrlBase64 : kBase64;
  int base64_length = (data.size() + 2) / 3 * 4;
  buffer->reserve(buffer->size() + base64_length);
  for (int i = 0; i < base64_length; i++) {
    int index = i - (i + 3) / 4;
    if (index >= data.size()) {
      if (!(option & URL) && (option & PADDING)) { buffer->push_back('='); }
      continue;
    }
    int value = ((((data[index] & 255L) << 8) |
                 ((index + 1 < data.size() ? data[index + 1] : 0) & 255L))
                 >> kShift[i % 4]) & 63;
    DCHECK_LE(0, value);
    DCHECK_GE(63, value);
    buffer->push_back(pattern[value]);
  }
}

void Base64::Decode(StringPiece data, int option, BufferInterface* buffer) {
  constexpr int kShift[] = {0, 4, 2, 0};
  int code = 0;
  buffer->reserve(data.size() * 3 / 4);
  for (int i = 0; i < data.size(); i++) {
    char c = data[i];
    code <<= 6;
    if ('A' <= c && c <= 'Z') {
      code += c - 'A';
    } else if ('a' <= c && c <= 'z') {
      code += c - 'a' + 26;
    } else if ('0' <= c && c <= '9') {
      code += c - '0' + 52;
    } else if (c == '+' || c == '-') {
      code += 62;
    } else if (c == '/' || c == '_') {
      code += 63;
    } else if (c == '=') {
      break;
    }
    if (i % 4 > 0) {
      buffer->push_back(code >> kShift[i % 4]);
    }
  }
}
