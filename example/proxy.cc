#include "handler.h"

bool ProxyHandler(Handler* handler) {
  Proxy proxy;
  proxy.Init("tcp://216.239.34.21:80");  // imoz.jp
  StringPiece header = handler->MutableRequest()->GetHeader(""_a);
  StringPiece method, uri;
  SplitToStringPieces(header, ' ', &method, &uri, nullptr);
  if (uri.starts_with("/proxy"_a)) {
    uri.remove_prefix("/proxy"_a.size());
  }
  handler->MutableRequest()->SetHeader(
      ""_a, StrCat(method, " ", uri, " HTTP/1.1"));
  handler->MutableRequest()->SetHeader("Host"_a, "imoz.jp"_a);
  handler->MutableRequest()->SetHeader("Connection"_a, "close"_a);
  if (!proxy.Send(handler->MutableRequest(), handler->MutableResponse())) {
    handler->SetHeader(""_a, "HTTP/1.1 404 Not Found"_a);
    handler->SetHeader("Content-Type"_a, "text/html; charset=UTF8"_a);
    handler->Print("Not found");
    return true;
  }
  return true;
}
REGISTER_HANDLER(handler, "/proxy/", ProxyHandler);
