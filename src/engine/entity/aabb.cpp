#include "engine/entity/aabb.hpp"

#include "common/constant.hpp"

using namespace engine::shader;

using namespace common;

namespace engine::entity {

AABB::AABB(float x, float y, float z, float width, float height, float depth) {
    this->min_x = x;
    this->min_y = y;
    this->min_z = z;

    this->max_x = x + width;
    this->max_y = y + height;
    this->max_z = z + depth;

    /* Bottom */
    glm::vec3 b1(this->min_x, this->min_y, this->min_z);
    glm::vec3 b2(this->min_x, this->min_y, this->max_z);
    glm::vec3 b3(this->max_x, this->min_y, this->max_z);
    glm::vec3 b4(this->max_x, this->min_y, this->min_z);

    /* Top */
    glm::vec3 t1(this->min_x, this->max_y, this->min_z);
    glm::vec3 t2(this->min_x, this->max_y, this->max_z);
    glm::vec3 t3(this->max_x, this->max_y, this->max_z);
    glm::vec3 t4(this->max_x, this->max_y, this->min_z);

    this->_points[0] = b1;
    this->_points[1] = b2;
    this->_points[2] = b3;
    this->_points[3] = b4;

    this->_points[4] = t1;
    this->_points[5] = t2;
    this->_points[6] = t3;
    this->_points[7] = t4;

    this->_mesh.add_vertex(b1);
    this->_mesh.add_vertex(b2);
    this->_mesh.add_vertex(b3);
    this->_mesh.add_vertex(b4);

    this->_mesh.add_vertex(t1);
    this->_mesh.add_vertex(t2);
    this->_mesh.add_vertex(t3);
    this->_mesh.add_vertex(t4);

    this->_mesh.add_indices(this->_INDICES);

    this->_mesh.upload();
}

void AABB::render(Shader &shader) {
    glm::mat4 model(1.0f);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", WHITE_RGB);

    this->_mesh.render(this->_TOPOLOGY);
}

bool AABB::collide(Sphere &sphere) {
    glm::vec3 position = sphere.get_position();

    float radius = sphere.get_radius();

    float x = glm::clamp(position.x, this->min_x, this->max_x);
    float y = glm::clamp(position.y, this->min_y, this->max_y);
    float z = glm::clamp(position.z, this->min_z, this->max_z);

    glm::vec3 delta = position - glm::vec3(x, y, z);

    return glm::dot(delta, delta) <= radius * radius;
}

glm::vec3 &AABB::get_point(int index) {
    return this->_points[index];
}

} // namespace engine::entity
