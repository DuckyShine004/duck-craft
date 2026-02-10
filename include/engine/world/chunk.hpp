#pragma once

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

    int _height_map[engine::world::config::CHUNK_SIZE2];

    engine::world::Block _blocks[engine::world::config::CHUNK_SIZE3];

    engine::world::Chunk *_neighbours[6];

    engine::model::Mesh _mesh;

    int get_block_id(int x, int y, int z);

    void merge_X_faces(int &index_offset);
    void merge_Y_faces(int &index_offset);
    void merge_Z_faces(int &index_offset);

    void merge_left_faces(int &index_offset, int dx);
    void merge_right_faces(int &index_offset, int dx);

    void merge_top_faces(int &index_offset, int dy);
    void merge_bottom_faces(int &index_offset, int dy);

    void merge_front_faces(int &index_offset, int dz);
    void merge_back_faces(int &index_offset, int dz);
};

} // namespace engine::world
