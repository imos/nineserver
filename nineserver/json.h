#pragma once
#include <string>
#include <type_traits>

#define PICOJSON_USE_INT64
#include "base/base.h"
#include "nineserver/common.h"
#include "nineserver/json11/json11.hpp"

using Json = json11::Json;

template<class T>
string JsonEncode(const T& value, bool prettify = false) {
  return Json(value).dump(prettify ? 0 : -1);
}
