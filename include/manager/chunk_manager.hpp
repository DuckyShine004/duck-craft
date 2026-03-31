#pragma once

#include "engine/world/world.hpp"

#include "engine/camera/camera.hpp"

#include "engine/threading/thread_pool.hpp"

#include "engine/shader/shader.hpp"

#include "manager/manager.hpp"

namespace manager {

class ChunkManager final : public Manager {
  public:
    static ChunkManager &get_instance();

    void initialise();

    void generate_chunk_at_global_position(const glm::vec3 &position);

    void generate_chunk_at_local_position(int local_x, int local_y, int local_z);

    void load_chunk(engine::camera::Camera *camera);

    void generate_height_map(int local_chunk_x, int local_chunk_z);

    void load_chunks(engine::camera::Camera *camera);

    void process_chunks(engine::camera::Camera *camera);

    void render_water(engine::shader::Shader &shader);
    void render_opaque(engine::shader::Shader &shader);
    void render_transparent(engine::shader::Shader &shader);

    void set_thread_pool(engine::threading::ThreadPool &thread_pool);

  private:
    std::unique_ptr<engine::world::World> _world;

    engine::threading::ThreadPool *_thread_pool;

    std::vector<std::uint32_t> _loaded_chunk_ids;

    ChunkManager();

    ~ChunkManager();
};

}; // namespace manager
