
cc_library(
    name = "simple_assembler",
    srcs = glob(["src/*.cpp"]),
    hdrs = ["src/assembler_main.h"],
    visibility = ["//visibility:public"],

)


cc_test (
    name = "tests",
    srcs = glob(["test/*.cpp"]),
    deps = [
        "simple_assembler",
        "@googletest//:gtest_main"
    ],
)
