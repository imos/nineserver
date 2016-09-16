licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "arena",
    srcs = ["arena.cc"],
    hdrs = ["arena.h"],
    deps = [
        ":arena_buffer",
        "//external:base",
        "//nineserver:common",
    ],
)

cc_test(
    name = "arena_test",
    srcs = ["arena_test.cc"],
    deps = [
        ":arena",
        "//external:testing_main",
    ],
)

cc_library(
    name = "arena_buffer",
    srcs = ["arena_buffer.cc"],
    hdrs = ["arena_buffer.h"],
    deps = [
        ":arena_string_piece",
        "//external:base",
        "//nineserver:common",
    ],
)

cc_test(
    name = "arena_buffer_test",
    srcs = ["arena_buffer_test.cc"],
    deps = [
        ":arena_buffer",
        "//external:testing_main",
        "//nineserver/benchmark",
    ],
    tags = ["benchmark"],
)

cc_library(
    name = "arena_interface",
    srcs = ["arena_interface.cc"],
    hdrs = ["arena_interface.h"],
    deps = [
        ":arena",
        ":arena_buffer",
        "//external:base",
        "//nineserver:common",
    ],
)

cc_test(
    name = "arena_interface_test",
    srcs = ["arena_interface_test.cc"],
    deps = [
        ":arena_interface",
        "//external:testing_main",
        "//nineserver/benchmark",
    ],
    tags = ["benchmark"],
)

cc_library(
    name = "arena_string_piece",
    srcs = ["arena_string_piece.cc"],
    hdrs = ["arena_string_piece.h"],
    deps = [
        "//external:base",
        "//nineserver:common",
    ],
)
