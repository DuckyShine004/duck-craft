#pragma once

#include <memory>

#include <boost/unordered/concurrent_flat_map.hpp>

#include "engine/world/face.hpp"
#include "engine/world/block.hpp"
#include "engine/world/config.hpp"
#include "engine/world/generator.hpp"
#include "engine/world/height_map.hpp"
#include "engine/world/chunk_state.hpp"

#include "engine/model/mesh.hpp"
#include "engine/model/topology.hpp"

#include "engine/math/hash/vector/ivec3.hpp"

#include "engine/shader/shader.hpp"

namespace engine::world {

class Chunk {
  public:
    int global_x;
    int global_y;
    int global_z;

    int local_x;
    int local_y;
    int local_z;

    Chunk(int global_x, int global_y, int global_z);

    void generate(engine::world::Generator &generator, engine::world::HeightMap &height_map);

    void generate_mesh();

    void occlude_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>, engine::math::hash::vector::IVec3Hash, engine::math::hash::vector::IVec3Equal> &chunks);

    void occlude_dirty_borders(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>, engine::math::hash::vector::IVec3Hash, engine::math::hash::vector::IVec3Equal> &chunks);

    void upload_mesh();

    void render(engine::shader::Shader &shader);

    engine::world::Block &get_block(int x, int y, int z);

    engine::world::ChunkState get_state();

    void set_state(const engine::world::ChunkState &state);

    std::uint8_t get_dirty_borders_mask_and_reset();

    void set_dirty_border_state(int face_type_index, bool state);

    bool has_dirty_borders();

    bool can_dirty_border_task_run();

    void set_is_dirty_border_task_running(bool is_dirty_border_task_running);

    bool is_terrain_generation_complete();

    void set_is_terrain_generation_complete(bool is_terrain_generation_complete);

  private:
    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::TRIANGLE;

    static inline constexpr std::uint32_t _FULL_MASK = 0xFFFFFFFF;

    int _height_map[engine::world::config::CHUNK_SIZE2];

    engine::world::Block _blocks[engine::world::config::CHUNK_SIZE3];

    engine::model::Mesh _mesh;

    std::vector<engine::world::Face> _faces;

    std::atomic<engine::world::ChunkState> _state;

    std::atomic<bool> _is_dirty_border_task_running;

    std::atomic<bool> _is_terrain_generation_complete;

    std::atomic<std::uint8_t> _dirty_borders_mask;

    int get_block_id(int x, int y, int z);

    /**
     * @brief Culls a face of @p block based on the presence and type of an adjacent block (@p adjacent_block).
     *
     * @param block: The block whose face may be culled.
     * @param adjacent_block: The adjacent block used to determine whether the face of @block should be culled.
     * @param face_type_index The face direction of @p block.
     */
    void cull_face_based_on_adjacent_block(engine::world::Block &block, engine::world::Block &adjacent_block, int face_type_index);

    void merge_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int texture_id);

    void merge_XY_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int texture_id);
    void merge_XZ_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int texture_id);
    void merge_YZ_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int texture_id);

    void add_face(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int block_x, int block_y, int block_z, int width, int height, int depth);

    void occlude_border_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>, engine::math::hash::vector::IVec3Hash, engine::math::hash::vector::IVec3Equal> &chunks);

    void occlude_XY_faces(engine::world::Chunk &adjacent_chunk, const engine::world::FaceType &face_type);
    void occlude_XZ_faces(engine::world::Chunk &adjacent_chunk, const engine::world::FaceType &face_type);
    void occlude_YZ_faces(engine::world::Chunk &adjacent_chunk, const engine::world::FaceType &face_type);

    void clear_mesh();
};

} // namespace engine::world
