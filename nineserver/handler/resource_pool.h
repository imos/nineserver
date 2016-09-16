#pragma once
#include <functional>
#include <map>

#include "base/base.h"
#include "nineserver/arena/arena_string_piece.h"
#include "nineserver/http/stream/http_stream.h"
#include "third_party/glog/base/googleinit.h"

class ResourcePool {
 public:
  static void Register(
      const map<StringPiece, StringPiece>& additional_resources);

  static const StringPiece* GetOrNull(StringPiece key);

  static const map<StringPiece, StringPiece>& Get();
};

#define REGISTER_RESOURCE(body) \
    REGISTER_MODULE_INITIALIZER(resource, ResourcePool::Register(body));
