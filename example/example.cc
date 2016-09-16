#include "handler.h"

struct ExampleData {
  int counter = 0;
};

bool ExampleHandler(Handler* handler) {
  auto session = handler->MutableSession();
  auto lock = session->Lock();
  auto* data = session->Mutable<ExampleData>();
  data->counter++;
  handler->Print("You visited "_a,
                 handler->ArenaStrCat(data->counter), " times!"_a);
  return true;
}
REGISTER_HANDLER(handler, "/example$", ExampleHandler);
