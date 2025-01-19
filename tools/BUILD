load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//visibility:private"])

cc_binary(
    name = "ply_validator",
    srcs = ["ply_validator.cc"],
    deps = [
        "//plyodine:ply_reader",
    ],
)
