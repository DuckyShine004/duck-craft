#include "engine/entity/plane.hpp"

namespace engine::entity {

Plane::Plane() = default;

Plane::Plane(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3) {
    this->compute(p1, p2, p3);
}

float Plane::distance(glm::vec3 &point) {
    return glm::dot(this->normal, point) + this->_d;
}

void Plane::compute(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3) {
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;

    glm::vec3 v = p2 - p1;
    glm::vec3 w = p3 - p1;

    this->normal = glm::normalize(glm::cross(v, w));

    this->_d = -glm::dot(this->normal, p1);
}

} // namespace engine::entity
