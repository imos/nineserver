// Only one HandlerFactory is created, and one Handler is created per thread.
// Global objects must be passed from HandlerFactory, and thread-local data
// (e.g. cache) should be stored in Handler.  Handler and HandlerFactory are
// reused and not destructed while server is running.
#pragma once
#include "base/base.h"
#include "nineserver/util.h"

class Handler : public BaseHandler {
 public:
  Handler();
  ~Handler() override {}

  // Called when a request is ready to process.
  bool Run() override;

 private:
  Proxy proxy_;
};

class HandlerFactory : public BaseHandlerFactory {
 public:
  HandlerFactory() {}
  ~HandlerFactory() override {}

  BaseHandler* NewHandler() override { return new Handler(); }
};

int HandlerMain(int argc, char** argv);
