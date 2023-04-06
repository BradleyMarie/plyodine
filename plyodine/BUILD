load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "normalized_reader",
    hdrs = ["normalized_reader.h"],
    deps = [
        ":ply_property",
        ":ply_reader",
    ],
)

cc_library(
    name = "ply_header_reader",
    srcs = ["ply_header_reader.cc"],
    hdrs = ["ply_header_reader.h"],
)

cc_test(
    name = "ply_header_reader_test",
    srcs = ["ply_header_reader_test.cc"],
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
        "test_data/header_obj_info_allows_spaces.ply",
        "test_data/header_obj_info_empty.ply",
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
        ":ply_header_reader",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "ply_property",
    hdrs = ["ply_property.h"],
)

cc_library(
    name = "ply_reader",
    srcs = ["ply_reader.cc"],
    hdrs = ["ply_reader.h"],
    deps = [
        ":ply_header_reader",
        ":ply_property",
    ],
)

cc_test(
    name = "ply_reader_test",
    srcs = ["ply_reader_test.cc"],
    data = [
        "test_data/header_format_bad.ply",
        "test_data/ply_ascii_data.ply",
        "test_data/ply_ascii_empty.ply",
        "test_data/ply_ascii_empty_token.ply",
        "test_data/ply_ascii_entry_bad_double.ply",
        "test_data/ply_ascii_entry_bad_float.ply",
        "test_data/ply_ascii_entry_bad_int16.ply",
        "test_data/ply_ascii_entry_bad_int32.ply",
        "test_data/ply_ascii_entry_bad_int8.ply",
        "test_data/ply_ascii_entry_bad_uint16.ply",
        "test_data/ply_ascii_entry_bad_uint32.ply",
        "test_data/ply_ascii_entry_bad_uint8.ply",
        "test_data/ply_ascii_entry_too_large_double.ply",
        "test_data/ply_ascii_entry_too_large_float.ply",
        "test_data/ply_ascii_entry_too_large_int16.ply",
        "test_data/ply_ascii_entry_too_large_int32.ply",
        "test_data/ply_ascii_entry_too_large_int8.ply",
        "test_data/ply_ascii_entry_too_large_uint16.ply",
        "test_data/ply_ascii_entry_too_large_uint32.ply",
        "test_data/ply_ascii_entry_too_large_uint8.ply",
        "test_data/ply_ascii_invalid_character.ply",
        "test_data/ply_ascii_list_missing_entries.ply",
        "test_data/ply_ascii_list_sizes.ply",
        "test_data/ply_ascii_list_sizes_bad_int16.ply",
        "test_data/ply_ascii_list_sizes_bad_int32.ply",
        "test_data/ply_ascii_list_sizes_bad_int8.ply",
        "test_data/ply_ascii_list_sizes_bad_uint16.ply",
        "test_data/ply_ascii_list_sizes_bad_uint32.ply",
        "test_data/ply_ascii_list_sizes_bad_uint8.ply",
        "test_data/ply_ascii_list_sizes_negative_int16.ply",
        "test_data/ply_ascii_list_sizes_negative_int32.ply",
        "test_data/ply_ascii_list_sizes_negative_int8.ply",
        "test_data/ply_ascii_list_sizes_signed.ply",
        "test_data/ply_ascii_list_sizes_too_large_int16.ply",
        "test_data/ply_ascii_list_sizes_too_large_int32.ply",
        "test_data/ply_ascii_list_sizes_too_large_int8.ply",
        "test_data/ply_ascii_list_sizes_too_large_uint16.ply",
        "test_data/ply_ascii_list_sizes_too_large_uint32.ply",
        "test_data/ply_ascii_list_sizes_too_large_uint8.ply",
        "test_data/ply_ascii_mismatched_line_endings.ply",
        "test_data/ply_ascii_missing_element.ply",
        "test_data/ply_ascii_unused_tokens.ply",
        "test_data/ply_big_data.ply",
        "test_data/ply_big_empty.ply",
        "test_data/ply_big_list_sizes.ply",
        "test_data/ply_big_list_sizes_negative_int16.ply",
        "test_data/ply_big_list_sizes_negative_int32.ply",
        "test_data/ply_big_list_sizes_negative_int8.ply",
        "test_data/ply_big_list_sizes_signed.ply",
        "test_data/ply_little_data.ply",
        "test_data/ply_little_empty.ply",
        "test_data/ply_little_list_sizes.ply",
        "test_data/ply_little_list_sizes_negative_int16.ply",
        "test_data/ply_little_list_sizes_negative_int32.ply",
        "test_data/ply_little_list_sizes_negative_int8.ply",
        "test_data/ply_little_list_sizes_signed.ply",
    ],
    deps = [
        ":ply_reader",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "ply_writer",
    srcs = ["ply_writer.cc"],
    hdrs = ["ply_writer.h"],
    deps = [
        ":ply_property",
    ],
)

cc_test(
    name = "ply_writer_test",
    srcs = ["ply_writer_test.cc"],
    data = [
        "test_data/ply_ascii_data.ply",
        "test_data/ply_ascii_empty.ply",
        "test_data/ply_ascii_large_fp.ply",
        "test_data/ply_ascii_list_sizes.ply",
        "test_data/ply_ascii_small_fp.ply",
        "test_data/ply_big_data.ply",
        "test_data/ply_big_empty.ply",
        "test_data/ply_big_list_sizes.ply",
        "test_data/ply_little_data.ply",
        "test_data/ply_little_empty.ply",
        "test_data/ply_little_list_sizes.ply",
    ],
    deps = [
        ":ply_writer",
        "@com_google_googletest//:gtest_main",
    ],
)
