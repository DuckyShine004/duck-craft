#pragma once

#include <cstdint>

#include "engine/world/face_type.hpp"
#include "engine/world/block_type.hpp"

namespace engine::world {

class Block {
  public:
    Block();

    void set_type(const engine::world::BlockType &type);

    void set_face_state(const engine::world::FaceType &type, bool state);

    engine::world::BlockType &get_type();

    bool is_face_active(const engine::world::FaceType &type);

  private:
    static inline constexpr std::uint8_t _ALL_FACES = (1U << 6) - 1;

    engine::world::BlockType _type;

    std::uint8_t _face_mask;
};

} // namespace engine::world
