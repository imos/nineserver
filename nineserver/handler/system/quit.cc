#include "nineserver/handler/system/system.h"

bool QuitHandler(BaseHandler* handler) {
  handler->MutableResponse()->SetHeader(
      "Content-Type"_a, "text/plain; charset=UTF-8"_a);
  handler->MutableResponse()->Print("OK\n"_a);
  handler->Destroy();
  exit(0);
  return true;
}
REGISTER_HANDLER(handler, SYSTEM_HANDLER_PREFIX "/quit$", QuitHandler);
