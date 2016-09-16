#include "base/base.h"
#include "nineserver/handler/base_handler.h"
#include "nineserver/http/proxy.h"

DEFINE_string(proxy, "tcp://127.0.0.1:8080", "Address to proxy request.");
DEFINE_string(proxy_host, "localhost:8080", "Host name for proxy.");

namespace {

class Handler : public BaseHandler {
 public:
  Handler() {
    proxy_.Init(FLAGS_proxy);
  }
  ~Handler() override {}

  bool Run() override {
    MutableRequest()->SetHeader(
        "Host"_a, ArenaStringPiece(StringPiece(FLAGS_proxy_host)));
    return proxy_.Send(MutableRequest(), MutableResponse());
  }

 private:
  Proxy proxy_;
};

class HandlerFactory : public BaseHandlerFactory {
 public:
  HandlerFactory() {}
  ~HandlerFactory() override {}

  BaseHandler* NewHandler() override { return new BaseHandler(); }
};

}  // namespace

int HandlerMain(int argc, char **argv) {
  HandlerFactory handler_factory;
  handler_factory.Run();
  return 0;
}
