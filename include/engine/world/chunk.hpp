#pragma once

#include "engine/camera/camera.hpp"

#include "engine/world/face.hpp"
#include "engine/world/config.hpp"
#include "engine/world/chunk_task.hpp"
#include "engine/world/chunk_state.hpp"

#include "engine/model/mesh.hpp"
#include "engine/model/topology.hpp"

#include "engine/entity/aabb.hpp"

#include "engine/shader/shader.hpp"

namespace engine::world {

class Face;

class World;

class Chunk {
  public:
    int global_x;
    int global_y;
    int global_z;

    int local_x;
    int local_y;
    int local_z;

    engine::world::Chunk *neighbours[27];

    Chunk(int global_x, int global_y, int global_z);

    void generate_terrain(engine::world::World &world);
    bool can_generate_terrain();

    void propagate_sunlight(engine::world::World &world);
    bool can_propagate_sunlight();

    void generate_mesh(engine::camera::Camera &camera);
    bool can_generate_mesh();

    void upload_mesh();
    bool can_upload_mesh();

    void render_opaque(engine::shader::Shader &shader);
    void render_transparent(engine::shader::Shader &shader);

    std::uint16_t &get_block(int x, int y, int z);
    std::uint16_t &get_block(int index);

    std::uint16_t &get_light(int x, int y, int z);
    std::uint16_t &get_light(int index);

    engine::entity::AABB &get_aabb();

    bool is_state_set(const engine::world::ChunkState &state);
    void set_state(const engine::world::ChunkState &state);
    void clear_state(const engine::world::ChunkState &state);

    bool is_task_running(const engine::world::ChunkTask &task);
    bool is_no_task_running();
    void set_running_task(const engine::world::ChunkTask &task);
    void clear_running_task(const engine::world::ChunkTask &task);

    bool is_task_queued(const engine::world::ChunkTask &task);
    bool is_task_queue_empty();
    void clear_queued_task(const engine::world::ChunkTask &task);

    void process_dirty_neighbours_sunlight();

    template <typename... ChunkTasks> void queue_tasks(ChunkTasks... tasks) {
        std::uint8_t mask = 0;

        ((mask |= static_cast<std::uint8_t>(tasks)), ...);

        this->_queued_tasks.fetch_or(mask, std::memory_order_acq_rel);
    }

  private:
    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::TRIANGLE;

    // clang-format off
    static inline constexpr int _BLOCK_OFFSETS[6][4][3][3] = {
        // Top
        {
            {{-1,  1,  0}, { 0,  1,  1}, {-1,  1,  1}},
            {{ 1,  1,  0}, { 0,  1,  1}, { 1,  1,  1}},
            {{ 1,  1,  0}, { 0,  1, -1}, { 1,  1, -1}},
            {{-1,  1,  0}, { 0,  1, -1}, {-1,  1, -1}},
        },

        // Bottom
        {
            {{ 1, -1,  0}, { 0, -1,  1}, { 1, -1,  1}},
            {{-1, -1,  0}, { 0, -1,  1}, {-1, -1,  1}},
            {{-1, -1,  0}, { 0, -1, -1}, {-1, -1, -1}},
            {{ 1, -1,  0}, { 0, -1, -1}, { 1, -1, -1}},
        },

        // Right
        {
            {{ 1,  0,  1}, { 1, -1,  0}, { 1, -1,  1}},
            {{ 1,  0, -1}, { 1, -1,  0}, { 1, -1, -1}},
            {{ 1,  0, -1}, { 1,  1,  0}, { 1,  1, -1}},
            {{ 1,  0,  1}, { 1,  1,  0}, { 1,  1,  1}},
        },

        // Left
        {
            {{-1,  0, -1}, {-1, -1,  0}, {-1, -1, -1}},
            {{-1,  0,  1}, {-1, -1,  0}, {-1, -1,  1}},
            {{-1,  0,  1}, {-1,  1,  0}, {-1,  1,  1}},
            {{-1,  0, -1}, {-1,  1,  0}, {-1,  1, -1}},
        },

        // Front
        {
            {{-1,  0,  1}, { 0, -1,  1}, {-1, -1,  1}},
            {{ 1,  0,  1}, { 0, -1,  1}, { 1, -1,  1}},
            {{ 1,  0,  1}, { 0,  1,  1}, { 1,  1,  1}},
            {{-1,  0,  1}, { 0,  1,  1}, {-1,  1,  1}},
        },

        // Back
        {
            {{ 1,  0, -1}, { 0, -1, -1}, { 1, -1, -1}},
            {{-1,  0, -1}, { 0, -1, -1}, {-1, -1, -1}},
            {{-1,  0, -1}, { 0,  1, -1}, {-1,  1, -1}},
            {{ 1,  0, -1}, { 0,  1, -1}, { 1,  1, -1}},
        },
    };
    // clang-format on

    int _height_map[engine::world::config::CHUNK_SIZE2];

    std::uint16_t _blocks[engine::world::config::CHUNK_SIZE3];
    std::uint16_t _lights[engine::world::config::CHUNK_SIZE3];

    engine::model::Mesh _opaque_mesh;
    engine::model::Mesh _transparent_mesh;

    engine::entity::AABB _aabb;

    std::vector<engine::world::Face> _opaque_faces;
    std::vector<engine::world::Face> _transparent_faces;

    std::atomic<std::uint8_t> _state;
    std::atomic<std::uint8_t> _queued_tasks;
    std::atomic<std::uint8_t> _running_tasks;

    std::atomic<std::uint8_t> _dirty_neighbours_sunlight;

    int get_voxel_id(int x, int y, int z);

    void merge_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int texture_id);

    void merge_XY_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int texture_id);
    void merge_XZ_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int texture_id);
    void merge_YZ_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int texture_id);

    void add_face(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int block_x, int block_y, int block_z, int width, int height, int depth);

    int get_ambient_occlusion(engine::world::BlockType &block_type, int face_type_index, int vertex_index, int x, int y, int z);

    std::uint16_t *get_neighbour_block(int global_x, int global_y, int global_z);

    engine::world::Chunk *get_neighbour_chunk_local(int local_x, int local_y, int local_z);

    std::uint8_t get_neighbour_sunlight(int global_x, int global_y, int global_z);

    void clear_mesh();
};

} // namespace engine::world
