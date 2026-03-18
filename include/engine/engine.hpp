#pragma once

#include <GLFW/glfw3.h>

#include "engine/world/sky.hpp"
#include "engine/world/cloud.hpp"

#include "engine/threading/thread_pool.hpp"

#include "core/setting/setting.hpp"

namespace engine {

class Engine {
  public:
    void initialise();

    void update(GLFWwindow *window, float delta_time);

    void render();

  private:
    std::unique_ptr<engine::world::Sky> _sky;
    std::unique_ptr<engine::world::Cloud> _cloud;

    engine::threading::ThreadPool _thread_pool;

    float _time;

    std::unique_ptr<core::setting::Setting> _setting;
};

} // namespace engine
