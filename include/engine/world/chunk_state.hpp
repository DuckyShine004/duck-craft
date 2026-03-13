#pragma once

#include <cstdint>

namespace engine::world {

enum class ChunkState : std::uint8_t {
    TERRAIN_GENERATED = 1 << 0,
    RENDERING = 1 << 1,
};

}
