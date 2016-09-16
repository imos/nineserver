licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "system",
    srcs = glob(["*.cc"]),
    hdrs = ["system.h"],
    deps = [
        "//external:base",
        "//nineserver/handler:base_handler",
        "//nineserver/handler:logging",
        "//nineserver/http:html",
        "//nineserver/sql:mysql",
    ],
    alwayslink = 1,
)
