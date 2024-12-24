#include "plyodine/readers/triangle_mesh_reader.h"

#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

namespace plyodine {
namespace {

using ::testing::ElementsAre;

template <std::floating_point LocationType, std::floating_point NormalType,
          std::floating_point UVType, std::integral FaceIndexType>
class TestTriangleMeshReader final
    : public TriangleMeshReader<LocationType, NormalType, UVType,
                                FaceIndexType> {
 public:
  void Start() override {
    positions.clear();
    normals.clear();
    uvs.clear();
    faces.clear();
  }

  void AddVertex(const std::array<LocationType, 3>& position,
                 const std::array<NormalType, 3>* maybe_normal,
                 const std::array<UVType, 2>* maybe_uv) override {
    positions.push_back(position);

    if (maybe_normal) {
      normals.push_back(*maybe_normal);
    }

    if (maybe_uv) {
      uvs.push_back(*maybe_uv);
    }
  }

  void AddFace(const std::array<FaceIndexType, 3>& face) override {
    faces.push_back(face);
  }

  std::vector<std::array<LocationType, 3u>> positions;
  std::vector<std::array<NormalType, 3u>> normals;
  std::vector<std::array<UVType, 2u>> uvs;
  std::vector<std::array<FaceIndexType, 3u>> faces;
};

TEST(TriangleMeshReader, Empty) {
  std::stringstream input("ply\rformat ascii 1.0\rend_header\r");

  TestTriangleMeshReader<float, float, float, uint32_t> reader;
  EXPECT_EQ(reader.ReadFrom(input).message(),
            "Element vertex must have properties x, y, and z");
}

TEST(TriangleMeshReader, PositionBadType) {
  std::string names[] = {"x", "y", "z"};
  std::string types[] = {"char",
                         "uchar",
                         "short",
                         "ushort",
                         "int",
                         "uint",
                         "list uchar char",
                         "list uchar uchar",
                         "list uchar short",
                         "list uchar ushort",
                         "list uchar int",
                         "list uchar uint",
                         "list uchar float",
                         "list uchar double"};
  for (const auto& name : names) {
    for (const auto& type : types) {
      std::stringstream input(
          "ply\rformat ascii 1.0\relement vertex 1\rproperty " + type + " " +
          name + "\rend_header\r");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "The type of properties x, y, and z, on vertex elements must "
                "be either float or double");
    }
  }
}

TEST(TriangleMeshReader, PositionMissing) {
  std::string properties[] = {"property float x\r", "property float y\r",
                              "property float z\r"};
  for (size_t i = 0; i < 3; i++) {
    std::string input_string = "ply\rformat ascii 1.0\relement vertex 1\r";
    for (size_t j = 0; j < 3; j++) {
      if (i != j) {
        input_string += properties[j];
      }
    }
    input_string += "end_header\r";
    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;
    EXPECT_EQ(reader.ReadFrom(input).message(),
              "Element vertex must have properties x, y, and z");
  }
}

TEST(TriangleMeshReader, NormalBadType) {
  std::string names[] = {"nx", "ny", "nz"};
  std::string types[] = {"char",
                         "uchar",
                         "short",
                         "ushort",
                         "int",
                         "uint",
                         "list uchar char",
                         "list uchar uchar",
                         "list uchar short",
                         "list uchar ushort",
                         "list uchar int",
                         "list uchar uint",
                         "list uchar float",
                         "list uchar double"};
  for (const auto& name : names) {
    for (const auto& type : types) {
      std::stringstream input(
          "ply\rformat ascii 1.0\relement vertex 1\rproperty " + type + " " +
          name + "\rend_header\r");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;
      EXPECT_EQ(
          reader.ReadFrom(input).message(),
          "The type of properties nx, ny, and nz, on vertex elements must "
          "be either float or double");
    }
  }
}

TEST(TriangleMeshReader, UVBadType) {
  std::string names[] = {"u", "texture_u", "v", "texture_v",
                         "s", "texture_s", "t", "texture_t"};
  std::string types[] = {"char",
                         "uchar",
                         "short",
                         "ushort",
                         "int",
                         "uint",
                         "list uchar char",
                         "list uchar uchar",
                         "list uchar short",
                         "list uchar ushort",
                         "list uchar int",
                         "list uchar uint",
                         "list uchar float",
                         "list uchar double"};
  for (const auto& name : names) {
    for (const auto& type : types) {
      std::stringstream input(
          "ply\rformat ascii 1.0\relement vertex 1\rproperty " + type + " " +
          name + "\rend_header\r");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;
      EXPECT_EQ(
          reader.ReadFrom(input).message(),
          "The type of properties texture_s, texture_t, texture_u, texture_v, "
          "s, t, u, and v on vertex elements must be either float or double");
    }
  }
}

TEST(TriangleMeshReader, VertexIndicesBadType) {
  std::string names[] = {"vertex_indices"};
  std::string types[] = {
      "char", "uchar", "short",  "ushort",           "int",
      "uint", "float", "double", "list uchar float", "list uchar double"};
  for (const auto& name : names) {
    for (const auto& type : types) {
      std::stringstream input(
          "ply\rformat ascii 1.0\relement face 1\rproperty " + type + " " +
          name + "\rend_header\r");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "The type of property vertex_indices on face elements must be "
                "an integral list type");
    }
  }
}

