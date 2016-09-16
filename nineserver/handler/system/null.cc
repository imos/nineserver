#include "nineserver/handler/system/system.h"

bool NullHandler(BaseHandler* handler) {
  handler->MutableResponse()->Print("OK\n"_a);
  handler->MutableResponse()->SetHeader(
      "Content-Type"_a, "text/plain; charset=UTF-8"_a);
  return true;
}
REGISTER_HANDLER(handler, SYSTEM_HANDLER_PREFIX "/null$", NullHandler);
