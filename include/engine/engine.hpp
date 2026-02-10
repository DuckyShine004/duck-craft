#pragma once

#include "engine/camera/camera.hpp"

#include "engine/entity/quad.hpp"
#include "engine/entity/cube.hpp"
#include "engine/entity/sphere.hpp"

namespace engine {

class Engine {
  public:
    void initialise();

    void update(GLFWwindow *window, float delta_time);

    void render();

  private:
    std::vector<engine::entity::Quad> _quads;
    std::vector<engine::entity::Cube> _cubes;
    std::vector<engine::entity::Sphere> _spheres;

    float _time;
};

} // namespace engine
