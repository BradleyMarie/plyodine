#include "plyodine/ply_reader.h"

#include <fstream>

#include "googletest/include/gtest/gtest.h"

TEST(Header, BadStream) {
  std::ifstream input("notarealfile.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::IO_ERROR, result.error().Code());
  EXPECT_EQ("Bad stream passed", result.error().Message());
}

TEST(Header, Empty) {
  std::ifstream input("plyodine/test_data/header_empty.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("The first line of the file must exactly contain the magic string",
            result.error().Message());
}

TEST(Header, IllegalCharacters) {
  std::ifstream input("plyodine/test_data/header_illegal_characters.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("The file contained an invalid character",
            result.error().Message());
}

TEST(Header, Valid) {
  std::ifstream input("plyodine/test_data/header_valid.ply");
  auto result = plyodine::internal::ParseHeader(input);
  ASSERT_TRUE(result);
  EXPECT_EQ(plyodine::internal::Format::ASCII, result->format);
  EXPECT_EQ(1u, result->major_version);
  EXPECT_EQ(0u, result->minor_version);
  EXPECT_EQ(2u, result->comments.size());
  EXPECT_EQ("author: Greg Turk", result->comments.at(0));
  EXPECT_EQ("object: another cube", result->comments.at(1));
  EXPECT_EQ(3u, result->elements.size());

  EXPECT_EQ("vertex", result->elements.at(0).name);
  EXPECT_EQ(6u, result->elements.at(0).properties.size());
  EXPECT_EQ("x", result->elements.at(0).properties.at(0).name);
  EXPECT_EQ(plyodine::internal::Type::FLOAT,
            result->elements.at(0).properties.at(0).data_type);
  EXPECT_FALSE(result->elements.at(0).properties.at(0).list_type);
  EXPECT_EQ("y", result->elements.at(0).properties.at(1).name);
  EXPECT_EQ(plyodine::internal::Type::FLOAT,
            result->elements.at(0).properties.at(1).data_type);
  EXPECT_FALSE(result->elements.at(0).properties.at(1).list_type);
  EXPECT_EQ("z", result->elements.at(0).properties.at(2).name);
  EXPECT_EQ(plyodine::internal::Type::FLOAT,
            result->elements.at(0).properties.at(2).data_type);
  EXPECT_FALSE(result->elements.at(0).properties.at(2).list_type);
  EXPECT_EQ("red", result->elements.at(0).properties.at(3).name);
  EXPECT_EQ(plyodine::internal::Type::UINT8,
            result->elements.at(0).properties.at(3).data_type);
  EXPECT_FALSE(result->elements.at(0).properties.at(3).list_type);
  EXPECT_EQ("green", result->elements.at(0).properties.at(4).name);
  EXPECT_EQ(plyodine::internal::Type::UINT8,
            result->elements.at(0).properties.at(4).data_type);
  EXPECT_FALSE(result->elements.at(0).properties.at(4).list_type);
  EXPECT_EQ("blue", result->elements.at(0).properties.at(5).name);
  EXPECT_EQ(plyodine::internal::Type::UINT8,
            result->elements.at(0).properties.at(5).data_type);
  EXPECT_FALSE(result->elements.at(0).properties.at(5).list_type);

  EXPECT_EQ("face", result->elements.at(1).name);
  EXPECT_EQ(1u, result->elements.at(1).properties.size());
  EXPECT_EQ("vertex_index", result->elements.at(1).properties.at(0).name);
  EXPECT_EQ(plyodine::internal::Type::INT32,
            result->elements.at(1).properties.at(0).data_type);
  EXPECT_EQ(plyodine::internal::Type::UINT8,
            *result->elements.at(1).properties.at(0).list_type);

  EXPECT_EQ("edge", result->elements.at(2).name);
  EXPECT_EQ(5u, result->elements.at(2).properties.size());
  EXPECT_EQ("vertex1", result->elements.at(2).properties.at(0).name);
  EXPECT_EQ(plyodine::internal::Type::INT32,
            result->elements.at(2).properties.at(0).data_type);
  EXPECT_FALSE(result->elements.at(2).properties.at(0).list_type);
  EXPECT_EQ("vertex2", result->elements.at(2).properties.at(1).name);
  EXPECT_EQ(plyodine::internal::Type::INT32,
            result->elements.at(2).properties.at(1).data_type);
  EXPECT_FALSE(result->elements.at(2).properties.at(1).list_type);
  EXPECT_EQ("red", result->elements.at(2).properties.at(2).name);
  EXPECT_EQ(plyodine::internal::Type::UINT8,
            result->elements.at(2).properties.at(2).data_type);
  EXPECT_FALSE(result->elements.at(2).properties.at(2).list_type);
  EXPECT_EQ("green", result->elements.at(2).properties.at(3).name);
  EXPECT_EQ(plyodine::internal::Type::UINT8,
            result->elements.at(2).properties.at(3).data_type);
  EXPECT_FALSE(result->elements.at(2).properties.at(3).list_type);
  EXPECT_EQ("blue", result->elements.at(2).properties.at(4).name);
  EXPECT_EQ(plyodine::internal::Type::UINT8,
            result->elements.at(2).properties.at(4).data_type);
  EXPECT_FALSE(result->elements.at(2).properties.at(4).list_type);
}