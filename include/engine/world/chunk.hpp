#pragma once

#include <memory>

#include "engine/world/face.hpp"
#include "engine/world/block.hpp"
#include "engine/world/config.hpp"
#include "engine/world/generator.hpp"
#include "engine/world/height_map.hpp"
#include "engine/world/chunk_state.hpp"

#include "engine/model/mesh.hpp"
#include "engine/model/topology.hpp"

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

    void generate(engine::world::Generator &generator, std::unordered_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>> &chunks, engine::world::HeightMap &height_map);

    void generate_mesh();

    void occlude_faces(std::unordered_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>> &chunks);

    void upload_mesh();

    void render(engine::shader::Shader &shader);

    engine::world::Block &get_block(int x, int y, int z);

    engine::world::ChunkState get_state();

    void set_state(const engine::world::ChunkState &state);

  private:
    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::TRIANGLE;

    static inline constexpr std::uint32_t _FULL_MASK = 0xFFFFFFFF;

    int _height_map[engine::world::config::CHUNK_SIZE2];

    engine::world::Block _blocks[engine::world::config::CHUNK_SIZE3];

    engine::model::Mesh _mesh;

    std::vector<engine::world::Face> _faces;

    std::atomic<engine::world::ChunkState> _state;

    int get_block_id(int x, int y, int z);

    /**
     * @brief Culls a face of @p block_a based on the presence and type of an adjacent block (@p block_b).
     *
     * @param block_a: The block whose face may be culled.
     * @param block_b: The adjacent block used to determine whether the face of @block_a should be culled.
     * @param face_type_index The face direction of @p block_a.
     */
    void cull_face_based_on_adjacent_block(engine::world::Block &block_a, engine::world::Block &block_b, int face_type_index);

    void merge_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type);

    void merge_XY_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type);
    void merge_XZ_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type);
    void merge_YZ_faces(engine::world::BlockType &block_type, engine::world::FaceType &face_type);

    void add_face(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int block_x, int block_y, int block_z, int width, int height, int depth);

    void clear_mesh();
};

} // namespace engine::world
