#include "engine/world/config.hpp"
#include "engine/world/height_map.hpp"

#include "logger/logger_macros.hpp"

namespace engine::world {

HeightMap::HeightMap() {
}

void HeightMap::generate(Generator &generator, int global_chunk_x, int global_chunk_z) {
    int id = 0;

    for (int dz = 0; dz < config::CHUNK_SIZE; ++dz) {
        for (int dx = 0; dx < config::CHUNK_SIZE; ++dx) {
            int global_x = global_chunk_x + dx;
            int global_z = global_chunk_z + dz;

            int global_y = generator.get_height(global_x, global_z);

            this->_heights[id] = global_y;

            ++id;
        }
    }
}

// TODO: Remember to throw error
int HeightMap::get_height(int x, int z) {
    if (x < 0 || x >= config::CHUNK_SIZE || z < 0 || z >= config::CHUNK_SIZE) {
        return -1;
    }

    int id = x + (z << config::CHUNK_SIZE_BITS);

    return this->_heights[id];
}

} // namespace engine::world
