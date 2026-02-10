#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "engine/shader/shader.hpp"

#include "engine/model/transform.hpp"

#include "engine/camera/frustum.hpp"
#include "engine/camera/direction.hpp"

#include "engine/camera/component/mvp.hpp"
#include "engine/camera/component/view.hpp"
#include "engine/camera/component/rotation.hpp"

namespace engine::camera {

class Camera {
  public:
    engine::model::Transform transform;

    Camera();

    Camera(glm::vec3 position);

    Camera(float x, float y, float z);

    void update(GLFWwindow *window, float delta_time);

    void update_view();

    void update_projection();

    void upload_view_projection(engine::shader::Shader &shader);

    void upload_model_view_projection(engine::shader::Shader &shader);

    void upload_position(engine::shader::Shader &shader);

    void move(Direction direction, float delta_time);

    void rotate(double x, double y);

    void scroll(double x, double y);

    glm::vec3 get_front();

    engine::camera::Frustum &get_frustum();

    void set_near(float near);

    void set_far(float far);

  private:
    static inline constexpr float _FIELD_OF_VIEW = 45.0f;

    static inline constexpr std::pair<float, float> _FIELD_OF_VIEW_LIMITS = {1.0f, 90.0f};

    static inline constexpr float _NEAR = 0.1f;
    static inline constexpr float _FAR = 100.0f;

    static inline constexpr float _PITCH_LIMIT = 89.0f;
    static inline constexpr float _SENSITIVITY = 0.025f;

    static inline constexpr float _SPEED = 10.0f;

    static inline const glm::vec3 _WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    engine::camera::component::MVP _MVP_component;

    engine::camera::component::View _view_component;

    engine::camera::component::Rotation _rotation_component;

    engine::camera::Frustum _frustum;

    float _field_of_view;
    float _aspect_ratio;
    float _near;
    float _far;

    glm::vec2 _last_cursor_position;

    void update_rotation_component(glm::vec2 offset);

    void update_view_component();

    void update_aspect_ratio();

    glm::vec2 get_cursor_offset(glm::vec2 cursor_position);
};

} // namespace engine::camera
