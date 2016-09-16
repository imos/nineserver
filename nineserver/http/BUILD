licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "html",
    srcs = ["html.cc"],
    hdrs = ["html.h"],
    deps = [
        "//external:base",
    ],
)

cc_test(
    name = "html_test",
    srcs = ["html_test.cc"],
    deps = [
        ":html",
        "//external:testing_main",
    ],
)

cc_library(
    name = "http_contents",
    srcs = ["http_contents.cc"],
    hdrs = ["http_contents.h"],
    deps = [
        ":http_parameters",
        "//external:base",
        "//nineserver/arena",
        "//nineserver/arena:arena_interface",
        "//nineserver/io:generic_io",
        "//nineserver:common",
        "//nineserver:json",
    ],
)

cc_test(
    name = "http_contents_test",
    srcs = ["http_contents_test.cc"],
    deps = [
        ":http_contents",
        "//external:testing_main",
        "//nineserver/benchmark",
        "//nineserver/io:string_io",
    ],
    tags = ["benchmark"],
)

cc_library(
    name = "http_parameters",
    srcs = ["http_parameters.cc"],
    hdrs = ["http_parameters.h"],
    deps = [
        "//external:base",
        "//nineserver/io:generic_io",
        "//nineserver/arena",
        "//nineserver:common",
        "//nineserver:json",
    ],
)

cc_test(
    name = "http_parameters_test",
    srcs = ["http_parameters_test.cc"],
    deps = [
        ":http_parameters",
        "//external:testing_main",
        "//nineserver/benchmark",
    ],
    tags = ["benchmark"],
)

cc_library(
    name = "proxy",
    srcs = ["proxy.cc"],
    hdrs = ["proxy.h"],
    deps = [
        ":http_contents",
        "//external:base",
        "//nineserver/io:descriptor_io",
        "//nineserver:common",
    ],
)

cc_binary(
    name = "proxy_main",
    srcs = ["proxy_main.cc"],
    deps = [
        ":proxy",
        ":http_contents",
        "//external:base",
        "//nineserver/arena",
        "//nineserver/io:descriptor_io",
        "//nineserver:common",
    ],
)

cc_library(
    name = "session",
    srcs = ["session.cc"],
    hdrs = ["session.h"],
    deps = [
        "//external:base",
    ],
)

cc_test(
    name = "session_test",
    srcs = ["session_test.cc"],
    deps = [
        ":session",
        "//external:testing_main",
        "//nineserver:common",
    ],
)

cc_library(
    name = "url",
    srcs = ["url.cc"],
    hdrs = ["url.h"],
    deps = [
        "//external:base",
        "//nineserver/arena:arena_buffer",
    ],
)

cc_library(
    name = "util",
    srcs = ["util.cc"],
    hdrs = ["util.h"],
    deps = [
        "//external:base",
        "//nineserver/arena:arena_string_piece",
        "@cctz//:time_zone",
    ],
)

cc_test(
    name = "util_test",
    srcs = ["util_test.cc"],
    deps = [
        ":util",
        "//external:testing_main",
    ],
)
