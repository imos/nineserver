#include <signal.h>

#include "base/base.h"

int HandlerMain(int argc, char **argv);

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  signal(SIGPIPE, SIG_IGN);
  return HandlerMain(argc, argv);
}
