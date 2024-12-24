#include "plyodine/ply_writer.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <expected>
#include <functional>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <variant>
#include <vector>

#include "plyodine/error_code.h"
#include "plyodine/error_codes.h"

namespace plyodine {
namespace {

using ::plyodine::internal::MakeErrorCode;
using ::plyodine::internal::MakeUnexpected;

typedef std::tuple<std::vector<int8_t>, std::vector<uint8_t>,
                   std::vector<int16_t>, std::vector<uint16_t>,
                   std::vector<int32_t>, std::vector<uint32_t>,
                   std::vector<float>, std::vector<double>, std::stringstream>
    Context;

std::error_code ValidateName(const std::string& name) {
  if (name.empty()) {
    return MakeErrorCode(ErrorCode::WRITER_EMPTY_NAME_SPECIFIED);
  }

  for (char c : name) {
    if (!std::isgraph(c)) {
      return MakeErrorCode(ErrorCode::WRITER_NAME_CONTAINED_INVALID_CHARACTERS);
    }
  }

  return std::error_code();
}

bool ValidateComment(const std::string& comment) {
  for (char c : comment) {
    if (c == '\r' || c == '\n') {
      return false;
    }
  }

  return true;
}

template <std::integral SizeType, std::integral T>
std::error_code SerializeASCII(std::ostream& output, Context& context,
                               T value) {
  output << +value;
  if (!output) {
    return MakeErrorCode(ErrorCode::WRITER_WRITE_ERROR);
  }

  return std::error_code();
}

template <std::integral SizeType, std::floating_point T>
std::error_code SerializeASCII(std::ostream& output, Context& context,
                               T value) {
  if (!std::isfinite(value)) {
    return MakeErrorCode(ErrorCode::WRITER_ASCII_FLOAT_NOT_FINITE);
  }

  std::stringstream& storage = std::get<std::stringstream>(context);

  storage.str("");

  int log = static_cast<int>(std::log10(std::abs(value))) + 1;
  int num_digits = std::max(std::numeric_limits<T>::max_digits10 - log, 0);
  storage << std::fixed << std::setprecision(num_digits) << value;

  std::string_view result = storage.view();

  size_t dot = result.find(".");
  if (dot != std::string_view::npos) {
    result = result.substr(0u, result.find_last_not_of("0") + 1u);
    if (result.back() == '.') {
      result.remove_suffix(1u);
    }
  }

  output << result;
  if (!output) {
    return MakeErrorCode(ErrorCode::WRITER_WRITE_ERROR);
  }

  return std::error_code();
}

template <std::integral SizeType, typename T>
std::error_code SerializeASCII(std::ostream& output, Context& context,
                               std::span<const T> values) {
  if (std::numeric_limits<SizeType>::max() < values.size()) {
    return MakeErrorCode(ErrorCode::WRITER_LIST_INDEX_TOO_SMALL);
  }

  if (std::error_code error = SerializeASCII<SizeType, SizeType>(
          output, context, static_cast<SizeType>(values.size()));
      error) {
    return error;
  }

  for (const auto& entry : values) {
    output << ' ';
    if (!output) {
      return MakeErrorCode(ErrorCode::WRITER_WRITE_ERROR);
    }

    if (std::error_code error =
            SerializeASCII<SizeType>(output, context, entry);
        error) {
      return error;
    }
  }

  return std::error_code();
}

template <std::endian Endianness, std::integral SizeType, std::integral T>
std::error_code SerializeBinary(std::ostream& output, T value) {
  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  output.write(reinterpret_cast<char*>(&value), sizeof(value));
  if (!output) {
    return MakeErrorCode(ErrorCode::WRITER_WRITE_ERROR);
  }

  return std::error_code();
}

template <std::endian Endianness, std::integral SizeType, std::floating_point T>
std::error_code SerializeBinary(std::ostream& output, T value) {
  auto entry = std::bit_cast<
      std::conditional_t<std::is_same<T, float>::value, uint32_t, uintmax_t>>(
      value);

  if (Endianness != std::endian::native) {
    entry = std::byteswap(entry);
  }

  output.write(reinterpret_cast<char*>(&entry), sizeof(entry));
  if (!output) {
    return MakeErrorCode(ErrorCode::WRITER_WRITE_ERROR);
  }

  return std::error_code();
}

template <std::endian Endianness, std::integral SizeType, typename T>
std::error_code SerializeBinary(std::ostream& output,
                                std::span<const T> values) {
  if (std::numeric_limits<SizeType>::max() < values.size()) {
    return MakeErrorCode(ErrorCode::WRITER_LIST_INDEX_TOO_SMALL);
  }

  if (std::error_code error = SerializeBinary<Endianness, SizeType, SizeType>(
          output, static_cast<SizeType>(values.size()));
      error) {
    return error;
  }

  for (const auto& entry : values) {
    if (std::error_code error =
            SerializeBinary<Endianness, SizeType>(output, entry);
        error) {
      return error;
    }
  }

  return std::error_code();
}

template <typename T>
std::expected<T, std::error_code> CallCallback(
    const PlyWriter& ply_writer,
    std::expected<T, std::error_code> (PlyWriter::*callback)(
        const std::string&, size_t, const std::string&, size_t, uintmax_t)
        const,
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index, uintmax_t instance,
    Context& context) {
  return (ply_writer.*callback)(element_name, element_index, property_name,
                                property_index, instance);
}

template <typename T>
std::expected<std::span<const T>, std::error_code> CallCallback(
    const PlyWriter& ply_writer,
    std::expected<std::span<const T>, std::error_code> (PlyWriter::*callback)(
        const std::string&, size_t, const std::string&, size_t, uintmax_t,
        std::vector<T>&) const,
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index, uintmax_t instance,
    Context& context) {
  auto& vector = std::get<std::vector<T>>(context);
  vector.clear();
  return (ply_writer.*callback)(element_name, element_index, property_name,
                                property_index, instance, vector);
}

static const std::array<std::string, 16> kTypeNames = {
    "char", "char", "uchar", "uchar", "short", "short", "ushort", "ushort",
    "int",  "int",  "uint",  "uint",  "float", "float", "double", "double"};

void PropertyString(std::ostream& output, size_t type,
                    const std::string& name) {
  output << "property " << kTypeNames.at(type) << " " << name << "\r";
}

void PropertyListString(std::ostream& output, size_t list_type,
                        size_t data_type, const std::string& name) {
  static const std::array<std::string, 16> kListTypeNames = {"uchar", "ushort",
                                                             "uint"};

  output << "property list " << kListTypeNames.at(list_type) << " "
         << kTypeNames.at(data_type) << " " << name << "\r";
}

class PropertyWriterBase {
 public:
  virtual std::error_code Write(uintmax_t instance) = 0;
};

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
class PropertyWriter final : public PropertyWriterBase {
 public:
  PropertyWriter(std::ostream& output, const PlyWriter& ply_writer, T callback,
                 const std::string& element_name, size_t element_index,
                 const std::string& property_name, size_t property_index,
                 Context& context)
      : output_(output),
        ply_writer_(ply_writer),
        callback_(callback),
        element_name_(element_name),
        element_index_(element_index),
        property_name_(property_name),
        property_index_(property_index),
        context_(context) {}

