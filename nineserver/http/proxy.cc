#include "nineserver/http/proxy.h"

Proxy::Proxy() {}
Proxy::~Proxy() {}

void Proxy::Init(StringPiece address) {
  address_ = address.ToString();
}

bool Proxy::Send(HttpContents* request, HttpContents* response) {
  for (int fallback = 0; fallback < 2; fallback++) {
    response->Clear();
    if (io_.IsDestroyed()) {
      io_.InitDescriptorIO(address_);
    }
    if (!request->Write(&io_) ||
        request->IsDestroyed() || !response->Read(&io_)) {
      io_.Destroy();
      continue;
    }
    return true;
  }
  return false;
}
