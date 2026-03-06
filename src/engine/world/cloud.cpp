#include <glm/ext/matrix_transform.hpp>

#include "engine/world/cloud.hpp"

using namespace engine::shader;

namespace engine::world {

Cloud::Cloud() : _position(0.0f) {
    this->_mesh.add_vertices(this->_VERTICES);

    this->_mesh.add_indices(this->_INDICES);

    this->_mesh.upload();
}

void Cloud::update(glm::vec3 &camera_position) {
    this->_position.x = camera_position.x;
    this->_position.z = camera_position.z;
}

void Cloud::render(Shader &shader) {
    glm::mat4 model(1.0f);

    model = glm::translate(model, glm::vec3(this->_position.x, this->_CLOUD_HEIGHT, this->_position.z));

    shader.set_matrix4fv("u_model", model);

    this->_mesh.render(this->_TOPOLOGY);
}

} // namespace engine::world
