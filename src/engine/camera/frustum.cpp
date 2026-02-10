#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>

#include "engine/camera/frustum.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::shader;

using namespace engine::entity;

using namespace utility;

using namespace common;

namespace engine::camera {

Frustum::Frustum() {
    for (std::unique_ptr<Plane> &plane : this->_planes) {
        plane = std::make_unique<Plane>();
    }

    for (std::unique_ptr<Ray> &ray : this->_rays) {
        ray = std::make_unique<Ray>();
    }

    this->_mesh.add_indices(this->_INDICES);
}

void Frustum::update(const glm::vec3 &camera_position, const component::View &view_component, float field_of_view, float aspect_ratio, float near, float far) {
    const glm::vec3 &front = view_component.front;
    const glm::vec3 &up = view_component.up;
    const glm::vec3 &right = view_component.right;

    float angle = glm::radians(field_of_view) / 2.0f;

    this->near_height = 2.0f * glm::tan(angle) * near;
    this->near_width = this->near_height * aspect_ratio;

    float near_height_half = this->near_height / 2.0f;
    float near_width_half = this->near_width / 2.0f;

    this->far_height = 2.0f * glm::tan(angle) * far;
    this->far_width = this->far_height * aspect_ratio;

    float far_height_half = this->far_height / 2.0f;
    float far_width_half = this->far_width / 2.0f;

    glm::vec3 near_centre = camera_position + front * near;

    this->_near_top_left = near_centre + (up * near_height_half) - (right * near_width_half);
    this->_near_top_right = near_centre + (up * near_height_half) + (right * near_width_half);
    this->_near_bottom_left = near_centre - (up * near_height_half) - (right * near_width_half);
    this->_near_bottom_right = near_centre - (up * near_height_half) + (right * near_width_half);

    glm::vec3 far_centre = camera_position + front * far;

    this->_far_top_left = far_centre + (up * far_height_half) - (right * far_width_half);
    this->_far_top_right = far_centre + (up * far_height_half) + (right * far_width_half);
    this->_far_bottom_left = far_centre - (up * far_height_half) - (right * far_width_half);
    this->_far_bottom_right = far_centre - (up * far_height_half) + (right * far_width_half);

    // LOG_DEBUG("Near: {}, {}, {}, {}", glm::to_string(this->_near_bottom_left), glm::to_string(this->_near_bottom_right), glm::to_string(this->_near_top_right),
    // glm::to_string(this->_near_top_left)); LOG_DEBUG("Far: {},{},{},{}", glm::to_string(this->_far_bottom_left), glm::to_string(this->_far_bottom_right), glm::to_string(this->_far_top_right),
    // glm::to_string(this->_far_top_left));

    this->_planes[this->Side::TOP]->compute(this->_near_top_right, this->_near_top_left, this->_far_top_left);
    this->_planes[this->Side::BOTTOM]->compute(this->_near_bottom_left, this->_near_bottom_right, this->_far_bottom_right);
    this->_planes[this->Side::LEFT]->compute(this->_near_top_left, this->_near_bottom_left, this->_far_bottom_left);
    this->_planes[this->Side::RIGHT]->compute(this->_near_bottom_right, this->_near_top_right, this->_far_bottom_right);
    this->_planes[this->Side::NEAR]->compute(this->_near_top_left, this->_near_top_right, this->_near_bottom_right);
    this->_planes[this->Side::FAR]->compute(this->_far_top_right, this->_far_top_left, this->_far_bottom_left);

    glm::vec3 top_centre = (this->_near_top_left + this->_near_top_right + this->_far_top_left + this->_far_top_right) / 4.0f;
    glm::vec3 bottom_centre = (this->_near_bottom_left + this->_near_bottom_right + this->_far_bottom_left + this->_far_bottom_right) / 4.0f;
    glm::vec3 left_centre = (this->_near_top_left + this->_near_bottom_left + this->_far_top_left + this->_far_bottom_left) / 4.0f;
    glm::vec3 right_centre = (this->_near_top_right + this->_near_bottom_right + this->_far_top_right + this->_far_bottom_right) / 4.0f;

    this->_rays[this->Side::TOP]->compute(top_centre, this->_planes[this->Side::TOP]->normal);
    this->_rays[this->Side::BOTTOM]->compute(bottom_centre, this->_planes[this->Side::BOTTOM]->normal);
    this->_rays[this->Side::LEFT]->compute(left_centre, this->_planes[this->Side::LEFT]->normal);
    this->_rays[this->Side::RIGHT]->compute(right_centre, this->_planes[this->Side::RIGHT]->normal);
    this->_rays[this->Side::NEAR]->compute(near_centre, this->_planes[this->Side::NEAR]->normal);
    this->_rays[this->Side::FAR]->compute(far_centre, this->_planes[this->Side::FAR]->normal);

    this->create();
}

void Frustum::create() {
    this->_mesh.clear_vertices();

    this->_mesh.add_vertex(this->_near_bottom_left);
    this->_mesh.add_vertex(this->_near_bottom_right);
    this->_mesh.add_vertex(this->_near_top_right);
    this->_mesh.add_vertex(this->_near_top_left);

    this->_mesh.add_vertex(this->_far_bottom_left);
    this->_mesh.add_vertex(this->_far_bottom_right);
    this->_mesh.add_vertex(this->_far_top_right);
    this->_mesh.add_vertex(this->_far_top_left);

    this->_mesh.upload();

    for (std::unique_ptr<Ray> &ray : this->_rays) {
        ray->create();
    }
}

void Frustum::render(Shader &shader) {
    glm::mat4 model(1.0f);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", ColourUtility::get_high_precision_RGB(BLUE_RGB));

    this->_mesh.render(this->_TOPOLOGY);

    for (std::unique_ptr<Ray> &ray : this->_rays) {
        ray->render(shader);
    }
}

bool Frustum::intersect(glm::vec3 &point) {
    for (std::unique_ptr<Plane> &plane : this->_planes) {
        if (plane->distance(point) < 0.0f) {
            return false;
        }
    }

    return true;
}

bool Frustum::intersect(AABB &aabb) {
    for (std::unique_ptr<Plane> &plane : this->_planes) {
        // Only check for intersection of point
        int out = 0;

        for (int index = 0; index < 8; ++index) {
            if (plane->distance(aabb.get_point(index)) < 0.0f) {
                ++out;
            }
        }

        if (out == 8) {
            return false;
        }
    }

    return true;
}

} // namespace engine::camera
