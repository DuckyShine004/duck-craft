#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

#include "engine/entity/sphere.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

using namespace engine::shader;

using namespace engine::model;

using namespace utility;

using namespace common;

namespace engine::entity {

Sphere::Sphere() : Sphere(0.0f, 0.0f, 0.0f) {
}

Sphere::Sphere(float x, float y, float z) : _position(x, y, z) {
    this->create();
}

void Sphere::create() {
    float sector_step = glm::two_pi<float>() / this->_SECTORS;
    float stack_step = glm::pi<float>() / this->_STACKS;

    float length_inverse = 1.0f / this->_RADIUS;

    for (int stack_index = 0; stack_index <= this->_STACKS; ++stack_index) {
        float stack_angle = glm::half_pi<float>() - stack_index * stack_step;

        float xy = this->_RADIUS * cosf(stack_angle);
        float z = this->_RADIUS * sinf(stack_angle);

        for (int sector_index = 0; sector_index <= this->_SECTORS; ++sector_index) {
            float sector_angle = sector_index * sector_step;

            float x = xy * cosf(sector_angle);
            float y = xy * sinf(sector_angle);

            float nx = x * length_inverse;
            float ny = y * length_inverse;
            float nz = z * length_inverse;

            float u = (float)sector_index / this->_SECTORS;
            float v = (float)stack_index / this->_STACKS;

            this->_mesh.add_vertex(x, y, z, nx, ny, nz, u, v);
        }
    }

    for (int stack_index = 0; stack_index < this->_STACKS; ++stack_index) {
        int k1 = stack_index * (this->_SECTORS + 1);
        int k2 = k1 + this->_SECTORS + 1;

        for (int sector_index = 0; sector_index < this->_SECTORS; ++sector_index, ++k1, ++k2) {
            if (stack_index != 0) {
                this->_mesh.add_index(k1);
                this->_mesh.add_index(k2);
                this->_mesh.add_index(k1 + 1);
            }

            if (stack_index != this->_STACKS - 1) {
                this->_mesh.add_index(k1 + 1);
                this->_mesh.add_index(k2);
                this->_mesh.add_index(k2 + 1);
            }
        }
    }

    this->_mesh.upload();
}

/**
 * @brief Renders the sphere by setting up uniforms. Also NO NEED to apply scaling since we
 * it's scaled properly at mesh creation.
 *
 * @param shader
 */
void Sphere::render(Shader &shader) {
    glm::mat4 model(1.0f);

    model = glm::translate(model, this->_position);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", ColourUtility::get_high_precision_RGB(GREY_RGB));

    this->_mesh.render(this->_TOPOLOGY);
}

float Sphere::get_radius() {
    return this->_RADIUS;
}

glm::vec3 Sphere::get_position() {
    return this->_position;
}

} // namespace engine::entity
