#include "plyodine/ply_reader.h"

#include <bit>
#include <cassert>
#include <charconv>
#include <limits>
#include <sstream>
#include <type_traits>

#include "plyodine/ply_header.h"

namespace plyodine {
namespace {

struct Context {
  PlyReader* reader_;
  std::vector<int8_t> int8_;
  std::vector<uint8_t> uint8_;
  std::vector<int16_t> int16_;
  std::vector<uint16_t> uint16_;
  std::vector<int32_t> int32_;
  std::vector<uint32_t> uint32_;
  std::vector<float> float_;
  std::vector<double> double_;
  std::string token_;
};

std::string_view UnexpectedEOF() { return "Unexpected EOF"; }

std::string_view NegativeListSize() {
  return "The input contained a property list with a negative size";
}

template <std::endian Endianness, typename T, typename ReadType = T>
std::expected<void, std::string_view> ReadBinaryPropertyDataImpl(
    std::istream& input, std::vector<T>& output) {
  static_assert(sizeof(T) == sizeof(ReadType));

  if constexpr (std::endian::native == Endianness) {
    T data;
    if (!input.read(reinterpret_cast<char*>(&data), sizeof(T))) {
      return std::unexpected(UnexpectedEOF());
    }

    output.push_back(data);
  } else {
    ReadType data;
    if (!input.read(reinterpret_cast<char*>(&data), sizeof(T))) {
      return std::unexpected(UnexpectedEOF());
    }

    data = std::byteswap(data);

    if constexpr (std::is_same<T, ReadType>::value) {
      output.push_back(data);
    } else {
      output.push_back(std::bit_cast<T>(data));
    }
  }

  return std::expected<void, std::string_view>();
}

template <std::endian Endianness, typename T>
std::expected<size_t, std::string_view> ReadBinaryListSizeImpl(
    std::istream& input) {
  T result;

  if (!input.read(reinterpret_cast<char*>(&result), sizeof(T))) {
    return std::unexpected(UnexpectedEOF());
  }

  if constexpr (std::endian::native != Endianness) {
    result = std::byteswap(result);
  }

  if constexpr (std::is_signed<T>::value) {
    if (result < 0) {
      return std::unexpected(NegativeListSize());
    }
  }

  return result;
}

template <std::endian Endianness, typename T, typename ReadType = T>
std::expected<void, std::string_view> ReadBinaryPropertyScalarData(
    std::istream& input, std::string_view element_name,
    std::string_view property_name, size_t property_index,
    std::vector<T>& storage, PlyReader* reader) {
  storage.clear();

  auto result =
      ReadBinaryPropertyDataImpl<Endianness, T, ReadType>(input, storage);
  if (!result) {
    return result;
  }

  return reader->Handle(element_name, property_name, property_index,
                        storage[0]);
}

template <std::endian Endianness>
std::expected<void, std::string_view> ReadBinaryPropertyScalar(
    std::istream& input, std::string_view element_name,
    const PlyHeader::Property& header_property, size_t property_index,
    Context& context) {
  std::expected<void, std::string_view> result;

  switch (header_property.data_type) {
    case PlyHeader::Property::INT8:
      result = ReadBinaryPropertyScalarData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.int8_, context.reader_);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadBinaryPropertyScalarData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.uint8_, context.reader_);
      break;
    case PlyHeader::Property::INT16:
      result = ReadBinaryPropertyScalarData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.int16_, context.reader_);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadBinaryPropertyScalarData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.uint16_, context.reader_);
      break;
    case PlyHeader::Property::INT32:
      result = ReadBinaryPropertyScalarData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.int32_, context.reader_);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadBinaryPropertyScalarData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.uint32_, context.reader_);
      break;
    case PlyHeader::Property::FLOAT:
      result = ReadBinaryPropertyScalarData<Endianness, float, uint32_t>(
          input, element_name, header_property.name, property_index,
          context.float_, context.reader_);
      break;
    case PlyHeader::Property::DOUBLE:
      result = ReadBinaryPropertyScalarData<Endianness, double, uint64_t>(
          input, element_name, header_property.name, property_index,
          context.double_, context.reader_);
      break;
  }

  return result;
}

