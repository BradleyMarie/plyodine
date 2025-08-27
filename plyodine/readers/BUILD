load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(
    default_visibility = ["//visibility:public"],
    features = [
        "layering_check",
        "parse_headers",
    ],
)

cc_library(
    name = "triangle_mesh_reader",
    hdrs = ["triangle_mesh_reader.h"],
    deps = [
        "//plyodine:ply_reader",
    ],
)

cc_test(
    name = "triangle_mesh_reader_test",
    srcs = ["triangle_mesh_reader_test.cc"],
    deps = [
        ":triangle_mesh_reader",
        "@googletest//:gtest_main",
    ],
)
