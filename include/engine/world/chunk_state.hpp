#pragma once

namespace engine::world {

enum class ChunkState {
    EMPTY,
    GENERATING_TERRAIN,
    OCCLUDING_FACES,
    GENERATING_MESH,
    UPLOADING_MESH,
    RENDERING,
};

}
