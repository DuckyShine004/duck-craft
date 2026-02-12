#pragma once

#include <GLFW/glfw3.h>

namespace engine {

class Engine {
  public:
    void initialise();

    void update(GLFWwindow *window, float delta_time);

    void render();

  private:
    float _time;
};

} // namespace engine
