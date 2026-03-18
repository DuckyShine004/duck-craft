#pragma once

#include <cstdint>

namespace engine::world {

enum class BlockFlag : std::uint8_t {
    NONE = 0,
    TRANSPARENT = 1 << 0,
    LIGHT_PASSTHROUGH = 1 << 1,
};

inline constexpr BlockFlag operator|(engine::world::BlockFlag flag_a, engine::world::BlockFlag flag_b) {
    return static_cast<engine::world::BlockFlag>(static_cast<std::uint16_t>(flag_a) | static_cast<std::uint16_t>(flag_b));
}

} // namespace engine::world