template <std::endian Endianness>
std::expected<size_t, std::string_view> ReadBinaryListSize(
    std::istream& input, PlyHeader::Property::Type type) {
  std::expected<size_t, std::string_view> result;

  switch (type) {
    case PlyHeader::Property::INT8:
      result = ReadBinaryListSizeImpl<Endianness, int8_t>(input);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadBinaryListSizeImpl<Endianness, uint8_t>(input);
      break;
    case PlyHeader::Property::INT16:
      result = ReadBinaryListSizeImpl<Endianness, int16_t>(input);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadBinaryListSizeImpl<Endianness, uint16_t>(input);
      break;
    case PlyHeader::Property::INT32:
      result = ReadBinaryListSizeImpl<Endianness, int32_t>(input);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadBinaryListSizeImpl<Endianness, uint32_t>(input);
      break;
    default:
      assert(false);
  }

  return result;
}

template <std::endian Endianness, typename T, typename ReadType = T>
std::expected<void, std::string_view> ReadBinaryPropertyListData(
    std::istream& input, std::string_view element_name,
    std::string_view property_name, size_t property_index,
    std::vector<T>& storage, size_t num_to_read, PlyReader* reader) {
  storage.clear();

  for (size_t i = 0; i < num_to_read; i++) {
    auto result =
        ReadBinaryPropertyDataImpl<Endianness, T, ReadType>(input, storage);
    if (!result) {
      return result;
    }
  }

  return reader->Handle(element_name, property_name, property_index, storage);
}

