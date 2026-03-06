#pragma once

#include <GLFW/glfw3.h>

#include "engine/world/sky.hpp"
#include "engine/world/cloud.hpp"

#include "engine/threading/thread_pool.hpp"

namespace engine {

class Engine {
  public:
    void initialise();

    void update(GLFWwindow *window, float delta_time);

    void render();

  private:
    float _time;

    std::unique_ptr<engine::world::Sky> _sky;
    std::unique_ptr<engine::world::Cloud> _cloud;

    engine::threading::ThreadPool _thread_pool;
};

} // namespace engine
