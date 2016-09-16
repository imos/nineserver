licenses(["notice"])

package(default_visibility = ["//visibility:public"])

# Base library.
cc_library(
    name = "generic_io",
    srcs = ["generic_io.cc"],
    hdrs = ["generic_io.h"],
    deps = [
        "//external:base",
        "//nineserver/arena:arena_buffer",
        "//nineserver:common",
    ],
)

cc_library(
    name = "descriptor_io",
    srcs = ["descriptor_io.cc"],
    hdrs = ["descriptor_io.h"],
    deps = [
        ":generic_io",
        "//external:base",
        "//nineserver:common",
    ],
)

cc_library(
    name = "string_io",
    srcs = ["string_io.cc"],
    hdrs = ["string_io.h"],
    deps = [
        ":generic_io",
        "//external:base",
        "//nineserver/arena",
        "//nineserver:common",
    ],
)

cc_test(
    name = "string_io_test",
    srcs = ["string_io_test.cc"],
    deps = [
        ":string_io",
        "//external:testing_main",
        "//nineserver/arena:arena",
        "//nineserver/arena:arena_buffer",
    ],
)
