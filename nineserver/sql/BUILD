licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        ":mysql",
        "//external:base",
    ],
    linkopts = ["-lmysqlclient"],
)

cc_library(
    name = "mysql",
    srcs = ["mysql.cc"],
    hdrs = ["mysql.h"],
    deps = [
        "//external:base",
        "//nineserver/arena:arena_buffer",
        "//nineserver:json",
    ],
    linkopts = ["-lmysqlclient"],
)

cc_test(
    name = "mysql_test",
    srcs = ["mysql_test.cc"],
    deps = [
        ":mysql",
        "//external:testing_main",
    ],
)
