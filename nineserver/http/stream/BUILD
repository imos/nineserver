licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "http_stream",
    srcs = ["http_stream.cc"],
    hdrs = ["http_stream.h"],
    deps = [
        "//external:base",
        "//nineserver/arena",
        "//nineserver/http:http_contents",
        "//nineserver/http:http_parameters",
        "//nineserver/http:util",
        "//nineserver/http:url",
        "//nineserver:base64",
        "//nineserver:common",
        "//nineserver:json",
    ],
)

cc_library(
    name = "standalone_stream",
    srcs = ["standalone_stream.cc"],
    hdrs = ["standalone_stream.h"],
    deps = [
        ":http_stream",
        "//external:base",
        "//nineserver/arena",
        "//nineserver/http:http_contents",
        "//nineserver/http:http_parameters",
        "//nineserver/http:url",
        "//nineserver/http:util",
        "//nineserver/io:descriptor_io",
        "//nineserver:common",
        "//nineserver:signal",
    ],
)

cc_library(
    name = "string_stream",
    srcs = ["string_stream.cc"],
    hdrs = ["string_stream.h"],
    deps = [
        ":http_stream",
        "//external:base",
        "//nineserver/io:string_io",
    ],
)

cc_test(
    name = "string_stream_test",
    srcs = ["string_stream_test.cc"],
    deps = [
        ":string_stream",
        "//external:testing_main",
        "//nineserver/benchmark",
    ],
    tags = ["benchmark"],
)
