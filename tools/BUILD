load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
    default_visibility = ["//visibility:private"],
    features = [
        "layering_check",
        "parse_headers",
    ],
)

cc_binary(
    name = "ply_sanitizer",
    srcs = ["ply_sanitizer.cc"],
    deps = [
        "//plyodine:ply_header_reader",
        "//plyodine:ply_reader",
        "//plyodine:ply_writer",
    ],
)

cc_binary(
    name = "ply_validator",
    srcs = ["ply_validator.cc"],
    deps = [
        "//plyodine:ply_reader",
    ],
)
