#pragma once

#include "engine/world/config.hpp"
#include "engine/world/generator.hpp"

namespace engine::world {

class HeightMap {
  public:
    HeightMap();

    void generate(engine::world::Generator &generator, int global_chunk_x, int global_chunk_z);

    int get_height(int x, int z);

  private:
    int _heights[engine::world::config::CHUNK_SIZE2];
};

} // namespace engine::world
