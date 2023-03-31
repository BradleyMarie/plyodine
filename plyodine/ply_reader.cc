#include "plyodine/ply_reader.h"

#include <bit>
#include <cassert>
#include <charconv>
#include <limits>
#include <sstream>
#include <tuple>
#include <type_traits>

#include "plyodine/ply_header.h"

namespace plyodine {
namespace {

struct Context {
  PlyReader* reader_;
  std::vector<std::tuple<
      uint64_t, std::string_view,
      std::vector<std::tuple<std::string_view, PlyReader::Callback>>>>
      callbacks_;
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

template <std::endian Endianness, size_t Index, typename T,
          typename ReadType = T>
std::expected<void, std::string_view> ReadBinaryPropertyScalarData(
    std::istream& input, std::string_view element_name,
    std::string_view property_name, PlyReader::Callback callback,
    std::vector<T>& storage, PlyReader* reader) {
  storage.clear();

  auto result =
      ReadBinaryPropertyDataImpl<Endianness, T, ReadType>(input, storage);
  if (!result) {
    return result;
  }

  auto actual_callback = std::get<Index>(callback);
  if (actual_callback == nullptr) {
    return std::expected<void, std::string_view>();
  }

  return (reader->*actual_callback)(element_name, property_name, storage[0]);
}

template <std::endian Endianness>
std::expected<void, std::string_view> ReadBinaryPropertyScalar(
    std::istream& input, std::string_view element_name,
    const PlyHeader::Property& header_property, PlyReader::Callback callback,
    Context& context) {
  std::expected<void, std::string_view> result;

  switch (header_property.data_type) {
    case PlyHeader::Property::INT8:
      result = ReadBinaryPropertyScalarData<Endianness, Property::INT8>(
          input, element_name, header_property.name, callback, context.int8_,
          context.reader_);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadBinaryPropertyScalarData<Endianness, Property::UINT8>(
          input, element_name, header_property.name, callback, context.uint8_,
          context.reader_);
      break;
    case PlyHeader::Property::INT16:
      result = ReadBinaryPropertyScalarData<Endianness, Property::INT16>(
          input, element_name, header_property.name, callback, context.int16_,
          context.reader_);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadBinaryPropertyScalarData<Endianness, Property::UINT16>(
          input, element_name, header_property.name, callback, context.uint16_,
          context.reader_);
      break;
    case PlyHeader::Property::INT32:
      result = ReadBinaryPropertyScalarData<Endianness, Property::INT32>(
          input, element_name, header_property.name, callback, context.int32_,
          context.reader_);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadBinaryPropertyScalarData<Endianness, Property::UINT32>(
          input, element_name, header_property.name, callback, context.uint32_,
          context.reader_);
      break;
    case PlyHeader::Property::FLOAT:
      result = ReadBinaryPropertyScalarData<Endianness, Property::FLOAT, float,
                                            uint32_t>(
          input, element_name, header_property.name, callback, context.float_,
          context.reader_);
      break;
    case PlyHeader::Property::DOUBLE:
      result = ReadBinaryPropertyScalarData<Endianness, Property::DOUBLE,
                                            double, uint64_t>(
          input, element_name, header_property.name, callback, context.double_,
          context.reader_);
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

template <std::endian Endianness, size_t Index, typename T,
          typename ReadType = T>
std::expected<void, std::string_view> ReadBinaryPropertyListData(
    std::istream& input, std::string_view element_name,
    std::string_view property_name, PlyReader::Callback callback,
    std::vector<T>& storage, size_t num_to_read, PlyReader* reader) {
  storage.clear();

  for (size_t i = 0; i < num_to_read; i++) {
    auto result =
        ReadBinaryPropertyDataImpl<Endianness, T, ReadType>(input, storage);
    if (!result) {
      return result;
    }
  }

  auto actual_callback = std::get<Index>(callback);
  if (actual_callback == nullptr) {
    return std::expected<void, std::string_view>();
  }

  return (reader->*actual_callback)(element_name, property_name, storage);
}

template <std::endian Endianness>
std::expected<void, std::string_view> ReadBinaryPropertyList(
    std::istream& input, std::string_view element_name,
    const PlyHeader::Property& header_property, PlyReader::Callback callback,
    Context& context) {
  auto num_to_read =
      ReadBinaryListSize<Endianness>(input, *header_property.list_type);
  if (!num_to_read) {
    return std::unexpected(num_to_read.error());
  }

  std::expected<void, std::string_view> result;
  switch (header_property.data_type) {
    case PlyHeader::Property::INT8:
      result = ReadBinaryPropertyListData<Endianness, Property::INT8_LIST>(
          input, element_name, header_property.name, callback, context.int8_,
          *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadBinaryPropertyListData<Endianness, Property::UINT8_LIST>(
          input, element_name, header_property.name, callback, context.uint8_,
          *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::INT16:
      result = ReadBinaryPropertyListData<Endianness, Property::INT16_LIST>(
          input, element_name, header_property.name, callback, context.int16_,
          *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadBinaryPropertyListData<Endianness, Property::UINT16_LIST>(
          input, element_name, header_property.name, callback, context.uint16_,
          *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::INT32:
      result = ReadBinaryPropertyListData<Endianness, Property::INT32_LIST>(
          input, element_name, header_property.name, callback, context.int32_,
          *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadBinaryPropertyListData<Endianness, Property::UINT32_LIST>(
          input, element_name, header_property.name, callback, context.uint32_,
          *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::FLOAT:
      result = ReadBinaryPropertyListData<Endianness, Property::FLOAT_LIST,
                                          float, uint32_t>(
          input, element_name, header_property.name, callback, context.float_,
          *num_to_read, context.reader_);
      break;
    case PlyHeader::Property::DOUBLE:
      result = ReadBinaryPropertyListData<Endianness, Property::DOUBLE_LIST,
                                          double, uint64_t>(
          input, element_name, header_property.name, callback, context.double_,
          *num_to_read, context.reader_);
      break;
  }

  return result;
}

template <std::endian Endianness>
std::expected<void, std::string_view> ReadBinaryData(std::istream& input,
                                                     const PlyHeader& header,
                                                     Context& context) {
  for (size_t e = 0; e < header.elements.size(); e++) {
    for (uint64_t i = 0; i < header.elements[e].num_in_file; i++) {
      for (size_t p = 0; p < header.elements[e].properties.size(); p++) {
        if (header.elements[e].properties[p].list_type) {
          auto error = ReadBinaryPropertyList<Endianness>(
              input, header.elements[e].name, header.elements[e].properties[p],
              std::get<1>(std::get<2>(context.callbacks_[e])[p]), context);
          if (!error) {
            return error;
          }
        } else {
          auto error = ReadBinaryPropertyScalar<Endianness>(
              input, header.elements[e].name, header.elements[e].properties[p],
              std::get<1>(std::get<2>(context.callbacks_[e])[p]), context);
          if (!error) {
            return error;
          }
        }
      }
    }
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ReadNextAsciiToken(std::istream& input,
                                                         std::string& token,
                                                         bool last_line) {
  token.clear();

  char c = 0;
  while (input.get(c)) {
    if (c == ' ') {
      break;
    }

    token.push_back(c);
  }

  if (!c) {
    if (last_line) {
      return std::unexpected(UnexpectedEOF());
    }

    return std::unexpected(
        "The input contained an element with too few tokens");
  }

  if (token.empty()) {
    return std::unexpected("The input contained an empty token");
  }

  return std::expected<void, std::string_view>();
}

template <typename T>
std::expected<void, std::string_view> ReadAsciiPropertyDataImpl(
    std::istream& input, std::string& token, std::vector<T>& output,
    bool last_line) {
  auto success = ReadNextAsciiToken(input, token, last_line);
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
    std::istream& input, std::string& token, bool last_line) {
  auto success = ReadNextAsciiToken(input, token, last_line);
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

template <size_t Index, typename T>
std::expected<void, std::string_view> ReadAsciiPropertyScalarData(
    std::istream& input, std::string_view element_name,
    std::string_view property_name, PlyReader::Callback callback,
    std::string& token, std::vector<T>& storage, PlyReader* reader,
    bool last_line) {
  storage.clear();

  auto result = ReadAsciiPropertyDataImpl(input, token, storage, last_line);
  if (!result) {
    return result;
  }

  auto actual_callback = std::get<Index>(callback);
  if (actual_callback == nullptr) {
    return std::expected<void, std::string_view>();
  }

  return (reader->*actual_callback)(element_name, property_name, storage[0]);
}

std::expected<void, std::string_view> ReadAsciiPropertyScalar(
    std::istream& input, std::string_view element_name,
    const PlyHeader::Property& header_property, PlyReader::Callback callback,
    Context& context, bool last_line) {
  std::expected<void, std::string_view> result;

  switch (header_property.data_type) {
    case PlyHeader::Property::INT8:
      result = ReadAsciiPropertyScalarData<Property::INT8>(
          input, element_name, header_property.name, callback, context.token_,
          context.int8_, context.reader_, last_line);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadAsciiPropertyScalarData<Property::UINT8>(
          input, element_name, header_property.name, callback, context.token_,
          context.uint8_, context.reader_, last_line);
      break;
    case PlyHeader::Property::INT16:
      result = ReadAsciiPropertyScalarData<Property::INT16>(
          input, element_name, header_property.name, callback, context.token_,
          context.int16_, context.reader_, last_line);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadAsciiPropertyScalarData<Property::UINT16>(
          input, element_name, header_property.name, callback, context.token_,
          context.uint16_, context.reader_, last_line);
      break;
    case PlyHeader::Property::INT32:
      result = ReadAsciiPropertyScalarData<Property::INT32>(
          input, element_name, header_property.name, callback, context.token_,
          context.int32_, context.reader_, last_line);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadAsciiPropertyScalarData<Property::UINT32>(
          input, element_name, header_property.name, callback, context.token_,
          context.uint32_, context.reader_, last_line);
      break;
    case PlyHeader::Property::FLOAT:
      result = ReadAsciiPropertyScalarData<Property::FLOAT>(
          input, element_name, header_property.name, callback, context.token_,
          context.float_, context.reader_, last_line);
      break;
    case PlyHeader::Property::DOUBLE:
      result = ReadAsciiPropertyScalarData<Property::DOUBLE>(
          input, element_name, header_property.name, callback, context.token_,
          context.double_, context.reader_, last_line);
      break;
  }

  return result;
}

std::expected<size_t, std::string_view> ReadAsciiListSize(
    std::istream& input, std::string& token, PlyHeader::Property::Type type,
    bool last_line) {
  std::expected<size_t, std::string_view> result;

  switch (type) {
    case PlyHeader::Property::INT8:
      result = ReadAsciiListSizeImpl<int8_t>(input, token, last_line);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadAsciiListSizeImpl<uint8_t>(input, token, last_line);
      break;
    case PlyHeader::Property::INT16:
      result = ReadAsciiListSizeImpl<int16_t>(input, token, last_line);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadAsciiListSizeImpl<uint16_t>(input, token, last_line);
      break;
    case PlyHeader::Property::INT32:
      result = ReadAsciiListSizeImpl<int32_t>(input, token, last_line);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadAsciiListSizeImpl<uint32_t>(input, token, last_line);
      break;
    default:
      assert(false);
  }

  return result;
}

template <size_t Index, typename T>
std::expected<void, std::string_view> ReadAsciiPropertyListData(
    std::istream& input, std::string_view element_name,
    std::string_view property_name, PlyReader::Callback callback,
    std::string& token, std::vector<T>& storage, size_t num_to_read,
    PlyReader* reader, bool last_line) {
  storage.clear();

  for (size_t i = 0; i < num_to_read; i++) {
    auto result =
        ReadAsciiPropertyDataImpl<T>(input, token, storage, last_line);
    if (!result) {
      return result;
    }
  }

  auto actual_callback = std::get<Index>(callback);
  if (actual_callback == nullptr) {
    return std::expected<void, std::string_view>();
  }

  return (reader->*actual_callback)(element_name, property_name, storage);
}

std::expected<void, std::string_view> ReadAsciiPropertyList(
    std::istream& input, std::string_view element_name,
    const PlyHeader::Property& header_property, PlyReader::Callback callback,
    Context& context, bool last_line) {
  auto num_to_read = ReadAsciiListSize(input, context.token_,
                                       *header_property.list_type, last_line);
  if (!num_to_read) {
    return std::unexpected(num_to_read.error());
  }

  std::expected<void, std::string_view> result;
  switch (header_property.data_type) {
    case PlyHeader::Property::INT8:
      result = ReadAsciiPropertyListData<Property::INT8_LIST>(
          input, element_name, header_property.name, callback, context.token_,
          context.int8_, *num_to_read, context.reader_, last_line);
      break;
    case PlyHeader::Property::UINT8:
      result = ReadAsciiPropertyListData<Property::UINT8_LIST>(
          input, element_name, header_property.name, callback, context.token_,
          context.uint8_, *num_to_read, context.reader_, last_line);
      break;
    case PlyHeader::Property::INT16:
      result = ReadAsciiPropertyListData<Property::INT16_LIST>(
          input, element_name, header_property.name, callback, context.token_,
          context.int16_, *num_to_read, context.reader_, last_line);
      break;
    case PlyHeader::Property::UINT16:
      result = ReadAsciiPropertyListData<Property::UINT16_LIST>(
          input, element_name, header_property.name, callback, context.token_,
          context.uint16_, *num_to_read, context.reader_, last_line);
      break;
    case PlyHeader::Property::INT32:
      result = ReadAsciiPropertyListData<Property::INT32_LIST>(
          input, element_name, header_property.name, callback, context.token_,
          context.int32_, *num_to_read, context.reader_, last_line);
      break;
    case PlyHeader::Property::UINT32:
      result = ReadAsciiPropertyListData<Property::UINT32_LIST>(
          input, element_name, header_property.name, callback, context.token_,
          context.uint32_, *num_to_read, context.reader_, last_line);
      break;
    case PlyHeader::Property::FLOAT:
      result = ReadAsciiPropertyListData<Property::FLOAT_LIST>(
          input, element_name, header_property.name, callback, context.token_,
          context.float_, *num_to_read, context.reader_, last_line);
      break;
    case PlyHeader::Property::DOUBLE:
      result = ReadAsciiPropertyListData<Property::DOUBLE_LIST>(
          input, element_name, header_property.name, callback, context.token_,
          context.double_, *num_to_read, context.reader_, last_line);
      break;
  }

  return result;
}

std::expected<void, std::string_view> ReadAsciiData(std::istream& input,
                                                    const PlyHeader& header,
                                                    Context& context) {
  std::stringstream line;

  for (size_t e = 0; e < header.elements.size(); e++) {
    for (uint64_t i = 0; i < header.elements[e].num_in_file; i++) {
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

      for (size_t p = 0; p < header.elements[e].properties.size(); p++) {
        if (header.elements[e].properties[p].list_type) {
          auto error = ReadAsciiPropertyList(
              line, header.elements[e].name, header.elements[e].properties[p],
              std::get<1>(std::get<2>(context.callbacks_[e])[p]), context,
              input.eof());
          if (!error) {
            return error;
          }
        } else {
          auto error = ReadAsciiPropertyScalar(
              line, header.elements[e].name, header.elements[e].properties[p],
              std::get<1>(std::get<2>(context.callbacks_[e])[p]), context,
              input.eof());
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
  }

  return std::expected<void, std::string_view>();
}

std::tuple<std::string_view, PlyReader::Callback> MakeCallback(
    const std::unordered_map<
        std::string_view,
        std::pair<uint64_t,
                  std::unordered_map<std::string_view, Property::Type>>>&
        all_properties,
    const std::unordered_map<
        std::string_view,
        std::unordered_map<std::string_view, PlyReader::Callback>>& callbacks,
    std::string_view element_name, std::string_view property_name) {
  Property::Type type =
      all_properties.at(element_name).second.at(property_name);

  PlyReader::Callback callback;
  switch (type) {
    case Property::INT8:
      callback = PlyReader::Int8PropertyCallback(nullptr);
      break;
    case Property::INT8_LIST:
      callback = PlyReader::Int8PropertyListCallback(nullptr);
      break;
    case Property::UINT8:
      callback = PlyReader::UInt8PropertyCallback(nullptr);
      break;
    case Property::UINT8_LIST:
      callback = PlyReader::UInt8PropertyListCallback(nullptr);
      break;
    case Property::INT16:
      callback = PlyReader::Int16PropertyCallback(nullptr);
      break;
    case Property::INT16_LIST:
      callback = PlyReader::Int16PropertyListCallback(nullptr);
      break;
    case Property::UINT16:
      callback = PlyReader::UInt16PropertyCallback(nullptr);
      break;
    case Property::UINT16_LIST:
      callback = PlyReader::UInt16PropertyListCallback(nullptr);
      break;
    case Property::INT32:
      callback = PlyReader::Int32PropertyCallback(nullptr);
      break;
    case Property::INT32_LIST:
      callback = PlyReader::Int32PropertyListCallback(nullptr);
      break;
    case Property::UINT32:
      callback = PlyReader::UInt32PropertyCallback(nullptr);
      break;
    case Property::UINT32_LIST:
      callback = PlyReader::UInt32PropertyListCallback(nullptr);
      break;
    case Property::FLOAT:
      callback = PlyReader::FloatPropertyCallback(nullptr);
      break;
    case Property::FLOAT_LIST:
      callback = PlyReader::FloatPropertyListCallback(nullptr);
      break;
    case Property::DOUBLE:
      callback = PlyReader::DoublePropertyCallback(nullptr);
      break;
    case Property::DOUBLE_LIST:
      callback = PlyReader::DoublePropertyListCallback(nullptr);
      break;
  }

  auto elements_iter = callbacks.find(element_name);
  if (elements_iter != callbacks.end()) {
    auto property_iter = elements_iter->second.find(property_name);
    if (property_iter != elements_iter->second.end()) {
      if (type == property_iter->second.index()) {
        callback = property_iter->second;
      }
    }
  }

  return std::make_tuple(property_name, callback);
}

}  // namespace

std::expected<void, std::string_view> PlyReader::ReadFrom(std::istream& input) {
  auto header = ReadPlyHeader(input);
  if (!header) {
    return std::unexpected(header.error());
  }

  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<std::string_view, Property::Type>>>
      all_properties;
  for (const auto& element : header->elements) {
    std::unordered_map<std::string_view, Property::Type> properties;
    for (const auto& property : element.properties) {
      if (property.list_type) {
        switch (property.data_type) {
          case PlyHeader::Property::INT8:
            properties[property.name] = Property::INT8_LIST;
            break;
          case PlyHeader::Property::UINT8:
            properties[property.name] = Property::UINT8_LIST;
            break;
          case PlyHeader::Property::INT16:
            properties[property.name] = Property::INT16_LIST;
            break;
          case PlyHeader::Property::UINT16:
            properties[property.name] = Property::UINT16_LIST;
            break;
          case PlyHeader::Property::INT32:
            properties[property.name] = Property::INT32_LIST;
            break;
          case PlyHeader::Property::UINT32:
            properties[property.name] = Property::UINT32_LIST;
            break;
          case PlyHeader::Property::FLOAT:
            properties[property.name] = Property::FLOAT_LIST;
            break;
          case PlyHeader::Property::DOUBLE:
            properties[property.name] = Property::DOUBLE_LIST;
            break;
        }
      } else {
        switch (property.data_type) {
          case PlyHeader::Property::INT8:
            properties[property.name] = Property::INT8;
            break;
          case PlyHeader::Property::UINT8:
            properties[property.name] = Property::UINT8;
            break;
          case PlyHeader::Property::INT16:
            properties[property.name] = Property::INT16;
            break;
          case PlyHeader::Property::UINT16:
            properties[property.name] = Property::UINT16;
            break;
          case PlyHeader::Property::INT32:
            properties[property.name] = Property::INT32;
            break;
          case PlyHeader::Property::UINT32:
            properties[property.name] = Property::UINT32;
            break;
          case PlyHeader::Property::FLOAT:
            properties[property.name] = Property::FLOAT;
            break;
          case PlyHeader::Property::DOUBLE:
            properties[property.name] = Property::DOUBLE;
            break;
        }
      }
    }
    all_properties[element.name] =
        std::make_pair(element.num_in_file, std::move(properties));
  }

  auto started = Start(all_properties, header->comments, header->object_info);
  if (!started) {
    return std::unexpected(started.error());
  }

  Context context = {this};
  for (const auto& element : header->elements) {
    std::vector<std::tuple<std::string_view, PlyReader::Callback>> callbacks;
    for (const auto& property : element.properties) {
      callbacks.push_back(
          MakeCallback(all_properties, *started, element.name, property.name));
    }
    context.callbacks_.emplace_back(element.num_in_file, element.name,
                                    std::move(callbacks));
  }

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

// Static assertions to ensure variants of Callback are properly ordered
static_assert(PlyReader::Callback(PlyReader::Int8PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::INT8));
static_assert(PlyReader::Callback(PlyReader::Int8PropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::INT8_LIST));
static_assert(PlyReader::Callback(PlyReader::UInt8PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::UINT8));
static_assert(PlyReader::Callback(PlyReader::UInt8PropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::UINT8_LIST));
static_assert(PlyReader::Callback(PlyReader::Int16PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::INT16));
static_assert(PlyReader::Callback(PlyReader::Int16PropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::INT16_LIST));
static_assert(PlyReader::Callback(PlyReader::UInt16PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::UINT16));
static_assert(
    PlyReader::Callback(PlyReader::UInt16PropertyListCallback(nullptr))
        .index() == static_cast<size_t>(Property::Type::UINT16_LIST));
static_assert(PlyReader::Callback(PlyReader::Int32PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::INT32));
static_assert(PlyReader::Callback(PlyReader::Int32PropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::INT32_LIST));
static_assert(PlyReader::Callback(PlyReader::UInt32PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::UINT32));
static_assert(
    PlyReader::Callback(PlyReader::UInt32PropertyListCallback(nullptr))
        .index() == static_cast<size_t>(Property::Type::UINT32_LIST));
static_assert(PlyReader::Callback(PlyReader::FloatPropertyCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::FLOAT));
static_assert(PlyReader::Callback(PlyReader::FloatPropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::FLOAT_LIST));
static_assert(PlyReader::Callback(PlyReader::DoublePropertyCallback(nullptr))
                  .index() == static_cast<size_t>(Property::Type::DOUBLE));
static_assert(
    PlyReader::Callback(PlyReader::DoublePropertyListCallback(nullptr))
        .index() == static_cast<size_t>(Property::Type::DOUBLE_LIST));

}  // namespace plyodine