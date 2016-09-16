#pragma once
#include <openssl/evp.h>

#include "base/base.h"
#include "nineserver/arena/arena_buffer.h"

void Md5(StringPiece data, bool binary, BufferInterface* buffer);
string Md5(StringPiece data, bool binary = false);
void Sha1(StringPiece data, bool binary, BufferInterface* buffer);
string Sha1(StringPiece data, bool binary = false);
void Sha256(StringPiece data, bool binary, BufferInterface* buffer);
string Sha256(StringPiece data, bool binary = false);
void Sha512(StringPiece data, bool binary, BufferInterface* buffer);
string Sha512(StringPiece data, bool binary = false);
