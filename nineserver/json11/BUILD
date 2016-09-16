licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "json11",
    srcs = ["json11.cpp"],
    hdrs = ["json11.hpp"],
    deps = ["//external:base"],
)

cc_test(
    name = "test",
    srcs = ["test.cpp"],
    deps = [":json11"],
)
