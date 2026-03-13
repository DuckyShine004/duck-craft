#pragma once

#include <cstdint>

#include "engine/world/block_type.hpp"

namespace engine::world::Block {

/* NOTE:
 * 0-7: Block Type
 * 8: Visibility
 * ...: Extra reserve bits for future
 */

inline constexpr std::uint16_t BLOCK_TYPE_MASK = 0x00FF;
inline constexpr std::uint16_t TRANSPARENT_MASK = 0x100;

inline constexpr engine::world::BlockType get_type(const std::uint16_t &block) {
    return static_cast<BlockType>(block & BLOCK_TYPE_MASK);
}

inline constexpr void set_type(std::uint16_t &block, const engine::world::BlockType &type) {
    // Clear the previous block type
    block = (block & ~BLOCK_TYPE_MASK) | static_cast<std::uint8_t>(type);
}

inline constexpr void set_transparent(std::uint16_t &block, bool is_transparent) {
    if (is_transparent) {
        block |= TRANSPARENT_MASK;
    } else {
        block &= ~TRANSPARENT_MASK;
    }
}

inline constexpr bool is_transparent(const std::uint16_t &block) {
    return block & TRANSPARENT_MASK;
}

// class Block {
//   public:
//     Block();
//
//     void set_type(const engine::world::BlockType &type);
//
//     void set_face_state(int face_type_index, bool state);
//     void set_face_state(const engine::world::FaceType &type, bool state);
//
//     engine::world::BlockType &get_type();
//
//     bool is_face_active(const engine::world::FaceType &type);
//
//     void set_sunlight(std::uint8_t sunlight);
//
//     std::uint8_t get_sunlight();
//
//   private:
//     static inline constexpr std::uint8_t _ALL_FACES = (1U << 6) - 1;
//
// engine::world::BlockType _type;
//
//     std::uint8_t _face_mask;
//
//     std::uint8_t _sunlight;
// };

} // namespace engine::world::Block
