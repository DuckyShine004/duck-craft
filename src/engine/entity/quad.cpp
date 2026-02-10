#include <glm/gtc/quaternion.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "engine/entity/quad.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

using namespace engine::shader;

using namespace engine::model;

using namespace utility;

using namespace common;

namespace engine::entity {

Quad::Quad() : Quad(0.0f, 0.0f, 0.0f, 1.0f, 1.0f) {
}

Quad::Quad(float x, float y, float z) : Quad(x, y, z, 1.0f, 1.0f) {
}

Quad::Quad(float x, float y, float z, float width, float height) : Quad(x, y, z, width, height, 0.0f, 0.0f, 0.0f) {
}

Quad::Quad(float x, float y, float z, float width, float height, float angle_x, float angle_y, float angle_z) {
    this->transform.position = glm::vec3(x, y, z);
    this->transform.scale = glm::vec3(width, height, 1.0f);
    this->transform.set_rotation_euler_xyz(angle_x, angle_y, angle_z);

    this->create();
}

Mesh &Quad::get_mesh() {
    return this->_mesh;
}

void Quad::create() {
    this->_mesh.add_vertices(this->_VERTICES, this->_NORMALS, this->_UVS);

    this->_mesh.add_indices(this->_INDICES);
}

void Quad::render(Shader &shader) {
    glm::mat4 model(1.0f);

    model = glm::translate(model, this->transform.position);
    model *= glm::mat4_cast(this->transform.rotation);
    model = glm::scale(model, this->transform.scale);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", ColourUtility::get_high_precision_RGB(GREY_RGB));

    this->_mesh.render(this->_TOPOLOGY);
}

} // namespace engine::entity
