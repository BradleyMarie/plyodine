load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(
    default_visibility = ["//visibility:public"],
    features = [
        "layering_check",
        "parse_headers",
    ],
)

cc_library(
    name = "in_memory_writer",
    srcs = ["in_memory_writer.cc"],
    hdrs = ["in_memory_writer.h"],
    deps = [
        "//plyodine:ply_writer",
    ],
)

cc_test(
    name = "in_memory_writer_test",
    srcs = ["in_memory_writer_test.cc"],
    deps = [
        ":in_memory_writer",
        "@googletest//:gtest_main",
    ],
)
