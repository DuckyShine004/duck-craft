#include <GLFW/glfw3.h>

#include "external/glad/glad.h"

#include "application/application.hpp"

#include "engine/engine.hpp"

#include "manager/chunk_manager.hpp"
#include "manager/shader_manager.hpp"
#include "manager/camera_manager.hpp"
#include "manager/display_manager.hpp"
#include "manager/texture_manager.hpp"

#include "common/constant.hpp"

#include "logger/logger_macros.hpp"

using namespace engine;

using namespace manager;

namespace application {

Application::Application() : _last_time(0.0f) {
}

Application::~Application() {
    this->on_cleanup();
}

bool Application::initialise() {
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialise GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    GLFWwindow *window = glfwCreateWindow(DisplayManager::WIDTH, DisplayManager::HEIGHT, common::TITLE, nullptr, nullptr);

    if (window == nullptr) {
        LOG_ERROR("Failed to create window");
        this->on_cleanup();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialise GLAD");
        this->on_cleanup();
        return false;
    }

    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(window, Application::on_key_press);
    glfwSetFramebufferSizeCallback(window, Application::on_window_resize);
    glfwSetCursorPosCallback(window, Application::on_cursor);
    glfwSetScrollCallback(window, Application::on_scroll);

    this->_window = window;

    return true;
}

void Application::load() {
    int width;
    int height;

    glfwGetFramebufferSize(this->_window, &width, &height);

    glViewport(0, 0, width, height);

    ChunkManager::get_instance().initialise();
    ShaderManager::get_instance().initialise();
    CameraManager::get_instance().initialise();
    DisplayManager::get_instance().initialise();
    TextureManager::get_instance().initialise();

    this->_engine.initialise();
}

void Application::run() {
    glfwSwapInterval(0);

    while (!glfwWindowShouldClose(this->_window)) {
        this->update(this->_engine);
        this->render(this->_engine);

        glfwSwapBuffers(this->_window);
        glfwPollEvents();
    }

    this->on_cleanup();
}

void Application::update(Engine &engine) {
    float time = glfwGetTime();

    float delta_time = time - this->_last_time;

    this->_last_time = time;

    engine.update(this->_window, delta_time);
}

void Application::render(Engine &engine) {
    const float R = 50.0f / 255.0f;
    const float G = 51.0f / 255.0f;
    const float B = 76.0f / 255.0f;

    glClearColor(R, G, B, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    engine.render();
}

void Application::on_key_press(GLFWwindow *window, int key, int scanmode, int action, int mods) {
    Application *application = static_cast<Application *>(glfwGetWindowUserPointer(window));

    application->handle_key_press(window, key, scanmode, action, mods);
}

void Application::handle_key_press(GLFWwindow *window, int key, int scanmode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        int cursor_mode = glfwGetInputMode(this->_window, GLFW_CURSOR);

        if (cursor_mode == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(this->_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(this->_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        CameraManager::get_instance().set_next();
    }
}

void Application::on_window_resize(GLFWwindow *window, int width, int height) {
    Application *application = static_cast<Application *>(glfwGetWindowUserPointer(window));

    application->handle_window_resize(window, width, height);
}

void Application::handle_window_resize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    DisplayManager::get_instance().update(width, height);
}

void Application::on_cursor(GLFWwindow *window, double x, double y) {
    Application *application = static_cast<Application *>(glfwGetWindowUserPointer(window));

    application->handle_cursor(window, x, y);
}

void Application::handle_cursor(GLFWwindow *window, double x, double y) {
    CameraManager &camera_manager = CameraManager::get_instance();

    camera::Camera *camera = CameraManager::get_instance().get_camera();

    if (camera == nullptr) {
        LOG_WARN("There are no active cameras. Cannot scroll camera.");
        return;
    }

    camera->rotate(x, y);
}

void Application::on_scroll(GLFWwindow *window, double x, double y) {
    Application *application = static_cast<Application *>(glfwGetWindowUserPointer(window));

    application->handle_scroll(window, x, y);
}

void Application::handle_scroll(GLFWwindow *window, double x, double y) {
    CameraManager &camera_manager = CameraManager::get_instance();

    camera::Camera *camera = CameraManager::get_instance().get_camera();

    if (camera == nullptr) {
        LOG_WARN("There are no active cameras. Cannot scroll camera.");
        return;
    }

    camera->scroll(x, y);
}

void Application::on_cleanup() {
    if (this->_window != nullptr) {
        glfwDestroyWindow(this->_window);

        this->_window = nullptr;
    }

    glfwTerminate();
}

} // namespace application
