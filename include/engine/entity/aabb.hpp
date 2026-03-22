#pragma once

#include "engine/entity/sphere.hpp"

#include "engine/model/mesh.hpp"
#include "engine/model/topology.hpp"

#include "engine/shader/shader.hpp"

namespace engine::entity {

class AABB {
  public:
    float min_x;
    float min_y;
    float min_z;

    float max_x;
    float max_y;
    float max_z;

    /**
     * @brief Construct AABB, bottom-to-top with anti-clockwise order, using (0,0,0) as origin anchor
     *
     * @param x: x position
     * @param y: y position
     * @param z: z position
     * @param width: width of AABB
     * @param height: height of AABB
     * @param depth: depth of AABB
     */
    AABB(float x, float y, float z, float width, float height, float depth);

    void render(shader::Shader &shader);

    bool collide(engine::entity::Sphere &sphere);

    glm::vec3 &get_point(int index);

  private:
    static inline constexpr engine::model::Topology _TOPOLOGY = engine::model::Topology::LINE;

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

    engine::model::Mesh _mesh;

    glm::vec3 _points[8];
};

} // namespace engine::entity