TEST(TriangleMeshReader, VertexIndicesMissing) {
  std::stringstream input(
      "ply\rformat ascii 1.0\relement vertex 1\rproperty float x\rproperty "
      "float y\rproperty float z\rend_header\r");
  TestTriangleMeshReader<float, float, float, uint32_t> reader;
  EXPECT_EQ(reader.ReadFrom(input).message(),
            "Element face must have property vertex_indices");
}

TEST(TriangleMeshReader, PropertyTypes) {
  std::string xs[] = {"property float x\r", "property double x\r"};
  std::string ys[] = {"property float y\r", "property double y\r"};
  std::string zs[] = {"property float z\r", "property double z\r"};
  std::string nxs[] = {"property float nx\r", "property double nx\r"};
  std::string nys[] = {"property float ny\r", "property double ny\r"};
  std::string nzs[] = {"property float nz\r", "property double nz\r"};
  std::string us[] = {
      "property float u\r",         "property double u\r",
      "property float s\r",         "property double s\r",
      "property float texture_u\r", "property double texture_u\r",
      "property float texture_s\r", "property double texture_s\r"};
  std::string vs[] = {
      "property float v\r",         "property double v\r",
      "property float t\r",         "property double t\r",
      "property float texture_v\r", "property double texture_v\r",
      "property float texture_t\r", "property double texture_t\r"};
  std::string vertex_indices[] = {"property list uchar char vertex_indices\r",
                                  "property list uchar uchar vertex_indices\r",
                                  "property list uchar short vertex_indices\r",
                                  "property list uchar ushort vertex_indices\r",
                                  "property list uchar int vertex_indices\r",
                                  "property list uchar uint vertex_indices\r"};

  for (const auto& x : xs) {
    for (const auto& y : ys) {
      for (const auto& z : zs) {
        for (const auto& nx : nxs) {
          for (const auto& ny : nys) {
            for (const auto& nz : nzs) {
              for (const auto& u : us) {
                for (const auto& v : vs) {
                  for (const auto& vertex_index : vertex_indices) {
                    std::stringstream input(
                        "ply\rformat ascii 1.0\relement vertex 0\r" + x + y +
                        z + nx + ny + nz + u + v + "element face 0\r" +
                        vertex_index + "end_header\r");
                    TestTriangleMeshReader<float, float, float, uint32_t>
                        reader;
                    EXPECT_EQ(0, reader.ReadFrom(input).value());
                    EXPECT_TRUE(reader.positions.empty());
                    EXPECT_TRUE(reader.normals.empty());
                    EXPECT_TRUE(reader.uvs.empty());
                    EXPECT_TRUE(reader.faces.empty());
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(TriangleMeshReader, OutOfRangePosition) {
  std::string position[] = {
      "1.0", "1000000000000000000000000000000000000000000000000000000000.0"};

  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 1\rproperty double x\rproperty "
        "double y\rproperty double z\relement face 0\rproperty list uchar "
        "uchar vertex_indices\rend_header\r";
    for (size_t j = 0; j < 3; j++) {
      if (i == j) {
        input_string += position[1];
      } else {
        input_string += position[0];
      }

      if (j != 2) {
        input_string += " ";
      }
    }
    input_string += "\r";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    if (i == 0) {
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "Input contained a non-finite value for x");
    } else if (i == 1) {
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "Input contained a non-finite value for y");
    } else {
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "Input contained a non-finite value for z");
    }
  }
}

TEST(TriangleMeshReader, OutOfRangeNormal) {
  std::string position[] = {
      "1.0", "1000000000000000000000000000000000000000000000000000000000.0"};

  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 1\rproperty double x\rproperty "
        "double y\rproperty double z\rproperty double nx\rproperty "
        "double ny\rproperty double nz\relement face 0\rproperty list uchar "
        "uchar vertex_indices\rend_header\r0.0 0.0 0.0 ";
    for (size_t j = 0; j < 3; j++) {
      if (i == j) {
        input_string += position[1];
      } else {
        input_string += position[0];
      }

      if (j != 2) {
        input_string += " ";
      }
    }
    input_string += "\r";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    if (i == 0) {
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "Input contained a non-finite value for nx");
    } else if (i == 1) {
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "Input contained a non-finite value for ny");
    } else {
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "Input contained a non-finite value for nz");
    }
  }
}

TEST(TriangleMeshReader, UVs) {
  std::string position[] = {
      "1.0", "1000000000000000000000000000000000000000000000000000000000.0"};

  for (size_t i = 0; i < 2; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 1\rproperty double x\rproperty "
        "double y\rproperty double z\rproperty double u\rproperty "
        "double v\relement face 0\rproperty list uchar uchar "
        "vertex_indices\rend_header\r0.0 0.0 0.0 ";
    for (size_t j = 0; j < 2; j++) {
      if (i == j) {
        input_string += position[1];
      } else {
        input_string += position[0];
      }

      if (j != 2) {
        input_string += " ";
      }
    }
    input_string += "\r";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    if (i == 0) {
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "Input contained a non-finite value for u");
    } else {
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "Input contained a non-finite value for v");
    }
  }
}

