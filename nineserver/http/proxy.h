#pragma once
#include <string>

#include "base/base.h"
#include "nineserver/common.h"
#include "nineserver/http/http_contents.h"
#include "nineserver/io/descriptor_io.h"

class Proxy {
 public:
  Proxy();
  ~Proxy();

  void Init(StringPiece address);

  bool Send(HttpContents* request, HttpContents* response);

 private:
  string address_;
  DescriptorIO io_;
};
