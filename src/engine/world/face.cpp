#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>

#include "engine/world/face.hpp"
#include "engine/world/chunk.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::model;

using namespace engine::math::hash::vector;

namespace engine::world {

Face::Face(BlockType &block_type, FaceType &face_type, int x, int y, int z, int width, int height, int depth, int texture_id) : _block_type(block_type), _face_type(face_type), x(x), y(y), z(z), width(width), height(height), depth(depth), texture_id(texture_id) {
    switch (this->_face_type) {
        case FaceType::TOP:
            this->create_top_vertices();
            break;
        case FaceType::BOTTOM:
            this->create_bottom_vertices();
            break;
        case FaceType::LEFT:
            this->create_left_vertices();
            break;
        case FaceType::RIGHT:
            this->create_right_vertices();
            break;
        case FaceType::FRONT:
            this->create_front_vertices();
            break;
        case FaceType::BACK:
            this->create_back_vertices();
            break;
    }
}

void Face::create_top_vertices() {
    this->vertices[0] = Vertex(this->x, this->y + 1.0f, this->z + this->depth, 0.0, 1.0f, 0.0f, 0.0f, 0.0f, this->texture_id);
    this->vertices[1] = Vertex(this->x + this->width, this->y + 1.0f, this->z + this->depth, 0.0f, 1.0f, 0.0f, this->width, 0.0f, this->texture_id);
    this->vertices[2] = Vertex(this->x + width, this->y + 1.0f, this->z, 0.0f, 1.0f, 0.0f, this->width, this->depth, this->texture_id);
    this->vertices[3] = Vertex(this->x, this->y + 1.0f, this->z, 0.0f, 1.0f, 0.0f, 0.0f, this->depth, this->texture_id);
}

void Face::create_bottom_vertices() {
    this->vertices[0] = Vertex(this->x + this->width, this->y, this->z + this->depth, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, this->texture_id);
    this->vertices[1] = Vertex(this->x, this->y, this->z + this->depth, 0.0f, -1.0f, 0.0f, this->width, 0.0f, this->texture_id);
    this->vertices[2] = Vertex(this->x, this->y, this->z, 0.0f, -1.0f, 0.0f, this->width, this->depth, this->texture_id);
    this->vertices[3] = Vertex(this->x + this->width, this->y, this->z, 0.0f, -1.0f, 0.0f, 0.0f, this->depth, this->texture_id);
}

void Face::create_left_vertices() {
    this->vertices[0] = Vertex(this->x, this->y, this->z, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, this->texture_id);
    this->vertices[1] = Vertex(this->x, this->y, this->z + this->depth, -1.0f, 0.0f, 0.0f, this->depth, 0.0f, this->texture_id);
    this->vertices[2] = Vertex(this->x, this->y + this->height, this->z + this->depth, -1.0f, 0.0f, 0.0f, this->depth, this->height, this->texture_id);
    this->vertices[3] = Vertex(this->x, this->y + this->height, this->z, -1.0f, 0.0f, 0.0f, 0.0f, this->height, this->texture_id);
}

void Face::create_right_vertices() {
    this->vertices[0] = Vertex(this->x + 1.0f, this->y, this->z + this->depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, this->texture_id);
    this->vertices[1] = Vertex(this->x + 1.0f, this->y, this->z, 1.0f, 0.0f, 0.0f, this->depth, 0.0f, this->texture_id);
    this->vertices[2] = Vertex(this->x + 1.0f, this->y + this->height, this->z, 1.0f, 0.0f, 0.0f, this->depth, this->height, this->texture_id);
    this->vertices[3] = Vertex(this->x + 1.0f, this->y + this->height, this->z + this->depth, 1.0f, 0.0f, 0.0f, 0.0f, this->height, this->texture_id);
}

void Face::create_front_vertices() {
    this->vertices[0] = Vertex(this->x, this->y, this->z + 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, this->texture_id);
    this->vertices[1] = Vertex(this->x + this->width, this->y, this->z + 1.0f, 0.0f, 0.0f, 1.0f, this->width, 0.0f, this->texture_id);
    this->vertices[2] = Vertex(this->x + this->width, this->y + this->height, this->z + 1.0f, 0.0f, 0.0f, 1.0f, this->width, this->height, this->texture_id);
    this->vertices[3] = Vertex(this->x, this->y + this->height, this->z + 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, this->height, this->texture_id);
}

void Face::create_back_vertices() {
    this->vertices[0] = Vertex(this->x + this->width, this->y, this->z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, this->texture_id);
    this->vertices[1] = Vertex(this->x, this->y, this->z, 0.0f, 0.0f, -1.0f, this->width, 0.0f, this->texture_id);
    this->vertices[2] = Vertex(this->x, this->y + this->height, this->z, 0.0f, 0.0f, -1.0f, this->width, this->height, this->texture_id);
    this->vertices[3] = Vertex(this->x + this->width, this->y + this->height, this->z, 0.0f, 0.0f, -1.0f, 0.0f, this->height, this->texture_id);
}

void Face::set_ambient_occlusion_state(int vertex_index, std::uint8_t ambient_occlusion_mask) {
    this->vertices[vertex_index].ambient_occlusion_state = (ambient_occlusion_mask >> (vertex_index << 1)) & 0b11;
}

void Face::set_sunlight(int vertex_index, std::uint16_t sunlight_mask) {
    this->vertices[vertex_index].sunlight = (sunlight_mask >> (vertex_index << 2)) & 0xF;
}

void Face::add_to_mesh(Mesh &mesh, int index_offset) {
    for (Vertex &vertex : this->vertices) {
        mesh.add_vertex(vertex);
    }

    if (this->vertices[1].ambient_occlusion_state + this->vertices[3].ambient_occlusion_state > this->vertices[0].ambient_occlusion_state + this->vertices[2].ambient_occlusion_state) {
        mesh.add_index(0 + index_offset);
        mesh.add_index(1 + index_offset);
        mesh.add_index(3 + index_offset);
        mesh.add_index(2 + index_offset);
        mesh.add_index(3 + index_offset);
        mesh.add_index(1 + index_offset);
    } else {
        mesh.add_index(0 + index_offset);
        mesh.add_index(1 + index_offset);
        mesh.add_index(2 + index_offset);
        mesh.add_index(2 + index_offset);
        mesh.add_index(3 + index_offset);
        mesh.add_index(0 + index_offset);
    }
}

} // namespace engine::world
