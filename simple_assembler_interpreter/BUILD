
cc_library(
    name = "assembler",
    srcs = glob(["src/*.cpp"]),
    hdrs = ["src/assembler_main.h"],
    visibility = ["//visibility:public"],

)


cc_test (
    name = "tests",
    srcs = glob(["test/*.cpp"]),
    deps = [
        "assembler",
        "@googletest//:gtest_main"
    ],
)
