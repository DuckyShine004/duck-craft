#pragma once

#include <glm/glm.hpp>

namespace engine::model {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    Vertex() : Vertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) {
    }

    Vertex(glm::vec3 &position) : Vertex(position.x, position.y, position.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) {
    }

    Vertex(float x, float y, float z) : Vertex(x, y, z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) {
    }

    Vertex(float x, float y, float z, float u, float v) : Vertex(x, y, z, 0.0f, 0.0f, 0.0f, u, v) {
    }

    Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) : position(x, y, z), normal(nx, ny, nz), uv(u, v) {
    }
};

} // namespace engine::model
