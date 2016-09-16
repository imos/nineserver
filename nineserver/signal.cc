#include "nineserver/signal.h"

#include <mutex>
#include <vector>

#include "base/base.h"

namespace {

std::mutex handler_mutex;
vector<std::function<void()>>* handlers;

}  // namespace

void AddExitHandler(const std::function<void()>& handler) {
  std::lock_guard<std::mutex> lock(handler_mutex);
  if (handlers == nullptr) {
    handlers = new vector<std::function<void()>>();
    handlers->reserve(128);
  }
  handlers->push_back(handler);
}

void RunExitHandlers() {
  vector<std::function<void()>>* exit_handlers;
  {
    std::lock_guard<std::mutex> lock(handler_mutex);
    if (handlers == nullptr) { return; }
    exit_handlers = handlers;
    handlers = nullptr;
  }
  for (auto it = exit_handlers->rbegin(); it != exit_handlers->rend(); it++) {
    (*it)();
  }
}
