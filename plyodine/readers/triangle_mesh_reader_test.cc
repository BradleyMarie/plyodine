#include "plyodine/readers/triangle_mesh_reader.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <system_error>
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

  void AddTriangle(
      const std::array<FaceIndexType, 3>& vertex_indices) override {
    faces.push_back(vertex_indices);
  }

  std::vector<std::array<LocationType, 3u>> positions;
  std::vector<std::array<NormalType, 3u>> normals;
  std::vector<std::array<UVType, 2u>> uvs;
  std::vector<std::array<FaceIndexType, 3u>> faces;
};

std::string InfiniteDouble() {
  double f = std::numeric_limits<double>::infinity();

  std::string as_string(sizeof(double), '\0');
  memcpy(as_string.data(), &f, sizeof(double));

  return as_string;
}

std::string ZeroDouble() {
  double f = 0.0;

  std::string as_string(sizeof(double), '\0');
  memcpy(as_string.data(), &f, sizeof(double));

  return as_string;
}

std::string Endianness() {
  if (std::endian::native == std::endian::big) {
    return "binary_big_endian";
  }
  return "binary_little_endian";
}

TEST(TriangleMeshReader, DefaultErrorCondition) {
  std::stringstream input("ply\rformat ascii 1.0\rend_header\r");

  TestTriangleMeshReader<float, float, float, uint32_t> reader;
  const std::error_category& error_catgegory =
      reader.ReadFrom(input).category();

  EXPECT_NE(error_catgegory.default_error_condition(0),
            std::errc::invalid_argument);
  for (int i = 1; i <= 51; i++) {
    EXPECT_EQ(error_catgegory.default_error_condition(i),
              std::errc::invalid_argument);
  }
  EXPECT_NE(error_catgegory.default_error_condition(52),
            std::errc::invalid_argument);
}

TEST(TriangleMeshReader, VertexMissing) {
  std::stringstream input("ply\rformat ascii 1.0\rend_header\r");

  TestTriangleMeshReader<float, float, float, uint32_t> reader;
  EXPECT_EQ(reader.ReadFrom(input).message(),
            "The input did not contain required element 'vertex'");
}

TEST(TriangleMeshReader, FaceMissing) {
  std::stringstream input(
      "ply\r"
      "format ascii 1.0\r"
      "element vertex 0\r"
      "property float x\r"
      "property float y\r"
      "property float z\r"
      "end_header\r");

  TestTriangleMeshReader<float, float, float, uint32_t> reader;
  EXPECT_EQ(reader.ReadFrom(input).message(),
            "The input did not contain required element 'face'");
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
  for (size_t i = 0; i < 3; i++) {
    for (const auto& type : types) {
      std::string properties[3] = {"property float x\r", "property float y\r",
                                   "property float z\r"};
      properties[i] = "property " + type + " " + names[i] + "\r";

      std::stringstream input("ply\rformat ascii 1.0\relement vertex 1\r" +
                              properties[0] + properties[1] + properties[2] +
                              "element face 1\rproperty list uchar uchar "
                              "vertex_indices\rend_header\r");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "The input specified an invalid type for property '" +
                    names[i] +
                    "' on element 'vertex' (must be 'float' or 'double')");
    }
  }
}

TEST(TriangleMeshReader, PositionMissing) {
  std::string names[] = {"x", "y", "z"};
  std::string properties[] = {"property float x\r", "property float y\r",
                              "property float z\r"};
  for (size_t i = 0; i < 3; i++) {
    std::string input_string = "ply\rformat ascii 1.0\relement vertex 1\r";
    for (size_t j = 0; j < 3; j++) {
      if (i != j) {
        input_string += properties[j];
      }
    }
    input_string +=
        "element face 1\rproperty list uchar uchar "
        "vertex_indices\rend_header\r";
    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;
    EXPECT_EQ(reader.ReadFrom(input).message(),
              "The input did not contain required property '" + names[i] +
                  "' on element 'vertex'");
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
          "ply\rformat ascii 1.0\relement vertex 1\rproperty float x\rproperty "
          "float y\rproperty float z\rproperty " +
          type + " " + name +
          "\relement face 1\rproperty list uchar uchar "
          "vertex_indices\rend_header\r");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "The input specified an invalid type for property '" + name +
                    "' on element 'vertex' (must be 'float' or 'double')");
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
          "ply\rformat ascii 1.0\relement vertex 1\rproperty float x\rproperty "
          "float y\rproperty float z\rproperty " +
          type + " " + name +
          "\relement face 1\rproperty list uchar uchar "
          "vertex_indices\rend_header\r");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "The input specified an invalid type for property '" + name +
                    "' on element 'vertex' (must be 'float' or 'double')");
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
          "ply\rformat ascii 1.0\relement vertex 1\rproperty float x\rproperty "
          "float y\rproperty float z\relement face 1\rproperty " +
          type + " " + name + "\rend_header\r");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;
      EXPECT_EQ(reader.ReadFrom(input).message(),
                "The input specified an invalid type for property "
                "'vertex_indices' on element 'face' (must be one of 'char', "
                "'uchar', 'short', 'ushort', 'int', or 'uint')");
    }
  }
}

TEST(TriangleMeshReader, VertexIndicesMissing) {
  std::stringstream input(
      "ply\rformat ascii 1.0\relement vertex 1\rproperty float x\rproperty "
      "float y\rproperty float z\relement face 1\rend_header\r");
  TestTriangleMeshReader<float, float, float, uint32_t> reader;
  EXPECT_EQ(reader.ReadFrom(input).message(),
            "The input did not contain required property 'vertex_indices' on "
            "element 'face'");
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
  std::string names[] = {"x", "y", "z"};

  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat " + Endianness() +
        " 1.0\relement vertex 1\rproperty double x\rproperty "
        "double y\rproperty double z\relement face 0\rproperty list uchar "
        "uchar vertex_indices\rend_header\r";
    for (size_t j = 0; j < 3; j++) {
      if (i == j) {
        input_string += InfiniteDouble();
      } else {
        input_string += ZeroDouble();
      }
    }

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "The input contained an invalid value for property '" + names[i] +
                  "' on element 'vertex' (must be finite)");
  }
}

TEST(TriangleMeshReader, CouldNotFitPosition) {
  std::string names[] = {"x", "y", "z"};
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

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "The input contained a value of property '" + names[i] +
                  "' on element 'vertex' that could not fit finitely into "
                  "destination type 'float'");
  }
}

TEST(TriangleMeshReader, OutOfRangeNormal) {
  std::string names[] = {"nx", "ny", "nz"};

  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat " + Endianness() +
        " 1.0\relement vertex 1\rproperty double x\rproperty "
        "double y\rproperty double z\rproperty double nx\rproperty "
        "double ny\rproperty double nz\relement face 0\rproperty list uchar "
        "uchar vertex_indices\rend_header\r" +
        ZeroDouble() + ZeroDouble() + ZeroDouble();

    for (size_t j = 0; j < 3; j++) {
      if (i == j) {
        input_string += InfiniteDouble();
      } else {
        input_string += ZeroDouble();
      }
    }

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "The input contained an invalid value for property '" + names[i] +
                  "' on element 'vertex' (must be finite)");
  }
}

TEST(TriangleMeshReader, CouldNotFitNormal) {
  std::string names[] = {"nx", "ny", "nz"};
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

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "The input contained a value of property '" + names[i] +
                  "' on element 'vertex' that could not fit finitely into "
                  "destination type 'float'");
  }
}

TEST(TriangleMeshReader, ValidatesNormalWhenMissing) {
  std::string names[] = {"nx", "ny", "nz"};

  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 1\rproperty double x\rproperty "
        "double y\rproperty double z\rproperty double " +
        names[i] +
        "\relement face 0\rproperty list uchar "
        "uchar vertex_indices\rend_header\r0.0 0.0 0.0 "
        "1000000000000000000000000000000000000000000000000000000000.0";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "The input contained a value of property '" + names[i] +
                  "' on element 'vertex' that could not fit finitely into "
                  "destination type 'float'");
  }
}

