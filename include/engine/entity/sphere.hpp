/** https://www.songho.ca/opengl/gl_sphere.html */

#pragma once

#include <glm/glm.hpp>

#include "engine/shader/shader.hpp"

#include "engine/model/mesh.hpp"
#include "engine/model/topology.hpp"

namespace engine::entity {

class Sphere {
  public:
    Sphere();

    Sphere(float x, float y, float z);

    void render(engine::shader::Shader &shader);

    glm::vec3 get_position();

    float get_radius();

  private:
    static inline constexpr float _RADIUS = 1.0f;

    static inline constexpr int _STACKS = 50;
    static inline constexpr int _SECTORS = 50;

    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::TRIANGLE;

    glm::vec3 _position;

    engine::model::Mesh _mesh;

    void create();
};

} // namespace engine::entity