  std::error_code Write(uintmax_t instance) override;

 private:
  std::ostream& output_;
  const PlyWriter& ply_writer_;
  T callback_;
  const std::string& element_name_;
  size_t element_index_;
  const std::string& property_name_;
  size_t property_index_;
  Context& context_;
  size_t value_type_index_;
  std::optional<size_t> list_size_type_;
};

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
std::error_code PropertyWriter<Ascii, Endianness, SizeType, T>::Write(
    uintmax_t instance) {
  auto result =
      CallCallback(ply_writer_, callback_, element_name_, element_index_,
                   property_name_, property_index_, instance, context_);
  if (!result) {
    return result.error();
  }

  if constexpr (Ascii) {
    if (std::error_code error =
            SerializeASCII<SizeType>(output_, context_, *result);
        error) {
      return error;
    }
  } else {
    if (std::error_code error =
            SerializeBinary<Endianness, SizeType>(output_, *result);
        error) {
      return error;
    }
  }

  if (!output_) {
    return MakeErrorCode(ErrorCode::WRITER_WRITE_ERROR);
  }

  return std::error_code();
}

template <bool Ascii, std::endian Endianness, typename T>
std::expected<std::unique_ptr<PropertyWriterBase>, std::error_code>
BuildPropertyWriter(std::ostream& output, const PlyWriter& ply_writer,
                    T callback, const std::string& element_name,
                    size_t element_index, const std::string& property_name,
                    size_t property_index, Context& context,
                    size_t value_type_index,
                    std::function<std::expected<size_t, std::error_code>(
                        const std::string&, size_t, const std::string&, size_t)>
                        get_property_list_size_type) {
  using R = std::decay_t<decltype(*CallCallback(
      ply_writer, callback, element_name, element_index, property_name,
      property_index, 0u, context))>;

  if (std::error_code error = ValidateName(property_name); error) {
    return std::unexpected(error);
  }

  std::optional<size_t> maybe_list_size_type;
  if constexpr (std::is_class<R>::value) {
    auto list_size_type = get_property_list_size_type(
        element_name, element_index, property_name, property_index);
    if (!list_size_type) {
      return std::unexpected(list_size_type.error());
    }

    PropertyListString(output, *list_size_type, value_type_index,
                       property_name);
    maybe_list_size_type = *list_size_type;
  } else {
    PropertyString(output, value_type_index, property_name);
  }

  if (!output) {
    return MakeUnexpected(ErrorCode::WRITER_WRITE_ERROR);
  }

  if constexpr (std::is_class<R>::value) {
    if (*maybe_list_size_type == 0) {
      return std::make_unique<PropertyWriter<Ascii, Endianness, uint8_t, T>>(
          output, ply_writer, callback, element_name, element_index,
          property_name, property_index, context);
    }

    if (*maybe_list_size_type == 1) {
      return std::make_unique<PropertyWriter<Ascii, Endianness, uint16_t, T>>(
          output, ply_writer, callback, element_name, element_index,
          property_name, property_index, context);
    }
  }

  return std::make_unique<PropertyWriter<Ascii, Endianness, uint32_t, T>>(
      output, ply_writer, callback, element_name, element_index, property_name,
      property_index, context);
}

template <bool Ascii, std::endian Endianness, typename T>
std::expected<std::vector<std::pair<
                  uintmax_t, std::vector<std::unique_ptr<PropertyWriterBase>>>>,
              std::error_code>
WriteHeader(std::ostream& output, const PlyWriter& ply_writer,
            const std::function<std::expected<size_t, std::error_code>(
                const std::string&, size_t, const std::string&, size_t)>&
                get_property_list_size_type,
            const char* format,
            std::map<std::string, uintmax_t>& num_element_instances,
            const std::map<std::string, std::map<std::string, T>>& callbacks,
            const std::vector<std::string>& comments,
            const std::vector<std::string>& object_info, Context& context) {
  output << "ply\rformat " << format << " 1.0\r";
  if (!output) {
    return MakeUnexpected(ErrorCode::WRITER_WRITE_ERROR);
  }

  for (const auto& comment : comments) {
    if (!ValidateComment(comment)) {
      return MakeUnexpected(ErrorCode::WRITER_COMMENT_CONTAINS_NEWLINE);
    }

    output << "comment " << comment << "\r";
    if (!output) {
      return MakeUnexpected(ErrorCode::WRITER_WRITE_ERROR);
    }
  }

  for (const auto& info : object_info) {
    if (!ValidateComment(info)) {
      return MakeUnexpected(ErrorCode::WRITER_OBJ_INFO_CONTAINS_NEWLINE);
    }

    output << "obj_info " << info << "\r";
    if (!output) {
      return MakeUnexpected(ErrorCode::WRITER_WRITE_ERROR);
    }
  }

  std::vector<
      std::pair<uintmax_t, std::vector<std::unique_ptr<PropertyWriterBase>>>>
      actual_callbacks;
  for (const auto& element : callbacks) {
    if (std::error_code error = ValidateName(element.first); error) {
      return std::unexpected(error);
    }

    output << "element " << element.first << " "
           << num_element_instances[element.first] << "\r";
    if (!output) {
      return MakeUnexpected(ErrorCode::WRITER_WRITE_ERROR);
    }

    std::vector<std::unique_ptr<PropertyWriterBase>> row;
    for (const auto& property : element.second) {
      auto write_callback = std::visit(
          [&](auto callback) {
            return BuildPropertyWriter<Ascii, Endianness>(
                output, ply_writer, callback, element.first,
                actual_callbacks.size(), property.first, row.size(), context,
                property.second.index(), get_property_list_size_type);
          },
          property.second);
      if (!write_callback) {
        return std::unexpected(write_callback.error());
      }

      row.emplace_back(std::move(*write_callback));
    }
    actual_callbacks.emplace_back(num_element_instances.at(element.first),
                                  std::move(row));
  }

  output << "end_header\r";
  if (!output) {
    return MakeUnexpected(ErrorCode::WRITER_WRITE_ERROR);
  }

  return actual_callbacks;
}

template <std::endian Endianness, typename T>
std::error_code WriteToBinaryImpl(
    std::ostream& output, const PlyWriter& ply_writer,
    const std::function<std::expected<size_t, std::error_code>(
        const std::string&, size_t, const std::string&, size_t)>&
        get_property_list_size_type,
    std::map<std::string, uintmax_t>& num_element_instances,
    const std::map<std::string, std::map<std::string, T>>& callbacks,
    const std::vector<std::string>& comments,
    const std::vector<std::string>& object_info) {
  const char* format;
  if constexpr (Endianness == std::endian::big) {
    format = "binary_big_endian";
  } else {
    format = "binary_little_endian";
  }

  Context context;
  auto actual_callbacks = WriteHeader<false, Endianness>(
      output, ply_writer, get_property_list_size_type, format,
      num_element_instances, callbacks, comments, object_info, context);
  if (!actual_callbacks) {
    return actual_callbacks.error();
  }

  for (const auto& element : *actual_callbacks) {
    for (uintmax_t instance = 0; instance < element.first; instance++) {
      for (const auto& property_writer : element.second) {
        if (std::error_code error = property_writer->Write(instance); error) {
          return error;
        }
      }
    }
  }

  return std::error_code();
}

}  // namespace

std::error_code PlyWriter::WriteTo(std::ostream& stream) const {
  if constexpr (std::endian::native == std::endian::big) {
    return WriteToBigEndian(stream);
  } else {
    return WriteToLittleEndian(stream);
  }
}

std::error_code PlyWriter::WriteToASCII(std::ostream& stream) const {
  if (stream.fail()) {
    return MakeErrorCode(ErrorCode::BAD_STREAM);
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, Callback>> callbacks;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error =
          Start(num_element_instances, callbacks, comments, object_info);
      error) {
    return error;
  }

  Context context;
  auto actual_callbacks = WriteHeader<true, std::endian::native>(
      stream, *this,
      [&](const std::string& element_name, size_t element_index,
          const std::string& property_name,
          size_t property_index) -> std::expected<size_t, std::error_code> {
        auto result = this->GetPropertyListSizeType(
            element_name, element_index, property_name, property_index);
        if (!result) {
          return std::unexpected(result.error());
        }
        return static_cast<size_t>(*result);
      },
      "ascii", num_element_instances, callbacks, comments, object_info,
      context);
  if (!actual_callbacks) {
    return actual_callbacks.error();
  }

  for (const auto& element : *actual_callbacks) {
    for (uintmax_t instance = 0; instance < element.first; instance++) {
      bool first = true;
      for (const auto& property_writer : element.second) {
        if (!first) {
          stream << ' ';
          if (!stream) {
            return MakeErrorCode(ErrorCode::WRITER_WRITE_ERROR);
          }
        }

        first = false;

        if (std::error_code error = property_writer->Write(instance); error) {
          return error;
        }
      }

      stream << '\r';
      if (!stream) {
        return MakeErrorCode(ErrorCode::WRITER_WRITE_ERROR);
      }
    }
  }

  return std::error_code();
}

std::error_code PlyWriter::WriteToBigEndian(std::ostream& stream) const {
  if (stream.fail()) {
    return MakeErrorCode(ErrorCode::BAD_STREAM);
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, Callback>> callbacks;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error =
          Start(num_element_instances, callbacks, comments, object_info);
      error) {
    return error;
  }

  return WriteToBinaryImpl<std::endian::big>(
      stream, *this,
      [&](const std::string& element_name, size_t element_index,
          const std::string& property_name,
          size_t property_index) -> std::expected<size_t, std::error_code> {
        auto result = this->GetPropertyListSizeType(
            element_name, element_index, property_name, property_index);
        if (!result) {
          return std::unexpected(result.error());
        }
        return static_cast<size_t>(*result);
      },
      num_element_instances, callbacks, comments, object_info);
}

std::error_code PlyWriter::WriteToLittleEndian(std::ostream& stream) const {
  if (stream.fail()) {
    return MakeErrorCode(ErrorCode::BAD_STREAM);
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, Callback>> callbacks;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error =
          Start(num_element_instances, callbacks, comments, object_info);
      error) {
    return error;
  }

  return WriteToBinaryImpl<std::endian::little>(
      stream, *this,
      [&](const std::string& element_name, size_t element_index,
          const std::string& property_name,
          size_t property_index) -> std::expected<size_t, std::error_code> {
        auto result = this->GetPropertyListSizeType(
            element_name, element_index, property_name, property_index);
        if (!result) {
          return std::unexpected(result.error());
        }
        return static_cast<size_t>(*result);
      },
      num_element_instances, callbacks, comments, object_info);
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine