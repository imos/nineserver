#include "nineserver/io/string_io.h"

#include <vector>

#include "base/base.h"

int StringIO::ReadIO(char* buffer, int length) {
  int position = 0;
  for (; position < length && position_ < input_.size();
       position++, position_++) {
    buffer[position] = input_[position_];
  }
  return position;
}

bool StringIO::WriteIO(const vector<StringPiece>& data) {
  if (IsDestroyed()) { return false; }

  for (StringPiece buffer : data) {
    output_.append(buffer.data(), buffer.size());
  }
  return true;
}

void StringIO::CloseIO() {}
