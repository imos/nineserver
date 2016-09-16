#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <algorithm>
#include <string>
#include <vector>

#include "base/base.h"

#define CHECK_SYS(...) CHECK((__VA_ARGS__) >= 0) << strerror(errno)

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  int fd = socket(PF_INET, SOCK_STREAM, 0);
  CHECK_SYS(fd);

  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(0);
  int enable = 1;
  CHECK_SYS(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)));
  CHECK_SYS(::bind(fd, (sockaddr*)&server_addr, sizeof(server_addr)));
  socklen_t addrlen = sizeof(server_addr);
  CHECK_SYS(getsockname(fd, (sockaddr*)&server_addr, &addrlen));
  printf("%d\n", ntohs(server_addr.sin_port));
  return 0;
}
