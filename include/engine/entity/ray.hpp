#pragma once

#include <glm/glm.hpp>

#include "engine/shader/shader.hpp"

#include "engine/model/mesh.hpp"
#include "engine/model/topology.hpp"

namespace engine::entity {

class Ray {
  public:
    Ray();

    Ray(glm::vec3 source, glm::vec3 direction);

    void compute(glm::vec3 &source, glm::vec3 &direction);

    void create();

    void render(engine::shader::Shader &shader);

  private:
    static inline constexpr float _RAY_DISTANCE = 10.0f;

    static inline constexpr GLuint _INDICES[2] = {0, 1};

    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::LINE;

    engine::model::Mesh _mesh;

    glm::vec3 _source;
    glm::vec3 _target;

    glm::vec3 _direction;
};

} // namespace engine::entity
