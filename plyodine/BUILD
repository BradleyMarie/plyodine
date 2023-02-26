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
        "test_data/header_comment_allows_spaces.ply",
        "test_data/header_comment_empty.ply",
        "test_data/header_element_count_bad.ply",
        "test_data/header_element_count_negative.ply",
        "test_data/header_element_count_none.ply",
        "test_data/header_element_count_too_large.ply",
        "test_data/header_element_name_none.ply",
        "test_data/header_element_name_repeated.ply",
        "test_data/header_element_too_many.ply",
        "test_data/header_empty_line.ply",
        "test_data/header_end_too_many.ply",
        "test_data/header_format_ascii.ply",
        "test_data/header_format_bad.ply",
        "test_data/header_format_big_endian.ply",
        "test_data/header_format_little_endian.ply",
        "test_data/header_format_too_long.ply",
        "test_data/header_invalid_keyword.ply",
        "test_data/header_loose_property.ply",
        "test_data/header_mismatched_endings.ply",
        "test_data/header_property_list_data_type_bad.ply",
        "test_data/header_property_list_data_type_none.ply",
        "test_data/header_property_list_list_type_bad.ply",
        "test_data/header_property_list_list_type_none.ply",
        "test_data/header_property_list_name_duplicated.ply",
        "test_data/header_property_list_name_none.ply",
        "test_data/header_property_list_too_many.ply",
        "test_data/header_property_name_duplicated.ply",
        "test_data/header_property_name_none.ply",
        "test_data/header_property_too_many.ply",
        "test_data/header_property_type_bad.ply",
        "test_data/header_property_type_none.ply",
        "test_data/header_spaces_leading.ply",
        "test_data/header_spaces_multiple.ply",
        "test_data/header_spaces_trailing.ply",
        "test_data/header_valid_mac.ply",
        "test_data/header_valid_unix.ply",
        "test_data/header_valid_windows.ply",
    ],
    deps = [
        ":ply_reader",
        "@com_google_googletest//:gtest_main",
    ],
)
