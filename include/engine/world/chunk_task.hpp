#pragma once

#include <cstdint>

namespace engine::world {

enum class ChunkTask : std::uint8_t {
    TERRAIN_GENERATION = 1 << 0,
    LIGHT_PROPAGATION = 1 << 1,
    MESH_GENERATION = 1 << 2,
    MESH_UPLOAD = 1 << 3,
};

}
