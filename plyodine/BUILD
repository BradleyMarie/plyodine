load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "ply_reader",
    srcs = ["ply_reader.cc"],
    hdrs = ["ply_reader.h"],
)

cc_test(
    name = "ply_reader_test",
    srcs = ["ply_reader_test.cc"],
    data = [
        "test_data/header_empty.ply",
        "test_data/header_illegal_characters.ply",
        "test_data/header_valid_mac.ply",
        "test_data/header_valid_unix.ply",
        "test_data/header_valid_windows.ply",
    ],
    deps = [
        ":ply_reader",
        "@com_google_googletest//:gtest_main",
    ],
)
