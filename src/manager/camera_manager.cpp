#include "manager/camera_manager.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::camera;

namespace manager {

CameraManager::CameraManager() : _number_of_cameras(0) {
    this->_camera_iterator = this->_cameras.end();
}

CameraManager::~CameraManager() = default;

CameraManager &CameraManager::get_instance() {
    static CameraManager instance;

    return instance;
}

void CameraManager::initialise() {
}

void CameraManager::add_camera(std::string name) {
    auto [iterator, is_added] = this->_cameras.emplace(name, std::make_unique<Camera>());

    if (!is_added) {
        LOG_WARN("Camera with the name: {}, already exists.", name);
        return;
    }

    if (this->_number_of_cameras == 0) {
        this->_camera_iterator = this->_cameras.begin();
    }

    ++this->_number_of_cameras;
}

void CameraManager::set_camera(std::string name) {
    auto iterator = this->_cameras.find(name);

    if (iterator == this->_cameras.end()) {
        throw std::runtime_error(fmt::format("There is no camera with the name: {}", name));
    }

    this->_camera_iterator = iterator;
}

void CameraManager::set_next() {
    if (this->_number_of_cameras <= 1) {
        LOG_WARN("There are {} cameras. Cannot switch to next camera.", this->_number_of_cameras);
        return;
    }

    if (this->_camera_iterator == this->_cameras.end()) {
        this->_camera_iterator = this->_cameras.begin();
    } else {
        ++this->_camera_iterator;

        if (this->_camera_iterator == this->_cameras.end()) {
            this->_camera_iterator = this->_cameras.begin();
        }
    }

    LOG_INFO("Current camera: {}", this->_camera_iterator->first);
}

void CameraManager::set_previous() {
    if (this->_number_of_cameras <= 1) {
        LOG_WARN("There are {} cameras. Cannot switch to previous camera.", this->_number_of_cameras);
        return;
    }

    if (this->_camera_iterator == this->_cameras.begin()) {
        this->_camera_iterator = this->_cameras.end();
    } else {
        --this->_camera_iterator;

        if (this->_camera_iterator == this->_cameras.begin()) {
            this->_camera_iterator = this->_cameras.end();
        }
    }

    LOG_INFO("Current camera: {}", this->_camera_iterator->first);
}

engine::camera::Camera *CameraManager::get_camera() {
    if (this->_number_of_cameras == 0) {
        return nullptr;
    }

    return this->_camera_iterator->second.get();
}

engine::camera::Camera *CameraManager::get_camera(std::string name) {
    auto iterator = this->_cameras.find(name);

    if (iterator == this->_cameras.end()) {
        throw std::runtime_error(fmt::format("There is no camera with the name: {}", name));
    }

    return iterator->second.get();
}

std::vector<engine::camera::Camera *> CameraManager::get_cameras() const {
    std::vector<Camera *> cameras;

    cameras.reserve(this->_number_of_cameras);

    for (const auto &[name, camera] : this->_cameras) {
        cameras.push_back(camera.get());
    }

    return cameras;
}

} // namespace manager
