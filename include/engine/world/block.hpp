#pragma once

#include <cstdint>

#include "engine/world/block_type.hpp"
#include "engine/world/block_metadata.hpp"

namespace engine::world::Block {

/* NOTE:
 * 0-7: Block Type
 * 8: Transparency
 * ...: Extra reserve bits for future
 */

inline constexpr std::uint16_t BLOCK_TYPE_MASK = 0x00FF;

inline constexpr engine::world::BlockType get_type(const std::uint16_t &block) {
    return static_cast<engine::world::BlockType>(block & BLOCK_TYPE_MASK);
}

inline constexpr void set(std::uint16_t &block, const engine::world::BlockType &type) {
    engine::world::BlockMetadata metadata = engine::world::BLOCK_METADATA[static_cast<std::size_t>(type)];

    block = static_cast<std::uint16_t>(type) | (static_cast<std::uint16_t>(metadata.flags) << 8);
}

inline constexpr bool has_flag(const std::uint16_t &block, const engine::world::BlockFlag &flag) {
    return (block & 0xFF00) & (static_cast<std::uint16_t>(flag) << 8);
}

} // namespace engine::world::Block