template <std::endian Endianness>
std::expected<void, std::string_view> ReadBinaryPropertyList(
    std::istream& input, std::string_view element_name,
    const PlyHeader::Property& header_property, size_t property_index,
    Context& context) {
  auto num_to_read =
      ReadBinaryListSize<Endianness>(input, *header_property.list_type);
  if (!num_to_read) {
    return std::unexpected(num_to_read.error());
  }

  std::expected<void, std::string_view> result;
  switch (header_property.data_type) {
    case PlyHeader::Property::INT8:
      result = ReadBinaryPropertyListData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.int8_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadBinaryPropertyListData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.uint8_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::INT16:
      result = ReadBinaryPropertyListData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.int16_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadBinaryPropertyListData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.uint16_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::INT32:
      result = ReadBinaryPropertyListData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.int32_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadBinaryPropertyListData<Endianness>(
          input, element_name, header_property.name, property_index,
          context.uint32_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::FLOAT:
      result = ReadBinaryPropertyListData<Endianness, float, uint32_t>(
          input, element_name, header_property.name, property_index,
          context.float_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::DOUBLE:
      result = ReadBinaryPropertyListData<Endianness, double, uint64_t>(
          input, element_name, header_property.name, property_index,
          context.double_, *num_to_read, context.reader_);
      break;
  }

  return result;
}

template <std::endian Endianness>
std::expected<void, std::string_view> ReadBinaryData(std::istream& input,
                                                     const PlyHeader& header,
                                                     Context& context) {
  size_t property_index = 0;
  for (const auto& element : header.elements) {
    for (uint64_t e = 0; e < element.num_in_file; e++) {
      for (size_t p = 0; p < element.properties.size(); p++) {
        if (element.properties[p].list_type) {
          auto error = ReadBinaryPropertyList<Endianness>(
              input, element.name, element.properties[p], property_index + p,
              context);
          if (!error) {
            return error;
          }
        } else {
          auto error = ReadBinaryPropertyScalar<Endianness>(
              input, element.name, element.properties[p], property_index + p,
              context);
          if (!error) {
            return error;
          }
        }
      }
    }
    property_index += element.properties.size();
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ReadNextAsciiToken(std::istream& input,
                                                         std::string& token) {
  token.clear();

  char c;
  while (input.get(c)) {
    if (c == ' ') {
      break;
    }

    token.push_back(c);
  }

  if (token.empty()) {
    return std::unexpected("The input contained an empty token");
  }

  return std::expected<void, std::string_view>();
}

template <typename T>
std::expected<void, std::string_view> ReadAsciiPropertyDataImpl(
    std::istream& input, std::string& token, std::vector<T>& output) {
  auto success = ReadNextAsciiToken(input, token);
  if (!success) {
    return std::unexpected(success.error());
  }

  T data;
  auto parsing_result =
      std::from_chars(token.data(), token.data() + token.size(), data);
  if (parsing_result.ec == std::errc::result_out_of_range) {
    return std::unexpected(
        "The input contained a property entry that was out of range");
  } else if (parsing_result.ec != std::errc{}) {
    return std::unexpected("The input contained an unparsable property entry");
  }

  output.push_back(data);

  return std::expected<void, std::string_view>();
}

template <typename T>
std::expected<size_t, std::string_view> ReadAsciiListSizeImpl(
    std::istream& input, std::string& token) {
  auto success = ReadNextAsciiToken(input, token);
  if (!success) {
    return std::unexpected(success.error());
  }

  T result;
  auto parsing_result =
      std::from_chars(token.data(), token.data() + token.size(), result);
  if (parsing_result.ec == std::errc::result_out_of_range) {
    return std::unexpected(
        "The input contained a property list size that was out of range");
  } else if (parsing_result.ec != std::errc{}) {
    return std::unexpected(
        "The input contained an unparsable property list size");
  }

  if constexpr (std::is_signed<T>::value) {
    if (result < 0) {
      return std::unexpected(NegativeListSize());
    }
  }

  return result;
}

template <typename T>
std::expected<void, std::string_view> ReadAsciiPropertyScalarData(
    std::istream& input, std::string_view element_name,
    std::string_view property_name, size_t property_index, std::string& token,
    std::vector<T>& storage, PlyReader* reader) {
  storage.clear();

  auto result = ReadAsciiPropertyDataImpl(input, token, storage);
  if (!result) {
    return result;
  }

  return reader->Handle(element_name, property_name, property_index,
                        storage[0]);
}

std::expected<void, std::string_view> ReadAsciiPropertyScalar(
    std::istream& input, std::string_view element_name,
    const PlyHeader::Property& header_property, size_t property_index,
    Context& context) {
  std::expected<void, std::string_view> result;

  switch (header_property.data_type) {
    case PlyHeader::Property::INT8:
      result = ReadAsciiPropertyScalarData(
          input, element_name, header_property.name, property_index,
          context.token_, context.int8_, context.reader_);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadAsciiPropertyScalarData(
          input, element_name, header_property.name, property_index,
          context.token_, context.uint8_, context.reader_);
      break;
    case PlyHeader::Property::INT16:
      result = ReadAsciiPropertyScalarData(
          input, element_name, header_property.name, property_index,
          context.token_, context.int16_, context.reader_);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadAsciiPropertyScalarData(
          input, element_name, header_property.name, property_index,
          context.token_, context.uint16_, context.reader_);
      break;
    case PlyHeader::Property::INT32:
      result = ReadAsciiPropertyScalarData(
          input, element_name, header_property.name, property_index,
          context.token_, context.int32_, context.reader_);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadAsciiPropertyScalarData(
          input, element_name, header_property.name, property_index,
          context.token_, context.uint32_, context.reader_);
      break;
    case PlyHeader::Property::FLOAT:
      result = ReadAsciiPropertyScalarData(
          input, element_name, header_property.name, property_index,
          context.token_, context.float_, context.reader_);
      break;
    case PlyHeader::Property::DOUBLE:
      result = ReadAsciiPropertyScalarData(
          input, element_name, header_property.name, property_index,
          context.token_, context.double_, context.reader_);
      break;
  }

  return result;
}

std::expected<size_t, std::string_view> ReadAsciiListSize(
    std::istream& input, std::string& token, PlyHeader::Property::Type type) {
  std::expected<size_t, std::string_view> result;

  switch (type) {
    case PlyHeader::Property::INT8:
      result = ReadAsciiListSizeImpl<int8_t>(input, token);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadAsciiListSizeImpl<uint8_t>(input, token);
      break;
    case PlyHeader::Property::INT16:
      result = ReadAsciiListSizeImpl<int16_t>(input, token);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadAsciiListSizeImpl<uint16_t>(input, token);
      break;
    case PlyHeader::Property::INT32:
      result = ReadAsciiListSizeImpl<int32_t>(input, token);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadAsciiListSizeImpl<uint32_t>(input, token);
      break;
    default:
      assert(false);
  }

  return result;
}

template <typename T>
std::expected<void, std::string_view> ReadAsciiPropertyListData(
    std::istream& input, std::string_view element_name,
    std::string_view property_name, size_t property_index, std::string& token,
    std::vector<T>& storage, size_t num_to_read, PlyReader* reader) {
  storage.clear();

  for (size_t i = 0; i < num_to_read; i++) {
    auto result = ReadAsciiPropertyDataImpl<T>(input, token, storage);
    if (!result) {
      return result;
    }
  }

  return reader->Handle(element_name, property_name, property_index, storage);
}

std::expected<void, std::string_view> ReadAsciiPropertyList(
    std::istream& input, std::string_view element_name,
    const PlyHeader::Property& header_property, size_t property_index,
    Context& context) {
  auto num_to_read =
      ReadAsciiListSize(input, context.token_, *header_property.list_type);
  if (!num_to_read) {
    return std::unexpected(num_to_read.error());
  }

  std::expected<void, std::string_view> result;
  switch (header_property.data_type) {
    case PlyHeader::Property::INT8:
      result = ReadAsciiPropertyListData(
          input, element_name, header_property.name, property_index,
          context.token_, context.int8_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadAsciiPropertyListData(
          input, element_name, header_property.name, property_index,
          context.token_, context.uint8_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::INT16:
      result = ReadAsciiPropertyListData(
          input, element_name, header_property.name, property_index,
          context.token_, context.int16_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadAsciiPropertyListData(
          input, element_name, header_property.name, property_index,
          context.token_, context.uint16_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::INT32:
      result = ReadAsciiPropertyListData(
          input, element_name, header_property.name, property_index,
          context.token_, context.int32_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadAsciiPropertyListData(
          input, element_name, header_property.name, property_index,
          context.token_, context.uint32_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::FLOAT:
      result = ReadAsciiPropertyListData(
          input, element_name, header_property.name, property_index,
          context.token_, context.float_, *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::DOUBLE:
      result = ReadAsciiPropertyListData(
          input, element_name, header_property.name, property_index,
          context.token_, context.double_, *num_to_read, context.reader_);
      break;
  }

  return result;
}

std::expected<void, std::string_view> ReadAsciiData(std::istream& input,
                                                    const PlyHeader& header,
                                                    Context& context) {
  std::stringstream line;

  size_t property_index = 0;
  for (const auto& element : header.elements) {
    for (uint64_t e = 0; e < element.num_in_file; e++) {
      line.str("");
      line.clear();

      char c;
      while (input.get(c)) {
        if (c == header.line_ending[0]) {
          auto line_ending = header.line_ending;
          line_ending.remove_prefix(1);

          while (!line_ending.empty() && input.get(c)) {
            if (c != line_ending[0]) {
              return std::unexpected(
                  "The input contained mismatched line endings");
            }
            line_ending.remove_prefix(1);
          }

          break;
        }

        if (c != ' ' && !std::isgraph(c)) {
          return std::unexpected("The input contained an invalid character");
        }

        line.put(c);
      }

      for (size_t p = 0; p < element.properties.size(); p++) {
        if (element.properties[p].list_type) {
          auto error =
              ReadAsciiPropertyList(line, element.name, element.properties[p],
                                    property_index + p, context);
          if (!error) {
            return error;
          }
        } else {
          auto error =
              ReadAsciiPropertyScalar(line, element.name, element.properties[p],
                                      property_index + p, context);
          if (!error) {
            return error;
          }
        }
      }

      while (line.get(c)) {
        if (std::isgraph(c)) {
          return std::unexpected(
              "The input contained an element with unused tokens");
        }
      }
    }
    property_index += element.properties.size();
  }

  return std::expected<void, std::string_view>();
}

}  // namespace

std::expected<void, std::string_view> PlyReader::ReadFrom(std::istream& input) {
  auto header = ReadPlyHeader(input);
  if (!header) {
    return std::unexpected(header.error());
  }

  size_t property_index = 0;
  std::unordered_map<
      std::string_view,
      std::unordered_map<std::string_view, std::pair<size_t, Property::Type>>>
      all_properties;
  for (const auto& element : header->elements) {
    std::unordered_map<std::string_view, std::pair<size_t, Property::Type>>
        properties;
    for (const auto& property : element.properties) {
      if (property.list_type) {
        switch (property.data_type) {
          case PlyHeader::Property::INT8:
            properties[property.name] =
                std::make_pair(property_index++, Property::INT8_LIST);
            break;
          case PlyHeader::Property::UINT8:
            properties[property.name] =
                std::make_pair(property_index++, Property::UINT8_LIST);
            break;
          case PlyHeader::Property::INT16:
            properties[property.name] =
                std::make_pair(property_index++, Property::INT16_LIST);
            break;
          case PlyHeader::Property::UINT16:
            properties[property.name] =
                std::make_pair(property_index++, Property::UINT16_LIST);
            break;
          case PlyHeader::Property::INT32:
            properties[property.name] =
                std::make_pair(property_index++, Property::INT32_LIST);
            break;
          case PlyHeader::Property::UINT32:
            properties[property.name] =
                std::make_pair(property_index++, Property::UINT32_LIST);
            break;
          case PlyHeader::Property::FLOAT:
            properties[property.name] =
                std::make_pair(property_index++, Property::FLOAT_LIST);
            break;
          case PlyHeader::Property::DOUBLE:
            properties[property.name] =
                std::make_pair(property_index++, Property::DOUBLE_LIST);
            break;
        }
      } else {
        switch (property.data_type) {
          case PlyHeader::Property::INT8:
            properties[property.name] =
                std::make_pair(property_index++, Property::INT8);
            break;
          case PlyHeader::Property::UINT8:
            properties[property.name] =
                std::make_pair(property_index++, Property::UINT8);
            break;
          case PlyHeader::Property::INT16:
            properties[property.name] =
                std::make_pair(property_index++, Property::INT16);
            break;
          case PlyHeader::Property::UINT16:
            properties[property.name] =
                std::make_pair(property_index++, Property::UINT16);
            break;
          case PlyHeader::Property::INT32:
            properties[property.name] =
                std::make_pair(property_index++, Property::INT32);
            break;
          case PlyHeader::Property::UINT32:
            properties[property.name] =
                std::make_pair(property_index++, Property::UINT32);
            break;
          case PlyHeader::Property::FLOAT:
            properties[property.name] =
                std::make_pair(property_index++, Property::FLOAT);
            break;
          case PlyHeader::Property::DOUBLE:
            properties[property.name] =
                std::make_pair(property_index++, Property::DOUBLE);
            break;
        }
      }
    }
    all_properties[element.name] = properties;
  }

  Start(all_properties, header->comments, header->object_info);

  Context context = {this};

  std::expected<void, std::string_view> result;
  switch (header->format) {
    case PlyHeader::ASCII:
      result = ReadAsciiData(input, *header, context);
      break;
    case PlyHeader::BINARY_BIG_ENDIAN:
      result = ReadBinaryData<std::endian::big>(input, *header, context);
      break;
    case PlyHeader::BINARY_LITTLE_ENDIAN:
      result = ReadBinaryData<std::endian::little>(input, *header, context);
      break;
  }

  return result;
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine