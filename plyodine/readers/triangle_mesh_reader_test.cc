#include "plyodine/readers/triangle_mesh_reader.h"

#include <array>
#include <sstream>
#include <vector>

#include "googletest/include/gtest/gtest.h"

template <std::floating_point LocationType, std::floating_point NormalType,
          std::floating_point UVType, std::integral FaceIndexType>
class TestTriangleMeshReader final
    : public plyodine::TriangleMeshReader<LocationType, NormalType, UVType,
                                          FaceIndexType> {
 public:
  void Start() override {
    positions.clear();
    normals.clear();
    uvs.clear();
    faces.clear();
  }

  void Handle(LocationType position[3], NormalType maybe_normals[3],
              UVType maybe_uv[2]) override {
    positions.push_back({position[0], position[1], position[2]});

    if (maybe_normals) {
      normals.push_back({maybe_normals[0], maybe_normals[1], maybe_normals[2]});
    }

    if (maybe_uv) {
      uvs.push_back({maybe_uv[0], maybe_uv[1], maybe_uv[2]});
    }
  }

  void Handle(FaceIndexType face[3]) override {
    faces.push_back({face[0], face[1], face[2]});
  }

  std::vector<std::array<LocationType, 3u>> positions;
  std::vector<std::array<NormalType, 3u>> normals;
  std::vector<std::array<UVType, 3u>> uvs;
  std::vector<std::array<FaceIndexType, 3u>> faces;
};

TEST(TriangleMeshReader, Empty) {
  std::stringstream input("ply\rformat ascii 1.0\rend_header\r");

  TestTriangleMeshReader<float, float, float, uint32_t> reader;
  EXPECT_EQ(reader.ReadFrom(input).error(),
            "Element vertex must have properties x, y, and z");
}