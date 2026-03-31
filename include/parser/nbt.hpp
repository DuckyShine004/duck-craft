#pragma once

#include <vector>
#include <cstdint>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

namespace parser {

class NBTParser {
  public:
    static inline void parse(nlohmann::json &json, const std::vector<std::uint8_t> &data) {
        std::size_t position = 0;

        std::uint8_t tag_id = read_u8(data, position);

        std::uint16_t tag_name_length = read_u16(data, position);

        std::string tag_name = read_string(data, position, tag_name_length);

        json[tag_name] = parse_payload(data, position, tag_id);
    }

  private:
    static inline constexpr std::uint8_t read_u8(const std::vector<std::uint8_t> &data, std::size_t &position) {
        std::uint8_t result = data[position];

        ++position;

        return result;
    }

    static inline constexpr std::uint16_t read_u16(const std::vector<std::uint8_t> &data, std::size_t &position) {
        std::uint16_t result = 0U;

        result |= std::uint16_t(data[position] << 8);
        result |= std::uint16_t(data[position + 1]);

        position += 2;

        return result;
    }

    static inline constexpr std::uint32_t read_u32(const std::vector<std::uint8_t> &data, std::size_t &position) {
        std::uint32_t result = 0U;

        result |= std::uint32_t(data[position] << 24);
        result |= std::uint32_t(data[position + 1] << 16);
        result |= std::uint32_t(data[position + 2] << 8);
        result |= std::uint32_t(data[position + 3]);

        position += 4;

        return result;
    }

    static inline constexpr std::uint64_t read_u64(const std::vector<std::uint8_t> &data, std::size_t &position) {
        std::uint64_t result = 0U;

        result |= std::uint64_t(data[position]) << 56;
        result |= std::uint64_t(data[position + 1]) << 48;
        result |= std::uint64_t(data[position + 2]) << 40;
        result |= std::uint64_t(data[position + 3]) << 32;
        result |= std::uint64_t(data[position + 4]) << 24;
        result |= std::uint64_t(data[position + 5]) << 16;
        result |= std::uint64_t(data[position + 6]) << 8;
        result |= std::uint64_t(data[position + 7]);

        position += 8;

        return result;
    }

    static inline constexpr std::int8_t read_i8(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return static_cast<std::int8_t>(parser::NBTParser::read_u8(data, position));
    }

    static inline constexpr std::int16_t read_i16(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return static_cast<std::int16_t>(parser::NBTParser::read_u16(data, position));
    }

    static inline constexpr std::int32_t read_i32(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return static_cast<std::int32_t>(parser::NBTParser::read_u32(data, position));
    }

    static inline constexpr std::int64_t read_i64(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return static_cast<std::int64_t>(parser::NBTParser::read_u64(data, position));
    }

    static inline constexpr float read_f32(const std::vector<std::uint8_t> &data, std::size_t &position) {
        const std::uint32_t bits = parser::NBTParser::read_u32(data, position);

        return std::bit_cast<float>(bits);
    }

    static inline constexpr double read_f64(const std::vector<std::uint8_t> &data, std::size_t &position) {
        const std::uint64_t bits = parser::NBTParser::read_u64(data, position);

        return std::bit_cast<double>(bits);
    }

    static inline constexpr std::string read_string(const std::vector<std::uint8_t> &data, std::size_t &position, const std::size_t &length) {
        std::string result;

        result.reserve(length);

        for (std::size_t i = 0; i < length; ++i) {
            result.push_back(static_cast<char>(data[position + i]));
        }

        position += length;

        return result;
    }

    static inline nlohmann::json parse_byte_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return parser::NBTParser::read_i8(data, position);
    }

    static inline nlohmann::json parse_short_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return parser::NBTParser::read_i16(data, position);
    }

    static inline nlohmann::json parse_int_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return parser::NBTParser::read_i32(data, position);
    }

    static inline nlohmann::json parse_long_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return parser::NBTParser::read_i64(data, position);
    }

    static inline nlohmann::json parse_float_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return parser::NBTParser::read_f32(data, position);
    }

    static inline nlohmann::json parse_double_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        return parser::NBTParser::read_f64(data, position);
    }

    static inline nlohmann::json parse_byte_array_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        std::int32_t size = parser::NBTParser::read_i32(data, position);

        if (size < 0) {
            throw std::runtime_error("Size of byte array is negative");
        }

        nlohmann::json array = nlohmann::json::array();

        for (std::size_t i = 0; i < size; ++i) {
            array.push_back(parser::NBTParser::read_u8(data, position));
        }

        return array;
    }

    static inline nlohmann::json parse_string_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        std::uint16_t length = parser::NBTParser::read_u16(data, position);

        return parser::NBTParser::read_string(data, position, length);
    }

    static inline nlohmann::json parse_list_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        std::uint8_t type = parser::NBTParser::read_u8(data, position);

        std::int32_t size = parser::NBTParser::read_i32(data, position);

        if (size < 0) {
            throw std::runtime_error("Size of list is negative");
        }

        nlohmann::json array = nlohmann::json::array();

        for (std::size_t i = 0; i < size; ++i) {
            array.push_back(parser::NBTParser::parse_payload(data, position, type));
        }

        return array;
    }

    static inline nlohmann::json parse_compound_tag(const std::vector<std::uint8_t> &data, std::size_t &position) {
        nlohmann::json object = nlohmann::json::object();

        while (true) {
            std::uint8_t child_tag_id = parser::NBTParser::read_u8(data, position);

            if (child_tag_id == 0) {
                break;
            }

            std::uint16_t child_tag_name_length = parser::NBTParser::read_u16(data, position);

            std::string child_tag_name = parser::NBTParser::read_string(data, position, child_tag_name_length);

            object[child_tag_name] = parser::NBTParser::parse_payload(data, position, child_tag_id);
        }

        return object;
    }

    static inline nlohmann::json parse_payload(const std::vector<std::uint8_t> &data, std::size_t &position, const std::size_t &tag_id) {
        switch (tag_id) {
            case 1:
                {
                    return parser::NBTParser::parse_byte_tag(data, position);
                }
            case 2:
                {
                    return parser::NBTParser::parse_short_tag(data, position);
                }
            case 3:
                {
                    return parser::NBTParser::parse_int_tag(data, position);
                }
            case 4:
                {
                    return parser::NBTParser::parse_long_tag(data, position);
                }
            case 5:
                {
                    return parser::NBTParser::parse_float_tag(data, position);
                }
            case 6:
                {
                    return parser::NBTParser::parse_double_tag(data, position);
                }
            case 7:
                {
                    return parser::NBTParser::parse_byte_array_tag(data, position);
                }
            case 8:
                {
                    return parser::NBTParser::parse_string_tag(data, position);
                }
            case 9:
                {
                    return parser::NBTParser::parse_list_tag(data, position);
                }
            case 10:
                {
                    return parser::NBTParser::parse_compound_tag(data, position);
                }
            default:
                throw std::runtime_error(fmt::format("Tag ID: {} not found", tag_id));
        }
    }
};

} // namespace parser
