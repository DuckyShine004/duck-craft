#pragma once

#include "engine/world/face.hpp"
#include "engine/world/block.hpp"
#include "engine/world/config.hpp"
#include "engine/world/generator.hpp"

#include "engine/model/mesh.hpp"
#include "engine/model/topology.hpp"

#include "engine/shader/shader.hpp"

namespace engine::world {

class HeightMap;

// NOTE: Assume global offset
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

    void cull_block_faces();

    void render(engine::shader::Shader &shader);

  private:
    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::TRIANGLE;

    static inline constexpr std::uint32_t _FULL_MASK = 0xFFFFFFFF;

    int _height_map[engine::world::config::CHUNK_SIZE2];

    engine::world::Block _blocks[engine::world::config::CHUNK_SIZE3];

    engine::world::Chunk *_neighbours[6];

    engine::model::Mesh _mesh;

    std::vector<engine::world::Face> _faces;

    int get_block_id(int x, int y, int z);

    void merge_faces(BlockType &block_type, FaceType &face_type);

    void merge_XY_faces(BlockType &block_type, FaceType &face_type);
    void merge_XZ_faces(BlockType &block_type, FaceType &face_type);
    void merge_YZ_faces(BlockType &block_type, FaceType &face_type);

    void add_face(BlockType &block_type, FaceType &face_type, int block_x, int block_y, int block_z, int width, int height, int depth);

    void clear_mesh();
};

} // namespace engine::world