TEST(TriangleMeshReader, VertexIndexTooLow) {
  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 3\rproperty float x\rproperty "
        "float y\rproperty float z\relement face 1\rproperty list uchar char "
        "vertex_indices\rend_header\r0.0 0.0 0.0\r1.0 0.0 0.0\r0.0 1.0 0.0\r3 ";
    for (size_t j = 0; j < 3; j++) {
      if (i != j) {
        input_string += std::to_string(j);
      } else {
        input_string += "-1";
      }

      if (j != 2) {
        input_string += " ";
      }
    }
    input_string += "\r";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "A vertex index was out of range");
  }
}

TEST(TriangleMeshReader, VertexIndexMoreThanVertices) {
  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 3\rproperty float x\rproperty "
        "float y\rproperty float z\relement face 1\rproperty list uchar char "
        "vertex_indices\rend_header\r0.0 0.0 0.0\r1.0 0.0 0.0\r0.0 1.0 0.0\r3 ";
    for (size_t j = 0; j < 3; j++) {
      if (i != j) {
        input_string += std::to_string(j);
      } else {
        input_string += "4";
      }

      if (j != 2) {
        input_string += " ";
      }
    }
    input_string += "\r";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "A vertex index was out of range");
  }
}

TEST(TriangleMeshReader, VertexIndexTooBigForIndexType) {
  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 256\rproperty float x\rproperty "
        "float y\rproperty float z\relement face 1\rproperty list uchar uchar "
        "vertex_indices\rend_header\r";
    for (size_t j = 0; j < 256u; j++) {
      input_string += "0.0 0.0 0.0\r";
    }

    input_string += "3 ";
    for (size_t j = 0; j < 3; j++) {
      if (i != j) {
        input_string += std::to_string(j);
      } else {
        input_string += "128";
      }

      if (j != 2) {
        input_string += " ";
      }
    }
    input_string += "\r";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, int8_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "A vertex index was out of range");
  }
}

