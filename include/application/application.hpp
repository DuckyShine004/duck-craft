#pragma once

#include <GLFW/glfw3.h>

#include "engine/engine.hpp"

namespace application {

class Application {
  public:
    Application();

    ~Application();

    bool initialise();

    void load();

    void run();

  private:
    GLFWwindow *_window;

    engine::Engine _engine;

    float _last_time;

    static void on_key_press(GLFWwindow *window, int key, int scanmode, int action, int mods);

    static void on_window_resize(GLFWwindow *window, int width, int height);

    static void on_cursor(GLFWwindow *window, double x, double y);

    static void on_scroll(GLFWwindow *window, double x, double y);

    void update(engine::Engine &engine);

    void render(engine::Engine &engine);

    void handle_key_press(GLFWwindow *window, int key, int scanmode, int action, int mods);

    void handle_window_resize(GLFWwindow *window, int width, int height);

    void handle_cursor(GLFWwindow *window, double x, double y);

    void handle_scroll(GLFWwindow *window, double x, double y);

    void on_cleanup();
};

} // namespace application
