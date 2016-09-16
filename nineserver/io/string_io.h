#pragma once
#include <string>

#include "base/base.h"
#include "nineserver/arena/arena.h"
#include "nineserver/io/generic_io.h"

class StringIO : public GenericIO {
 public:
  explicit StringIO() {}
  ~StringIO() override { Destroy(); }

  void InitStringIO(ArenaStringPiece input) {
    position_ = 0;
    input_ = input;
    output_.clear();
    Init();
  }

  void InitStringIO(StringPiece input) {
    input_string_ = input.ToString();
    InitStringIO(ArenaStringPiece(StringPiece(input_string_)));
  }

  StringPiece input() const { return input_; }
  const string& output() const { return output_; }

  int ReadIO(char* buffer, int length) override;
  bool WriteIO(const vector<StringPiece>& data) override;
  void CloseIO() override;

 private:
  int position_;
  StringPiece input_;
  string input_string_;
  string output_;

  DISALLOW_COPY_AND_ASSIGN(StringIO);
};
