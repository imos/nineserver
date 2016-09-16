licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "benchmark",
    srcs = ["benchmark.cc"],
    hdrs = ["benchmark.h"],
    deps = [
        "//external:base",
    ],
)

cc_test(
    name = "benchmark_test",
    srcs = ["benchmark_test.cc"],
    deps = [
        ":benchmark",
        "//external:testing_main",
    ],
    tags = ["benchmark"],
)