TEST(TriangleMeshReader, ValidPositionsOnly) {
  std::string input_string =
      "ply\rformat ascii 1.0\relement vertex 3\rproperty float x\rproperty "
      "float y\rproperty float z\relement face 1\rproperty list uchar char "
      "vertex_indices\rend_header\r0.0 0.0 0.0\r1.0 0.0 0.0\r0.0 1.0 0.0\r4 0 "
      "1 2 1\r";

  std::stringstream input(input_string);
  TestTriangleMeshReader<float, float, float, uint32_t> reader;

  ASSERT_EQ(0, reader.ReadFrom(input).value());
  ASSERT_EQ(reader.positions.size(), 3u);
  EXPECT_THAT(reader.positions[0], ElementsAre(0.0, 0.0, 0.0));
  EXPECT_THAT(reader.positions[1], ElementsAre(1.0, 0.0, 0.0));
  EXPECT_THAT(reader.positions[2], ElementsAre(0.0, 1.0, 0.0));
  EXPECT_TRUE(reader.normals.empty());
  EXPECT_TRUE(reader.uvs.empty());
  ASSERT_EQ(reader.faces.size(), 2u);
  EXPECT_THAT(reader.faces[0], ElementsAre(0u, 1u, 2u));
  EXPECT_THAT(reader.faces[1], ElementsAre(0u, 2u, 1u));
}

TEST(TriangleMeshReader, Valid) {
  std::string input_string =
      "ply\rformat ascii 1.0\relement vertex 3\rproperty float x\rproperty "
      "float y\rproperty float z\rproperty float nx\rproperty float "
      "ny\rproperty float nz\rproperty float u\rproperty float v\relement face "
      "1\rproperty list uchar char vertex_indices\rend_header\r0.0 0.0 3.0 1.0 "
      "2.0 3.0 0.5 0.25\r1.0 0.0 0.0 1 0 0 0 0\r0.0 1.0 0.0 1 0 0 0 0 \r4 0 1 "
      "2 1\r";

  std::stringstream input(input_string);
  TestTriangleMeshReader<float, float, float, uint32_t> reader;

  ASSERT_EQ(0, reader.ReadFrom(input).value());
  ASSERT_EQ(reader.positions.size(), 3u);
  EXPECT_THAT(reader.positions[0], ElementsAre(0.0, 0.0, 3.0));
  EXPECT_THAT(reader.positions[1], ElementsAre(1.0, 0.0, 0.0));
  EXPECT_THAT(reader.positions[2], ElementsAre(0.0, 1.0, 0.0));
  ASSERT_EQ(reader.normals.size(), 3u);
  EXPECT_THAT(reader.normals[0], ElementsAre(1.0, 2.0, 3.0));
  EXPECT_THAT(reader.normals[1], ElementsAre(1.0, 0.0, 0.0));
  EXPECT_THAT(reader.normals[2], ElementsAre(1.0, 0.0, 0.0));
  ASSERT_EQ(reader.uvs.size(), 3u);
  EXPECT_THAT(reader.uvs[0], ElementsAre(0.5, 0.25));
  EXPECT_THAT(reader.uvs[1], ElementsAre(0.0, 0.0));
  EXPECT_THAT(reader.uvs[2], ElementsAre(0.0, 0.0));
  ASSERT_EQ(reader.faces.size(), 2u);
  EXPECT_THAT(reader.faces[0], ElementsAre(0u, 1u, 2u));
  EXPECT_THAT(reader.faces[1], ElementsAre(0u, 2u, 1u));
}

}  // namespace
}  // namespace plyodine