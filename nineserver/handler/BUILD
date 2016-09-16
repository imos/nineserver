licenses(["notice"])

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "data",
    srcs = glob(["data/**"]),
)

cc_library(
    name = "handler",
    deps = [
        ":base_handler",
        "//nineserver/handler/system",
    ],
),

cc_library(
    name = "base_handler",
    srcs = ["base_handler.cc"],
    hdrs = ["base_handler.h"],
    deps = [
        ":embed",
        ":logging",
        ":resource_pool",
        "//external:base",
        "//nineserver/arena:arena_interface",
        "//nineserver/http/stream:http_stream",
        "//nineserver/http/stream:standalone_stream",
        "//nineserver/http:session",
        "//nineserver/http:util",
        "//nineserver:common",
        "//nineserver:signal",
    ],
)

cc_test(
    name = "base_handler_test",
    srcs = ["base_handler_test.cc"],
    deps = [
        ":base_handler",
        "//external:testing_main",
        "//nineserver/benchmark",
        "//nineserver/handler/system",
        "//nineserver/http/stream:string_stream",
    ],
    tags = ["benchmark"],
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
    name = "logging",
    srcs = ["logging.cc"],
    hdrs = ["logging.h"],
    deps = [
        "//external:base",
    ],
)

cc_library(
    name = "resource_pool",
    srcs = ["resource_pool.cc"],
    hdrs = ["resource_pool.h"],
    deps = [
        "//external:base",
        "//nineserver/http/stream:http_stream",
    ],
)
