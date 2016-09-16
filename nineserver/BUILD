licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    srcs = ["common.cc"],
    hdrs = ["common.h"],
    deps = ["//external:base"],
)

cc_library(
    name = "util",
    srcs = ["util.cc"],
    hdrs = ["util.h"],
    deps = [
        ":common",
        ":hash",
        ":json",
        ":signal",
        "//external:base",
        "//nineserver/arena:arena",
        "//nineserver/arena:arena_buffer",
        "//nineserver/arena:arena_interface",
        "//nineserver/arena:arena_string_piece",
        "//nineserver/handler",
        "//nineserver/http/stream:http_stream",
        "//nineserver/http:html",
        "//nineserver/http:http_contents",
        "//nineserver/http:http_parameters",
        "//nineserver/http:proxy",
        "//nineserver/http:url",
        "//nineserver/http:util",
        "//nineserver/io:descriptor_io",
        "//nineserver/io:generic_io",
        "//nineserver/sql:mysql",
    ],
)

cc_library(
    name = "base64",
    srcs = ["base64.cc"],
    hdrs = ["base64.h"],
    deps = [
        "//external:base",
        "//nineserver/arena:arena_buffer",
    ],
)

cc_test(
    name = "base64_test",
    srcs = ["base64_test.cc"],
    deps = [
        ":base64",
        "//external:testing_main",
    ],
)

cc_binary(
    name = "hash_main",
    srcs = ["hash_main.cc"],
    deps = [
        ":hash",
        "//external:base",
        "//nineserver/arena:arena_buffer",
    ],
    linkopts = ["-lcrypto"],
)

cc_library(
    name = "hash",
    srcs = ["hash.cc"],
    hdrs = ["hash.h"],
    deps = [
        "//external:base",
        "//nineserver/arena:arena_buffer",
    ],
    linkopts = ["-lcrypto"],
)

cc_library(
    name = "json",
    srcs = ["json.cc"],
    hdrs = ["json.h"],
    deps = [
        ":common",
        "//external:base",
        "//nineserver/json11",
    ],
)

cc_test(
    name = "json_test",
    srcs = ["json_test.cc"],
    deps = [
        ":json",
        "//external:testing_main",
    ],
)

cc_test(
    name = "common_test",
    srcs = ["common_test.cc"],
    deps = [
        ":util",
        "//external:testing_main",
    ],
)

filegroup(
    name = "main",
    srcs = ["main.cc"],
)

cc_library(
    name = "signal",
    srcs = ["signal.cc"],
    hdrs = ["signal.h"],
    deps = ["//external:base"],
)
