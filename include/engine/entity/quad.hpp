#pragma once

#include "engine/model/mesh.hpp"
#include "engine/model/transform.hpp"

#include "engine/shader/shader.hpp"

namespace engine::entity {

class Quad {
  public:
    engine::model::Transform transform;

    Quad();

    Quad(float x, float y, float z);

    Quad(float x, float y, float z, float width, float height);

    Quad(float x, float y, float z, float width, float height, float angle_x, float angle_y, float angle_z);

    engine::model::Mesh &get_mesh();

    void render(shader::Shader &shader);

  private:
    // clang-format off
    static inline constexpr float _VERTICES[4][3] = {
        {-1.0f, -1.0f, 0.0f},
        {1.0f, -1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f}
    };

    static inline constexpr float _NORMALS[4][3] = {
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}
    };

    static inline constexpr float _UVS[4][2] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
    // clang-format on

    static inline constexpr GLuint _INDICES[6] = {0, 1, 2, 2, 3, 0};

    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::TRIANGLE;

    engine::model::Mesh _mesh;

    void create();
};

} // namespace engine::entity
