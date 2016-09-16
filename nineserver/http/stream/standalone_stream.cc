#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "base/base.h"
#include "nineserver/http/stream/http_stream.h"
#include "nineserver/http/stream/standalone_stream.h"
#include "nineserver/signal.h"

#define CHECK_SYS(...) CHECK((__VA_ARGS__) >= 0) << strerror(errno)

DEFINE_int32(port, 0, "Port number to listen.");
DEFINE_int32(connection, 1000, "The maximum number of connections to listen.");
DEFINE_int32(threads, 50, "The number of threads for HTTP.");
DEFINE_double(keep_alive_ratio, 0.5, "Ratio for keep-alive.");
DEFINE_int32(socket_timeout_in_ms, 0, "Read timeout in ms.");
DEFINE_bool(descriptor_pool, true, "Use descriptor pool.");

namespace {

const int kPollEvent = POLLIN | POLLPRI | POLLERR | POLLHUP;

class DescriptorPool {
 public:
  DescriptorPool() {
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    CHECK_SYS(listen_fd);
    AddExitHandler([listen_fd]() { close(listen_fd); });

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(FLAGS_port);
    int enable = 1;
    CHECK_SYS(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                         &enable, sizeof(enable)));
#ifdef TCP_DEFER_ACCEPT
    CHECK_SYS(setsockopt(listen_fd, IPPROTO_TCP, TCP_DEFER_ACCEPT,
                         &enable, sizeof(enable)));
#endif
    CHECK_SYS(::bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr)));
    CHECK_SYS(listen(listen_fd, FLAGS_connection));
    LOG(INFO) << "Listening :" << ntohs(server_addr.sin_port) << "...";

    pollfds_.push_back(
        {.fd = listen_fd, .events = kPollEvent, .revents = 0});
  }

  static DescriptorPool* Get() {
    static DescriptorPool* descriptor_pool = new DescriptorPool();
    return descriptor_pool;
  }

  void AddDescriptor(int descriptor) {
    if (descriptor < 0) return;
    std::lock_guard<std::mutex> pool_lock(pool_mutex_);
    pending_descriptors_.push_back(descriptor);
  }

  int GetDescriptor(int index) {
    if (!FLAGS_descriptor_pool || index % 2 == 0) {
      return accept(pollfds_[0].fd, nullptr, nullptr);
    }
    std::lock_guard<std::mutex> pollfds_guard(pollfds_mutex_);
    while (true) {
      if (!ready_descriptors_.empty()) {
        int descriptor = ready_descriptors_.back();
        ready_descriptors_.pop_back();
        return descriptor;
      }
      {
        std::lock_guard<std::mutex> pool_lock(pool_mutex_);
        for (int descriptor : pending_descriptors_) {
          pollfds_.push_back(
              {.fd = descriptor, .events = kPollEvent, .revents = 0});
        }
        pending_descriptors_.clear();
      }
      if (poll(pollfds_.data() + 1, pollfds_.size() - 1, 10) == 0) {
        continue;
      }
      for (int i = 1; i < pollfds_.size(); i++) {
        if (pollfds_[i].revents == 0) continue;
        if ((pollfds_[i].revents & POLLIN) != 0) {
          int descriptor = pollfds_[i].fd;
          if (i == 0) {
            descriptor = accept(descriptor, nullptr, nullptr);
            CHECK_SYS(descriptor);
          }
          ready_descriptors_.push_back(descriptor);
        } else if (i != 0 && close(pollfds_[i].fd)) {
          VLOG(1) << "Close error: " << strerror(errno);
        }
        if (i != 0) {
          swap(pollfds_[i], pollfds_.back());
          pollfds_.pop_back();
          i--;
        }
      }
    }
  }

 private:
  std::mutex pool_mutex_;
  vector<int> ready_descriptors_;
  vector<int> pending_descriptors_;

  std::mutex pollfds_mutex_;
  vector<struct pollfd> pollfds_;
};

}  // namespace

// static
StandaloneStream* StandaloneStream::New(int index) {
  DescriptorPool::Get();
  return new StandaloneStream(index);
}

// static
void StandaloneStream::StartThreads(
    const std::function<void(Stream*)>& action,
    vector<std::thread>* threads) {
  for (int i = 0; i < FLAGS_threads; i++) {
    threads->push_back(std::thread([action, i]() {
      std::unique_ptr<StandaloneStream> stream(StandaloneStream::New(i));
      action(stream.get());
      stream->Destroy();
    }));
  }
}

bool StandaloneStream::Init() {
  DCHECK(!GetKeepAlive());
  if (!Stream::Init()) {
    Destroy();
    return false;
  }

  int fd = DescriptorPool::Get()->GetDescriptor(index_);
  DVLOG(5) << "Descriptor " << fd << ".";

  static const int enable = 1;
  CHECK_SYS(setsockopt(
      fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable)));

  if (FLAGS_socket_timeout_in_ms > 0) {
    struct timeval tv;
    tv.tv_sec = FLAGS_socket_timeout_in_ms / 1000;
    tv.tv_usec = FLAGS_socket_timeout_in_ms % 1000 * 1000;
    CHECK_SYS(setsockopt(
        fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)));
  }

  io_.InitDescriptorIO(fd);
  if (keep_alive_count_ < FLAGS_threads * FLAGS_keep_alive_ratio) {
    SetKeepAlive();
    keep_alive_count_++;
  } else {
    LOG_FIRST_N(WARNING, 1)
        << "Exceed # of keep alive connections: " << keep_alive_count_;
  }
  return true;
}

bool StandaloneStream::Read() {
  if (!Stream::Read()) { return false; }

  if (GetRequest().GetHeader("Expect"_a) == "100-continue"_a) {
    MutableIO()->Write("HTTP/1.1 100 Continue\r\n\r\n"_a);
  }
  return true;
}

bool StandaloneStream::MayRelease() {
  if (!FLAGS_descriptor_pool) {
    return false;
  }

  if (!io_.Empty()) { return false; }
  struct pollfd fd =
      {.fd = io_.GetDescriptor(), .events = kPollEvent, .revents = 0};
  if (poll(&fd, 1, 50) > 0) {
    return false;
  }

  int descriptor = io_.ReleaseDescriptor();
  DescriptorPool::Get()->AddDescriptor(descriptor);
  Destroy();
  return true;
}

void StandaloneStream::Destroy() {
  if (GetKeepAlive()) {
    SetKeepAlive(false);
    keep_alive_count_--;
  }
  Stream::Destroy();
}

std::atomic<int> StandaloneStream::keep_alive_count_(0);
