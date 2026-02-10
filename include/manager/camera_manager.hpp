#pragma once

#include <map>
#include <memory>

#include "engine/camera/camera.hpp"

#include "manager/manager.hpp"

namespace manager {

class CameraManager final : public Manager {
  public:
    static CameraManager &get_instance();

    void initialise();

    void add_camera(std::string name);

    void set_camera(std::string name);

    void set_next();

    void set_previous();

    engine::camera::Camera *get_camera();

    engine::camera::Camera *get_camera(std::string name);

    std::vector<engine::camera::Camera *> get_cameras() const;

  private:
    std::map<std::string, std::unique_ptr<engine::camera::Camera>> _cameras;

    std::map<std::string, std::unique_ptr<engine::camera::Camera>>::iterator _camera_iterator;

    int _number_of_cameras;

    CameraManager();

    ~CameraManager();
};

} // namespace manager
