#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>

#include "engine/world/tree.hpp"
#include "engine/world/chunk.hpp"
#include "engine/world/generator.hpp"
#include "engine/world/height_map.hpp"

namespace engine::world {

class World {
  public:
    std::shared_ptr<engine::world::Generator> generator;

    std::unordered_map<glm::ivec2, std::unique_ptr<engine::world::HeightMap>> height_maps;

    std::unordered_map<glm::ivec3, std::uint32_t> chunk_ids;

    std::vector<std::unique_ptr<engine::world::Chunk>> chunks;

    World();

    engine::world::HeightMap *try_emplace_height_map(int chunk_local_x, int chunk_local_z, int chunk_global_x, int chunk_global_z);

    std::vector<Tree> get_trees(int chunk_local_x, int chunk_local_z);

    std::uint32_t try_emplace_chunk_id(int chunk_local_x, int chunk_local_y, int chunk_local_z, int chunk_global_x, int chunk_global_y, int chunk_global_z);

    engine::world::HeightMap *find_height_map(int chunk_local_x, int chunk_local_z);

    engine::world::Chunk *find_chunk(int chunk_local_x, int chunk_local_y, int chunk_local_z);

    std::uint16_t *find_global_block(int global_x, int global_y, int global_z);

    void set_chunk_neighbours(int chunk_local_x, int chunk_local_y, int chunk_local_z);

  private:
    std::uint32_t _global_chunk_id;
};

} // namespace engine::world
