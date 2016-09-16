// Only one HandlerFactory is created, and one Handler is created per thread.
// Global objects must be passed from HandlerFactory, and thread-local data
// (e.g. cache) should be stored in Handler.  Handler and HandlerFactory are
// reused and not destructed while server is running.
#include "handler.h"

#include "base/base.h"
#include "nineserver/util.h"

DEFINE_string(proxy, "tcp://127.0.0.1:8080", "Address to proxy request.");
DEFINE_string(proxy_host, "localhost:8080", "Host name for proxy.");
DECLARE_int32(port);

Handler::Handler() {
    proxy_.Init(FLAGS_proxy);
}

// Called when a request is ready to process.
bool Handler::Run() {
  MutableRequest()->SetHeader(
      "Host"_a, ArenaStringPiece(StringPiece(FLAGS_proxy_host)));
  MutableRequest()->SetHeader(
      "X-Nineserver-Port"_a, ArenaStrCat(FLAGS_port));
  return proxy_.Send(MutableRequest(), MutableResponse());
}

int HandlerMain(int argc, char** argv) {
  HandlerFactory handler_factory;
  handler_factory.Run();
  return 0;
}
