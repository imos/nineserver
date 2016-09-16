#pragma once
#include <sys/uio.h>
#include <unistd.h>
#include <string>

#include "base/base.h"
#include "nineserver/common.h"
#include "nineserver/io/generic_io.h"

class DescriptorIO : public GenericIO {
 public:
  DescriptorIO() {}
  ~DescriptorIO() override { Destroy(); }

  void InitDescriptorIO(StringPiece address);

  void InitDescriptorIO(int descriptor) {
    descriptor_ = descriptor;
    InitializeVectorWithCapacity(&write_pointers_, 1, 1024);
    Init();
  }

  int GetDescriptor() const { return descriptor_; }

  int ReleaseDescriptor() {
    DCHECK(Empty()) << "Buffer must be empty to release descriptor.";
    int released_descriptor = descriptor_;
    descriptor_ = -1;
    Destroy();
    return released_descriptor;
  }

  int ReadIO(char* buffer, int length) override;
  bool WriteIO(const vector<StringPiece>& data) override;
  void CloseIO() override;

 private:
  int descriptor_ = -1;
  vector<struct iovec> write_pointers_;

  DISALLOW_COPY_AND_ASSIGN(DescriptorIO);
};
