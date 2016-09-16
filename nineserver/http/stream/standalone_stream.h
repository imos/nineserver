#pragma once
#include <atomic>
#include <map>
#include <string>

#include "base/base.h"
#include "nineserver/arena/arena.h"
#include "nineserver/http/http_contents.h"
#include "nineserver/http/http_parameters.h"
#include "nineserver/http/stream/http_stream.h"
#include "nineserver/http/url.h"
#include "nineserver/io/descriptor_io.h"

class StandaloneStream : public Stream {
 public:
  ~StandaloneStream() override {};

  static StandaloneStream* New(int index);

  static void StartThreads(const std::function<void(Stream*)>& action,
                           vector<std::thread>* threads);


  const GenericIO& GetIO() const override { return io_; }
  GenericIO* MutableIO() override { return &io_; }

  bool Init() override;
  bool Read() override;
  void Destroy() override;
  bool MayRelease() override;

 private:
  explicit StandaloneStream(int index) : index_(index) {}

  static std::atomic<int> keep_alive_count_;

  int index_ = 0;
  DescriptorIO io_;
};
