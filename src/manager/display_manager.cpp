#include "manager/display_manager.hpp"

namespace manager {

DisplayManager::DisplayManager() = default;

DisplayManager::~DisplayManager() = default;

DisplayManager &DisplayManager::get_instance() {
    static DisplayManager instance;

    return instance;
}

void DisplayManager::initialise() {
    this->_width = this->WIDTH;
    this->_height = this->HEIGHT;
}

void DisplayManager::update(int width, int height) {
    this->_width = width;
    this->_height = height;

    this->_is_window_resized = true;
}

int DisplayManager::get_width() {
    return this->_width;
}

int DisplayManager::get_height() {
    return this->_height;
}

void DisplayManager::set_is_window_resized(bool is_window_resized) {
    this->_is_window_resized = false;
}

bool DisplayManager::is_window_resized() {
    return this->_is_window_resized;
}

} // namespace manager
