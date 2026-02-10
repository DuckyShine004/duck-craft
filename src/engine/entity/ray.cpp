#include "engine/entity/ray.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

using namespace engine::shader;

using namespace utility;

using namespace common;

namespace engine::entity {

Ray::Ray() : Ray(glm::vec3(0.0f), glm::vec3(0.0f)) {
}

Ray::Ray(glm::vec3 source, glm::vec3 direction) {
    this->compute(source, direction);

    this->_mesh.add_indices(this->_INDICES);
}

void Ray::compute(glm::vec3 &source, glm::vec3 &direction) {
    this->_source = source;
    this->_direction = direction;

    this->_target = source + direction * this->_RAY_DISTANCE;
}

void Ray::create() {
    this->_mesh.clear_vertices();

    this->_mesh.add_vertex(this->_source);
    this->_mesh.add_vertex(this->_target);

    this->_mesh.upload();
}

void Ray::render(Shader &shader) {
    glm::mat4 model(1.0f);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", ColourUtility::get_high_precision_RGB(BLUE_RGB));

    this->_mesh.render(this->_TOPOLOGY);
}

} // namespace engine::entity
