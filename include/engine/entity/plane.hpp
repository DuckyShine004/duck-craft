#pragma once

#include <glm/glm.hpp>

namespace engine::entity {

class Plane {
  public:
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;

    glm::vec3 normal;

    Plane();

    Plane(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3);

    float distance(glm::vec3 &point);

    void compute(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3);

  private:
    float _d;
};

} // namespace engine::entity