TEST(TriangleMeshReader, OutOfRangeUVs) {
  std::string u_names[] = {"texture_s", "texture_u", "s", "u"};
  std::string v_names[] = {"texture_t", "texture_v", "t", "v"};
  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      if (i == j) {
        continue;
      }

      std::string input_string =
          "ply\rformat " + Endianness() +
          " 1.0\relement vertex 1\rproperty double x\rproperty "
          "double y\rproperty double z\rproperty double " +
          u_names[i] +
          "\rproperty "
          "double " +
          v_names[j] +
          "\relement face 0\rproperty list uchar uchar "
          "vertex_indices\rend_header\r" +
          ZeroDouble() + ZeroDouble() + ZeroDouble();

      std::stringstream input_u(input_string + InfiniteDouble() + ZeroDouble());
      std::stringstream input_v(input_string + ZeroDouble() + InfiniteDouble());
      TestTriangleMeshReader<float, float, float, uint32_t> reader;

      EXPECT_EQ(reader.ReadFrom(input_u).message(),
                "The input contained an invalid value for property '" +
                    u_names[i] + "' on element 'vertex' (must be finite)");
      EXPECT_EQ(reader.ReadFrom(input_v).message(),
                "The input contained an invalid value for property '" +
                    v_names[j] + "' on element 'vertex' (must be finite)");
    }
  }
}

TEST(TriangleMeshReader, CouldNotFitUVs) {
  std::string u_names[] = {"texture_s", "texture_u", "s", "u"};
  std::string v_names[] = {"texture_t", "texture_v", "t", "v"};

  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      if (i == j) {
        continue;
      }

      std::string input_string =
          "ply\rformat ascii 1.0\relement vertex 1\rproperty double "
          "x\rproperty "
          "double y\rproperty double z\rproperty double " +
          u_names[i] +
          "\rproperty "
          "double " +
          v_names[j] +
          "\relement face 0\rproperty list uchar uchar "
          "vertex_indices\rend_header\r0.0 0.0 0.0 ";

      std::stringstream input_u(
          input_string +
          "1000000000000000000000000000000000000000000000000000000000.0 1.0 ");
      std::stringstream input_v(
          input_string +
          "1.0 1000000000000000000000000000000000000000000000000000000000.0");
      TestTriangleMeshReader<float, float, float, uint32_t> reader;

      EXPECT_EQ(reader.ReadFrom(input_u).message(),
                "The input contained a value of property '" + u_names[i] +
                    "' on element 'vertex' that could not fit finitely into "
                    "destination type 'float'");
      EXPECT_EQ(reader.ReadFrom(input_v).message(),
                "The input contained a value of property '" + v_names[j] +
                    "' on element 'vertex' that could not fit finitely into "
                    "destination type 'float'");
    }
  }
}

TEST(TriangleMeshReader, ValidatesUVsWhenMissing) {
  std::string names[] = {
      "texture_s", "texture_t", "texture_u", "texture_v", "s", "t", "u", "v",
  };

  for (size_t i = 0; i < 8; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 1\rproperty double x\rproperty "
        "double y\rproperty double z\rproperty double " +
        names[i] +
        "\relement face 0\rproperty list uchar "
        "uchar vertex_indices\rend_header\r0.0 0.0 0.0 "
        "1000000000000000000000000000000000000000000000000000000000.0";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "The input contained a value of property '" + names[i] +
                  "' on element 'vertex' that could not fit finitely into "
                  "destination type 'float'");
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
              "The input contained an invalid entry of property list "
              "'vertex_indices' on element 'face' (must be an index between 0 "
              "and the number of instances of element 'vertex')");
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
        input_string += "3";
      }

      if (j != 2) {
        input_string += " ";
      }
    }
    input_string += "\r";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint32_t> reader;

    EXPECT_EQ(reader.ReadFrom(input).message(),
              "The input contained an invalid entry of property list "
              "'vertex_indices' on element 'face' (must be an index between 0 "
              "and the number of instances of element 'vertex')");
  }
}

TEST(TriangleMeshReader, VertexIndexTooBigForIndexType) {
  for (size_t i = 0; i < 3; i++) {
    std::string input_string =
        "ply\rformat ascii 1.0\relement vertex 256\rproperty float x\rproperty "
        "float y\rproperty float z\relement face 1\rproperty list uchar ushort "
        "vertex_indices\rend_header\r";
    for (size_t j = 0; j < 256u; j++) {
      input_string += "0.0 0.0 0.0\r";
    }

    input_string += "3 ";
    for (size_t j = 0; j < 3; j++) {
      if (i != j) {
        input_string += std::to_string(j);
      } else {
        input_string += "256";
      }

      if (j != 2) {
        input_string += " ";
      }
    }
    input_string += "\r";

    std::stringstream input(input_string);
    TestTriangleMeshReader<float, float, float, uint8_t> reader;

    EXPECT_EQ(
        reader.ReadFrom(input).message(),
        "The input contained an entry of property list 'vertex_indices' on "
        "element 'face' that could not fit into destination type 'uchar'");
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