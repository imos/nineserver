#pragma once
#include "base/base.h"
#include "nineserver/http/stream/http_stream.h"
#include "nineserver/io/string_io.h"

class StringStream : public Stream {
 public:
  explicit StringStream() {}
  ~StringStream() override {}

  template<typename T>
  void SetInput(T data) { io_.InitStringIO(data); }
  const string& GetOutput();

  const GenericIO& GetIO() const override { return io_; }
  GenericIO* MutableIO() override { return &io_; }

 private:
  StringIO io_;
};
