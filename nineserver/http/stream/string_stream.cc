#include "base/base.h"
#include "nineserver/http/stream/string_stream.h"

const string& StringStream::GetOutput() { return io_.output(); }
