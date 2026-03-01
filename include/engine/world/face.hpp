#pragma once

#include <boost/unordered/concurrent_flat_map.hpp>

#include "engine/world/face_type.hpp"
#include "engine/world/block_type.hpp"

#include "engine/model/mesh.hpp"

namespace engine::world {

class Chunk;

class Face {
  public:
    // clang-format off
    static inline constexpr int I_NORMALS[6][3] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 0, 0},
        {-1, 0, 0},
        {0, 0, 1},
        {0, 0, -1},
    };
    //clang-format on
    
    int x;
    int y;
    int z;

    int width;
    int height;
    int depth;

    int texture_id;

    engine::model::Vertex vertices[4];

    Face(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int x, int y, int z, int width, int height, int depth, int texture_id);

    void set_ambient_occlusion_state(int vertex_index, std::uint8_t ambient_occlusion_mask);

    void set_sunlight(int vertex_index, std::uint16_t sunlight_mask);

    void add_to_mesh(engine::model::Mesh &mesh, int index_offset);

  private:
    engine::world::BlockType _block_type;

    engine::world::FaceType _face_type;

    void create_top_vertices();
    void create_bottom_vertices();
    void create_left_vertices();
    void create_right_vertices();
    void create_front_vertices();
    void create_back_vertices();
};

} // namespace engine::world
