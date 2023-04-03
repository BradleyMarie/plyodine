load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

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
        "@com_google_googletest//:gtest_main",
    ],
)
