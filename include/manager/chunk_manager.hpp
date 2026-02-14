#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <memory>
#include <unordered_map>

#include <boost/unordered/concurrent_flat_map.hpp>

#include "engine/world/chunk.hpp"
#include "engine/world/generator.hpp"
#include "engine/world/height_map.hpp"

#include "engine/threading/thread_pool.hpp"

#include "engine/math/hash/vector/ivec3.hpp"

#include "engine/shader/shader.hpp"

#include "manager/manager.hpp"

namespace manager {

class ChunkManager final : public Manager {
  public:
    static ChunkManager &get_instance();

    void initialise();

    void generate_chunk(const glm::vec3 &position);

    void generate_height_map(int local_chunk_x, int local_chunk_z);

    void process_chunks();

    void render(engine::shader::Shader &shader);

    void set_thread_pool(engine::threading::ThreadPool &thread_pool);

  private:
    std::shared_ptr<engine::world::Generator> _generator;

    // PERF: Performance of hash function is unknown, but data integrity is gauranteed?
    boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>, engine::math::hash::vector::IVec3Hash, engine::math::hash::vector::IVec3Equal> _chunks;

    // std::unordered_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>> _chunks;

    std::unordered_map<glm::ivec2, std::unique_ptr<engine::world::HeightMap>> _height_maps;

    engine::threading::ThreadPool *_thread_pool;

    ChunkManager();

    ~ChunkManager();
};

}; // namespace manager
