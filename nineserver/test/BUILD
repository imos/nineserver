licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "main",
    srcs = ["handler.cc", "//nineserver:main"],
    deps = [
        "//external:base",
        "//nineserver/handler",
        "//nineserver/http:proxy",
    ],
)

sh_test(
    name = "scenario_test",
    srcs = ["scenario_test.sh"],
    data = [
        ":main",
        "//nineserver/tools:free_port",
    ],
)
