#include <fstream>
#include <iostream>
#include <variant>

#include "plyodine/ply_reader.h"

class Reader : public plyodine::PlyReader {
 public:
  std::error_code Start(
      std::map<std::string, uintmax_t> num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
      std::vector<std::string> comments,
      std::vector<std::string> object_info) override;
};

template <typename T>
void UpdateCallback(std::function<std::error_code(T)>& callback) {
  callback = [](T) { return std::error_code(); };
}

std::error_code Reader::Start(
    std::map<std::string, uintmax_t> num_element_instances,
    std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
    std::vector<std::string> comments, std::vector<std::string> object_info) {
  for (auto& elements : callbacks) {
    for (auto& property : elements.second) {
      std::visit([](auto& callback) { UpdateCallback(callback); },
                 property.second);
    }
  }

  return std::error_code();
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: ply_validator <filename>" << std::endl;
    return EXIT_FAILURE;
  }

  std::ifstream file(argv[1], std::ios_base::in | std::ios_base::binary);
  if (!file) {
    std::cerr << "failed to open file" << std::endl;
    return EXIT_FAILURE;
  }

  Reader reader;
  if (std::error_code error = reader.ReadFrom(file); error) {
    std::cerr << error.message() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}