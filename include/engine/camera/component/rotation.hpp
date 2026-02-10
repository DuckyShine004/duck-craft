#pragma once

#include <glm/glm.hpp>

namespace engine::camera::component {

struct Rotation {
    float yaw;
    float pitch;

    glm::vec3 direction;

    Rotation() : yaw(-90.0f), pitch(0.0f), direction(glm::vec3(0.0f)) {
    }
};

} // namespace engine::camera::component
