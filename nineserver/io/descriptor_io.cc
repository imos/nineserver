#include "nineserver/io/descriptor_io.h"

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "base/base.h"

void DescriptorIO::InitDescriptorIO(StringPiece address) {
  if (address.starts_with("unix://")) {
    InitDescriptorIO(socket(AF_UNIX, SOCK_STREAM, 0));
    if (descriptor_ < 0) {
      LOG(ERROR) << "Failed to create a socket: " << strerror(errno);
      Destroy();
      return;
    }
    address.remove_prefix(strlen("unix://"));
    struct {
      sa_family_t sun_family;
      char sun_path[1000];
    } sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sun_family = AF_UNIX;
    CHECK_LT(address.size(), sizeof(sockaddr.sun_path));
    memcpy(sockaddr.sun_path, address.data(), address.size());
    sockaddr.sun_path[address.size()] = 0;
    if (connect(descriptor_, (struct sockaddr*)&sockaddr,
                sizeof(sockaddr)) < 0) {
      LOG_FIRST_N(WARNING, 100)
          << "Failed to connect to: unix://" << sockaddr.sun_path
          << ": " << strerror(errno);
      Destroy();
      return;
    }
  } else if (address.starts_with("tcp://")) {
    InitDescriptorIO(socket(AF_INET, SOCK_STREAM, 0));
    if (descriptor_ < 0) {
      LOG(ERROR) << "Failed to create a socket: " << strerror(errno);
      Destroy();
      return;
    }
    address.remove_prefix(strlen("tcp://"));
    vector<StringPiece> parts =
        strings::Split(address, strings::delimiter::Limit(":", 1));
    int32 port;
    if (parts.size() != 2 ||
        !safe_strto32(parts[1].data(), parts[1].size(), &port)) {
      LOG(ERROR) << "Failed to parse: " << address;
      Destroy();
      return;
    }
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(parts[0].ToString().c_str());
    if (connect(descriptor_, (struct sockaddr*)&sockaddr,
                sizeof(sockaddr)) < 0) {
      LOG_FIRST_N(WARNING, 100) << "Failed to connect to: tcp://" << address;
      Destroy();
      return;
    }
  } else {
    LOG(ERROR) << "Failed to parse address: " << address;
    Destroy();
  }
}

int DescriptorIO::ReadIO(char* buffer, int length) {
  if (descriptor_ < 0) { return -1; }
  return read(descriptor_, buffer, length);
}

bool DescriptorIO::WriteIO(const vector<StringPiece>& data) {
  if (IsDestroyed()) {
    return false;
  }

  write_pointers_.resize(data.size());
  int source_length = 0;
  for (int i = 0; i < data.size(); i++) {
    write_pointers_[i].iov_base =
        static_cast<void*>(const_cast<char*>(data[i].data()));
    write_pointers_[i].iov_len = data[i].size();
    source_length += data[i].size();
  }
  
  int length = writev(
      descriptor_, write_pointers_.data(), write_pointers_.size());
  if (length < 0) {
    VLOG(1) << "Failed to write " << source_length << " bytes data: "
                 << strerror(errno);
    Destroy();
    return false;
  }
  return true;
}

void DescriptorIO::CloseIO() {
  if (descriptor_ >= 0) {
    DVLOG(5) << "DescriptorIO::CloseIO()";
    close(descriptor_);
    descriptor_ = -1;
  }
}
