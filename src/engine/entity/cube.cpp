#include "engine/entity/cube.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

using namespace engine::shader;

using namespace utility;

using namespace common;

namespace engine::entity {

Cube::Cube() : Cube(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f) {
}

Cube::Cube(float x, float y, float z) : Cube(x, y, z, 1.0f, 1.0f, 1.0f) {
}

Cube::Cube(float x, float y, float z, float width, float height, float depth) : _aabb(x, y, z, width, height, depth) {
    this->transform.position = glm::vec3(x, y, z);
    this->transform.scale = glm::vec3(width, height, depth);

    this->set_colour(WHITE_RGB);

    this->create();
}

void Cube::create() {
    this->_mesh.add_vertices(this->_VERTICES, this->_NORMALS, this->_UVS);

    this->_mesh.add_indices(this->_INDICES);
}

void Cube::render(Shader &shader) {
    glm::mat4 model(1.0f);

    model = glm::translate(model, this->transform.position);
    model *= glm::mat4_cast(this->transform.rotation);
    model = glm::scale(model, this->transform.scale);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", this->_colour);

    this->_mesh.render(this->_TOPOLOGY);
}

engine::model::Mesh &Cube::get_mesh() {
    return this->_mesh;
}

engine::entity::AABB &Cube::get_aabb() {
    return this->_aabb;
}

void Cube::set_colour(const float (&colour)[3]) {
    this->_colour = ColourUtility::get_high_precision_RGB(colour);
}

} // namespace engine::entity
