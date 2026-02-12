#pragma once

#include "engine/world/face_type.hpp"
#include "engine/world/block_type.hpp"

#include "engine/model/mesh.hpp"

namespace engine::world {

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
    
    static inline constexpr int NUMBER_OF_FACES = 6;

    int x;
    int y;
    int z;

    int width;
    int height;
    int depth;

    Face(engine::world::BlockType &block_type, engine::world::FaceType &face_type, int x, int y, int z, int width, int height, int depth);

    void add_to_mesh(engine::model::Mesh &mesh);

  private:
    engine::world::BlockType block_type;

    engine::world::FaceType face_type;

    void add_top(engine::model::Mesh &mesh);
    void add_bottom(engine::model::Mesh &mesh);
    void add_left(engine::model::Mesh &mesh);
    void add_right(engine::model::Mesh &mesh);
    void add_front(engine::model::Mesh &mesh);
    void add_back(engine::model::Mesh &mesh);
};

} // namespace engine::world
