#include "handler.h"

namespace {

bool SqlHandler(Handler* handler) {
  Json json = MySQL::SelectAll("SHOW PROCESSLIST");
  handler->Print(JsonEncode(json));
  return true;
}
REGISTER_HANDLER(SqlHandler, "/sql$", SqlHandler);

}  // namespace
