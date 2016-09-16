#include "nineserver/handler/resource_pool.h"

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>

#include "base/base.h"

namespace {

map<StringPiece, StringPiece>* resources = nullptr;
std::mutex resources_mutex;

}  // namespace

void ResourcePool::Register(
    const map<StringPiece, StringPiece>& additional_resources) {
  std::lock_guard<std::mutex> resources_guard(resources_mutex);
  if (resources == nullptr) {
    resources = new std::decay<decltype(*resources)>::type();
  }
  for (const auto& key_and_value : additional_resources) {
    (*resources)[key_and_value.first] = key_and_value.second;
  }
}

const StringPiece* ResourcePool::GetOrNull(StringPiece key) {
  return FindOrNull(*resources, key);
}

const map<StringPiece, StringPiece>& ResourcePool::Get() {
  return *resources;
}

// Avoid resources == nullptr.
REGISTER_RESOURCE((map<StringPiece, StringPiece>({})));
