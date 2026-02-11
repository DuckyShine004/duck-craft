#include "engine/world/face.hpp"

using namespace engine::model;

namespace engine::world {

Face::Face(BlockType &block_type, FaceType &face_type, int x, int y, int z, int width, int height, int depth) : block_type(block_type), face_type(face_type), x(x), y(y), z(z), width(width), height(height), depth(depth) {
}

void Face::add_to_mesh(Mesh &mesh) {
    switch (this->face_type) {
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
    mesh.add_vertex(this->x, this->y + 1.0f, this->z + this->depth, 0.0, 1.0f, 0.0f, 0.0f, 0.0f);
    mesh.add_vertex(this->x + this->width, this->y + 1.0f, this->z + this->depth, 0.0f, 1.0f, 0.0f, this->width, 0.0f);
    mesh.add_vertex(this->x + width, this->y + 1.0f, this->z, 0.0f, 1.0f, 0.0f, this->width, this->depth);
    mesh.add_vertex(this->x, this->y + 1.0f, this->z, 0.0f, 1.0f, 0.0f, 0.0f, this->depth);
}

void Face::add_left(engine::model::Mesh &mesh) {
    mesh.add_vertex(this->x, this->y, this->z, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    mesh.add_vertex(this->x, this->y, this->z + this->depth, -1.0f, 0.0f, 0.0f, this->depth, 0.0f);
    mesh.add_vertex(this->x, this->y + this->height, this->z + this->depth, -1.0f, 0.0f, 0.0f, this->depth, this->height);
    mesh.add_vertex(this->x, this->y + this->height, this->z, -1.0f, 0.0f, 0.0f, 0.0f, this->height);
}

void Face::add_right(engine::model::Mesh &mesh) {
    mesh.add_vertex(this->x + 1.0f, this->y, this->z + this->depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    mesh.add_vertex(this->x + 1.0f, this->y, this->z, 1.0f, 0.0f, 0.0f, this->depth, 0.0f);
    mesh.add_vertex(this->x + 1.0f, this->y + this->height, this->z, 1.0f, 0.0f, 0.0f, this->depth, this->height);
    mesh.add_vertex(this->x + 1.0f, this->y + this->height, this->z + this->depth, 1.0f, 0.0f, 0.0f, 0.0f, this->height);
}

void Face::add_bottom(Mesh &mesh) {
    mesh.add_vertex(this->x + this->width, this->y, this->z + this->depth, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
    mesh.add_vertex(this->x, this->y, this->z + this->depth, 0.0f, -1.0f, 0.0f, this->width, 0.0f);
    mesh.add_vertex(this->x, this->y, this->z, 0.0f, -1.0f, 0.0f, this->width, this->depth);
    mesh.add_vertex(this->x + this->width, this->y, this->z, 0.0f, -1.0f, 0.0f, 0.0f, this->depth);
}

void Face::add_front(Mesh &mesh) {
    mesh.add_vertex(this->x, this->y, this->z + 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    mesh.add_vertex(this->x + this->width, this->y, this->z + 1.0f, 0.0f, 0.0f, 1.0f, this->width, 0.0f);
    mesh.add_vertex(this->x + this->width, this->y + this->height, this->z + 1.0f, 0.0f, 0.0f, 1.0f, this->width, this->height);
    mesh.add_vertex(this->x, this->y + this->height, this->z + 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, this->height);
}

void Face::add_back(Mesh &mesh) {
    mesh.add_vertex(this->x + this->width, this->y, this->z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    mesh.add_vertex(this->x, this->y, this->z, 0.0f, 0.0f, -1.0f, this->width, 0.0f);
    mesh.add_vertex(this->x, this->y + this->height, this->z, 0.0f, 0.0f, -1.0f, this->width, this->height);
    mesh.add_vertex(this->x + this->width, this->y + this->height, this->z, 0.0f, 0.0f, -1.0f, 0.0f, this->height);
}

} // namespace engine::world
