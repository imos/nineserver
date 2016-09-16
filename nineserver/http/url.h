#pragma once
#include <string>

#include "base/base.h"
#include "nineserver/arena/arena_buffer.h"

string UrlEncode(StringPiece value);
// TODO(P2): Deprecate this function.
string UrlDecode(StringPiece url);

// Decodes URL-encoded string.  Decodes any %## encoding in the given string.
// Plus symbols ('+') are decoded to a space character.
void UrlDecode(StringPiece url, BufferInterface* buffer);

void UrlEncode(StringPiece data, BufferInterface* buffer);

map<string, string> QueryDecode(StringPiece query);
