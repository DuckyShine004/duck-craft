#pragma once

#include <glm/glm.hpp>

namespace engine::camera::component {

struct View {
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 front;

    View() : right(glm::vec3(1.0f, 0.0f, 0.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)), front(glm::vec3(0.0f, 0.0f, -1.0f)) {
    }
};

} // namespace engine::camera::component
