#pragma once

#include <memory>

#include "engine/model/mesh.hpp"
#include "engine/model/topology.hpp"

#include "engine/entity/ray.hpp"
#include "engine/entity/aabb.hpp"
#include "engine/entity/plane.hpp"

#include "engine/camera/component/view.hpp"

#include "engine/shader/shader.hpp"

namespace engine::camera {

/* PERF: in terms of rendering, could compute initial points and apply transforms
 * instead of rebuilding mesh each time there's an update */
class Frustum {
  public:
    Frustum();

    void update(const glm::vec3 &camera_position, const engine::camera::component::View &view_component, float field_of_view, float aspect_ratio, float near, float far);

    void render(shader::Shader &shader);

    bool intersect(glm::vec3 &point);
    bool intersect(engine::entity::AABB &aabb);

  private:
    // clang-format off
    static inline constexpr GLuint _INDICES[24] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };
    // clang-format on

    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::LINE;

    // clang-format off
    enum Side {
        TOP = 0,
        BOTTOM,
        LEFT,
        RIGHT,
        NEAR,
        FAR,
    };
    // clang-format on

    engine::model::Mesh _mesh;

    glm::vec3 _near_top_left;
    glm::vec3 _near_top_right;
    glm::vec3 _near_bottom_left;
    glm::vec3 _near_bottom_right;

    glm::vec3 _far_top_left;
    glm::vec3 _far_top_right;
    glm::vec3 _far_bottom_left;
    glm::vec3 _far_bottom_right;

    float near_height;
    float near_width;

    float far_height;
    float far_width;

    std::unique_ptr<engine::entity::Plane> _planes[6];

    std::unique_ptr<engine::entity::Ray> _rays[6];

    void create();
};

} // namespace engine::camera
