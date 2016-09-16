#include "base/base.h"
#include "nineserver/arena/arena.h"
#include "nineserver/http/http_contents.h"
#include "nineserver/http/proxy.h"
#include "nineserver/io/descriptor_io.h"

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  Arena arena;
  DescriptorIO stdin;
  stdin.InitDescriptorIO(0);
  HttpContents request;
  request.Init(&arena);
  CHECK(request.Read(&stdin));
  LOG(INFO) << request.ToJson().dump();

  Proxy proxy;
  proxy.Init("tcp://216.239.36.21:80");

  HttpContents response;
  response.Init(&arena);
  proxy.Send(&request, &response);

  DescriptorIO stdout;
  stdout.InitDescriptorIO(1);
  CHECK(response.Write(&stdout));
  return 0;
}
