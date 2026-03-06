#pragma once

#include "engine/model/mesh.hpp"

#include "engine/shader/shader.hpp"

namespace engine::world {

class Cloud {
  public:
    Cloud();

    void update(glm::vec3 &camera_position);

    void render(engine::shader::Shader &shader);

  private:
    static inline float _CLOUD_HEIGHT = 128.0f;

    static inline constexpr float _SIZE = 1024.0f;

    // clang-format off
    static inline constexpr float _VERTICES[4][3] = {
        {-_SIZE, 0.0f, -_SIZE},
        { _SIZE, 0.0f, -_SIZE},
        { _SIZE, 0.0f,  _SIZE},
        {-_SIZE, 0.0f,  _SIZE}
    };
    // clang-format on

    static inline constexpr GLuint _INDICES[6] = {0, 1, 2, 0, 2, 3};

    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::TRIANGLE;

    glm::vec3 _position;

    engine::model::Mesh _mesh;
};

} // namespace engine::world
