licenses(["notice"])

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "data",
    srcs = glob(["data/**"]),
)

filegroup(
    name = "config",
    srcs = glob(["config/**"]),
)

cc_library(
    name = "embed",
    srcs = ["embed_data.cc"],
    deps = [
        "//external:base",
        "//nineserver/handler:resource_pool",
    ],
    alwayslink = 1,
)

genrule(
    name = "embed_data",
    srcs = [":data"],
    outs = ["embed_data.cc"],
    tools = ["//nineserver/tools:embed"],
    cmd = "$(location //nineserver/tools:embed) $(locations :data) > $(OUTS)",
)

cc_library(
    name = "library",
    srcs = glob(["*.cc"], exclude=["main.cc"]),
    hdrs = glob(["*.h"]),
    deps = [
        ":embed",
        "//external:base",
        "//nineserver:util",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "main",
    srcs = ["//nineserver:main"],
    deps = [
        ":library",
        "//external:base",
    ],
)

sh_binary(
    name = "run",
    srcs = ["run.sh"],
    data = [
        ":config",
        ":data",
        ":main",
        "//bin:imosh",
    ],
)

sh_test(
    name = "benchmark_test",
    srcs = ["benchmark_test.sh"],
    data = [
        ":main",
        "//nineserver/tools:free_port",
    ],
    tags = ["manual"],
)
