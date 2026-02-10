#pragma once

#include "manager/manager.hpp"

namespace manager {

class DisplayManager final : public Manager {
  public:
    static inline constexpr int WIDTH = 1280;
    static inline constexpr int HEIGHT = 720;

    static DisplayManager &get_instance();

    void initialise();

    void update(int width, int height);

    int get_width();
    int get_height();

    void set_is_window_resized(bool is_window_resized);

    bool is_window_resized();

  private:
    int _width;
    int _height;

    bool _is_window_resized;

    DisplayManager();

    ~DisplayManager();
};

}; // namespace manager
