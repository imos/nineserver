#include "nineserver/hash.h"

#include <openssl/evp.h>

#include "base/base.h"
#include "nineserver/arena/arena_buffer.h"

namespace {

char ToHex(int digit) {
  DCHECK_GE(digit, 0);
  DCHECK_LE(digit, 15);
  if (digit < 10) {
    return '0' + digit;
  }
  return 'a' + (digit - 10);
}

void AppendHash(StringPiece hash, bool binary, BufferInterface* buffer) {
  if (binary) {
    buffer->append(hash);
    return;
  }
  for (int i = 0; i < hash.size(); i++) {
    buffer->push_back(ToHex((hash[i] >> 4) & 15));
    buffer->push_back(ToHex(hash[i] & 15));
  }
}

}  // namespace

void Hash(const EVP_MD& md, StringPiece data, bool binary,
          BufferInterface* buffer) {
  char hash[EVP_MAX_MD_SIZE];
  EVP_MD_CTX ctx;
  EVP_MD_CTX_init(&ctx);
  EVP_DigestInit_ex(&ctx, &md, nullptr);
  EVP_DigestUpdate(&ctx, data.data(), data.size());
  unsigned int length;
  EVP_DigestFinal_ex(&ctx, reinterpret_cast<unsigned char*>(hash), &length);
  EVP_MD_CTX_cleanup(&ctx);
  AppendHash(StringPiece(hash, length), binary, buffer);
}

void Md5(StringPiece data, bool binary, BufferInterface* buffer) {
  static const EVP_MD* md = EVP_md5();
  Hash(*md, data, binary, buffer);
}

string Md5(StringPiece data, bool binary) {
  StringBuffer buffer;
  Md5(data, binary, &buffer);
  return std::move(*buffer.get());
}

void Sha1(StringPiece data, bool binary, BufferInterface* buffer) {
  static const EVP_MD* md = EVP_sha1();
  Hash(*md, data, binary, buffer);
}

string Sha1(StringPiece data, bool binary) {
  StringBuffer buffer;
  Sha1(data, binary, &buffer);
  return std::move(*buffer.get());
}

void Sha256(StringPiece data, bool binary, BufferInterface* buffer) {
  static const EVP_MD* md = EVP_sha256();
  Hash(*md, data, binary, buffer);
}

string Sha256(StringPiece data, bool binary) {
  StringBuffer buffer;
  Sha256(data, binary, &buffer);
  return std::move(*buffer.get());
}

void Sha512(StringPiece data, bool binary, BufferInterface* buffer) {
  static const EVP_MD* md = EVP_sha512();
  Hash(*md, data, binary, buffer);
}

string Sha512(StringPiece data, bool binary) {
  StringBuffer buffer;
  Sha512(data, binary, &buffer);
  return std::move(*buffer.get());
}
