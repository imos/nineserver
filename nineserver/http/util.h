#pragma once
#include <time.h>
#include <string>
#include <map>
#include <unordered_map>

#include "base/base.h"
#include "nineserver/arena/arena_string_piece.h"

string ToHttpDateTime(time_t time);
time_t FromHttpDateTime(const string& http_date_time);

const map<StringPiece, string>& GetMimeTypes();
ArenaStringPiece GetMimeType(StringPiece extension);

void SplitHeader(StringPiece header, StringPiece* key, StringPiece* value,
                 StringPiece* trailing);
