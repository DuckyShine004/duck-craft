#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine::model {

struct Transform {
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;

    Transform() : Transform(glm::vec3(0.0f), glm::vec3(1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {
    }

    Transform(float x, float y, float z) : Transform(glm::vec3(x, y, z)) {
    }

    Transform(glm::vec3 position) : Transform(position, glm::vec3(1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {
    }

    Transform(glm::vec3 position, glm::vec3 scale) : Transform(position, scale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {
    }

    Transform(glm::vec3 position, glm::vec3 scale, glm::quat rotation) : position(position), scale(scale), rotation(rotation) {
    }

    void set_rotation_euler_x(float angle) {
        // rotation.xglm::radians(angle);
    }

    void set_rotation_euler_y(float angle) {
    }

    void set_rotation_euler_z(float angle) {
    }

    void set_rotation_euler_xyz(float angle_x, float angle_y, float angle_z) {
        glm::quat quaternion_x = glm::angleAxis(glm::radians(angle_x), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat quaternion_y = glm::angleAxis(glm::radians(angle_y), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat quaternion_z = glm::angleAxis(glm::radians(angle_z), glm::vec3(0.0f, 0.0f, 1.0f));

        this->rotation = quaternion_z * quaternion_y * quaternion_x;
    }
};

} // namespace engine::model
