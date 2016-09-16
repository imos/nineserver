#include <openssl/evp.h>

#include "nineserver/hash.h"

DEFINE_string(algorithm, "sha256", "Algorithm.");

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  if (argc < 2) {
    fprintf(stderr, "Usage: hash_main --algorithm=<algorithm> string\n");
    OpenSSL_add_all_algorithms();
    exit(1);
  }
  StringBuffer buffer;
  if (FLAGS_algorithm == "md5") {
    Md5(argv[1], false, &buffer);
  } else if (FLAGS_algorithm == "sha1") {
    Sha1(argv[1], false, &buffer);
  } else if (FLAGS_algorithm == "sha256") {
    Sha256(argv[1], false, &buffer);
  } else {
    LOG(FATAL) << "Unknown algorithm: " << FLAGS_algorithm;
  }
  printf("%s\n", buffer.get()->c_str());
  return 0;
}
