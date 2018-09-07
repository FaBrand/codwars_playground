cc_library(
    name = "assembler",
    srcs = glob(["src/*.cpp"]),
)


cc_test (
    name = "tests",
    srcs = glob(["test/*.cpp"]),
    deps = [
        "assembler",
        "@gtest//:gtest",
        "@gtest//:gtest_main" # Only if hello_test.cc has no main()
    ],
)
