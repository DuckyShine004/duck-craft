#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>

#include "engine/world/face.hpp"
#include "engine/world/chunk.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::model;

using namespace engine::math::hash::vector;

namespace engine::world {

Face::Face(BlockType &block_type, FaceType &face_type, int x, int y, int z, int width, int height, int depth, int texture_id) : _block_type(block_type), _face_type(face_type), x(x), y(y), z(z), width(width), height(height), depth(depth), texture_id(texture_id) {
}

void Face::set_ambient_occlusion_state(int vertex_index, std::uint8_t ambient_occlusion_mask) {
    this->ambient_occlusion_states[vertex_index] = (ambient_occlusion_mask >> (vertex_index << 1)) & 0b11;
}

void Face::add_to_mesh(Mesh &mesh) {
    switch (this->_face_type) {
        case FaceType::TOP:
            this->add_top(mesh);
            break;
        case FaceType::BOTTOM:
            this->add_bottom(mesh);
            break;
        case FaceType::LEFT:
            this->add_left(mesh);
            break;
        case FaceType::RIGHT:
            this->add_right(mesh);
            break;
        case FaceType::FRONT:
            this->add_front(mesh);
            break;
        case FaceType::BACK:
            this->add_back(mesh);
            break;
    }
}

void Face::add_top(Mesh &mesh) {
    mesh.add_vertex(this->x, this->y + 1.0f, this->z + this->depth, 0.0, 1.0f, 0.0f, 0.0f, 0.0f, this->ambient_occlusion_states[0], this->texture_id);
    mesh.add_vertex(this->x + this->width, this->y + 1.0f, this->z + this->depth, 0.0f, 1.0f, 0.0f, this->width, 0.0f, this->ambient_occlusion_states[1], this->texture_id);
    mesh.add_vertex(this->x + width, this->y + 1.0f, this->z, 0.0f, 1.0f, 0.0f, this->width, this->depth, this->ambient_occlusion_states[2], this->texture_id);
    mesh.add_vertex(this->x, this->y + 1.0f, this->z, 0.0f, 1.0f, 0.0f, 0.0f, this->depth, this->ambient_occlusion_states[3], this->texture_id);
}

void Face::add_left(Mesh &mesh) {
    mesh.add_vertex(this->x, this->y, this->z, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, this->ambient_occlusion_states[0], this->texture_id);
    mesh.add_vertex(this->x, this->y, this->z + this->depth, -1.0f, 0.0f, 0.0f, this->depth, 0.0f, this->ambient_occlusion_states[1], this->texture_id);
    mesh.add_vertex(this->x, this->y + this->height, this->z + this->depth, -1.0f, 0.0f, 0.0f, this->depth, this->height, this->ambient_occlusion_states[2], this->texture_id);
    mesh.add_vertex(this->x, this->y + this->height, this->z, -1.0f, 0.0f, 0.0f, 0.0f, this->height, this->ambient_occlusion_states[3], this->texture_id);
}

void Face::add_right(Mesh &mesh) {
    mesh.add_vertex(this->x + 1.0f, this->y, this->z + this->depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, this->ambient_occlusion_states[0], this->texture_id);
    mesh.add_vertex(this->x + 1.0f, this->y, this->z, 1.0f, 0.0f, 0.0f, this->depth, 0.0f, this->ambient_occlusion_states[1], this->texture_id);
    mesh.add_vertex(this->x + 1.0f, this->y + this->height, this->z, 1.0f, 0.0f, 0.0f, this->depth, this->height, this->ambient_occlusion_states[2], this->texture_id);
    mesh.add_vertex(this->x + 1.0f, this->y + this->height, this->z + this->depth, 1.0f, 0.0f, 0.0f, 0.0f, this->height, this->ambient_occlusion_states[3], this->texture_id);
}

void Face::add_bottom(Mesh &mesh) {
    mesh.add_vertex(this->x + this->width, this->y, this->z + this->depth, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, this->ambient_occlusion_states[0], this->texture_id);
    mesh.add_vertex(this->x, this->y, this->z + this->depth, 0.0f, -1.0f, 0.0f, this->width, 0.0f, this->ambient_occlusion_states[1], this->texture_id);
    mesh.add_vertex(this->x, this->y, this->z, 0.0f, -1.0f, 0.0f, this->width, this->depth, this->ambient_occlusion_states[2], this->texture_id);
    mesh.add_vertex(this->x + this->width, this->y, this->z, 0.0f, -1.0f, 0.0f, 0.0f, this->depth, this->ambient_occlusion_states[3], this->texture_id);
}

void Face::add_front(Mesh &mesh) {
    mesh.add_vertex(this->x, this->y, this->z + 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, this->ambient_occlusion_states[0], this->texture_id);
    mesh.add_vertex(this->x + this->width, this->y, this->z + 1.0f, 0.0f, 0.0f, 1.0f, this->width, 0.0f, this->ambient_occlusion_states[1], this->texture_id);
    mesh.add_vertex(this->x + this->width, this->y + this->height, this->z + 1.0f, 0.0f, 0.0f, 1.0f, this->width, this->height, this->ambient_occlusion_states[2], this->texture_id);
    mesh.add_vertex(this->x, this->y + this->height, this->z + 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, this->height, this->ambient_occlusion_states[3], this->texture_id);
}

void Face::add_back(Mesh &mesh) {
    mesh.add_vertex(this->x + this->width, this->y, this->z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, this->ambient_occlusion_states[0], this->texture_id);
    mesh.add_vertex(this->x, this->y, this->z, 0.0f, 0.0f, -1.0f, this->width, 0.0f, this->ambient_occlusion_states[1], this->texture_id);
    mesh.add_vertex(this->x, this->y + this->height, this->z, 0.0f, 0.0f, -1.0f, this->width, this->height, this->ambient_occlusion_states[2], this->texture_id);
    mesh.add_vertex(this->x + this->width, this->y + this->height, this->z, 0.0f, 0.0f, -1.0f, 0.0f, this->height, this->ambient_occlusion_states[3], this->texture_id);
}

void Face::add_indices(Mesh &mesh, int index_offset) {
    if (this->ambient_occlusion_states[0] + this->ambient_occlusion_states[2] > this->ambient_occlusion_states[1] + this->ambient_occlusion_states[3]) {
        mesh.add_index(0 + index_offset);
        mesh.add_index(1 + index_offset);
        mesh.add_index(3 + index_offset);
        mesh.add_index(1 + index_offset);
        mesh.add_index(2 + index_offset);
        mesh.add_index(3 + index_offset);
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
