#pragma once

#include <glm/glm.hpp>

namespace engine::model {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    std::uint8_t ambient_occlusion_state;
    std::uint8_t sunlight;

    std::uint16_t texture_id;

    Vertex() : Vertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0U, 0U) {
    }

    Vertex(glm::vec3 &position) : Vertex(position.x, position.y, position.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0U, 0U) {
    }

    Vertex(float x, float y, float z) : Vertex(x, y, z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0U, 0U) {
    }

    Vertex(float x, float y, float z, float u, float v) : Vertex(x, y, z, 0.0f, 0.0f, 0.0f, u, v, 0U, 0U) {
    }

    Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) : Vertex(x, y, z, nx, ny, nz, u, v, 0U, 0U) {
    }

    Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v, std::uint16_t texture_id) : Vertex(x, y, z, nx, ny, nz, u, v, 0U, texture_id) {
    }

    Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v, std::uint8_t ambient_occlusion_state, std::uint16_t texture_id) : position(x, y, z), normal(nx, ny, nz), uv(u, v), ambient_occlusion_state(ambient_occlusion_state), texture_id(texture_id) {
    }
};

} // namespace engine::model
