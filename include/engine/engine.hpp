#pragma once

#include <GLFW/glfw3.h>

#include "engine/threading/thread_pool.hpp"

namespace engine {

class Engine {
  public:
    void initialise();

    void update(GLFWwindow *window, float delta_time);

    void render();

  private:
    float _time;

    engine::threading::ThreadPool _thread_pool;
};

} // namespace engine
