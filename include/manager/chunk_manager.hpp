#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>

#include "engine/world/chunk.hpp"
#include "engine/world/generator.hpp"
#include "engine/world/height_map.hpp"

#include "engine/shader/shader.hpp"

#include "manager/manager.hpp"

namespace manager {

class ChunkManager final : public Manager {
  public:
    static ChunkManager &get_instance();

    void initialise();

    void generate_chunk(const glm::vec3 &position);

    // void generate_chunk_local(int x, int y, int z);

    void generate_height_map(int local_chunk_x, int local_chunk_z);

    void render(engine::shader::Shader &shader);

    // engine::world::Chunk *get_chunk(glm::vec3 &position);

    engine::world::Chunk &get_chunk_local(int local_chunk_x, int local_chunk_y, int local_chunk_z);

    int get_chunk_local_id(int local_chunk_x, int local_chunk_y, int local_chunk_z);

    engine::world::HeightMap &get_height_map_local(int local_chunk_x, int local_chunk_z);

    int get_height_map_local_id(int local_chunk_x, int local_chunk_z);

  private:
    std::shared_ptr<engine::world::Generator> _generator;

    std::unordered_map<int, std::unique_ptr<engine::world::Chunk>> _chunks;

    std::unordered_map<int, std::unique_ptr<engine::world::HeightMap>> _height_maps;

    ChunkManager();

    ~ChunkManager();
};

}; // namespace manager
