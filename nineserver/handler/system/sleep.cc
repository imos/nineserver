#include "nineserver/handler/system/system.h"

bool SleepHandler(BaseHandler* handler) {
  handler->MutableResponse()->SetHeader(
      "Content-Type"_a, "text/plain; charset=UTF-8"_a);
  handler->MutableResponse()->Print("OK\n"_a);
  sleep(1);
  return true;
}
REGISTER_HANDLER(handler, SYSTEM_HANDLER_PREFIX "/sleep$", SleepHandler);
