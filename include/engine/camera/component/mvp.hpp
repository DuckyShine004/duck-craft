#pragma once

#include <glm/glm.hpp>

namespace engine::camera::component {

struct MVP {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    MVP() : model(glm::mat4(1.0f)), view(glm::mat4(1.0f)), projection(glm::mat4(1.0f)) {
    }
};

} // namespace engine::camera::component
