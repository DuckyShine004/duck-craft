#include <FastNoise/FastNoise.h>

#include "engine/engine.hpp"

#include "engine/shader/shader.hpp"

#include "manager/chunk_manager.hpp"
#include "manager/camera_manager.hpp"
#include "manager/shader_manager.hpp"
#include "manager/display_manager.hpp"
#include "manager/texture_manager.hpp"

#include "common/constant.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::shader;

using namespace engine::entity;

using namespace engine::camera;

using namespace manager;

using namespace common;

namespace engine {

void Engine::initialise() {
    ChunkManager &chunk_manager = ChunkManager::get_instance();

    CameraManager &camera_manager = CameraManager::get_instance();

    camera_manager.add_camera("player");
    camera_manager.add_camera("debug");

    camera_manager.set_camera("player");

    chunk_manager.generate_chunk(glm::vec3(0.0f));
    chunk_manager.generate_chunk(glm::vec3(-32.0f, 0.0f, 0.0f));
}

void Engine::update(GLFWwindow *window, float delta_time) {
    CameraManager &camera_manager = CameraManager::get_instance();

    Camera *camera = camera_manager.get_camera();

    // Camera *player_camera = camera_manager.get_camera("player");
    //
    // for (Cube &cube : this->_cubes) {
    //     if (player_camera->get_frustum().intersect(cube.get_aabb())) {
    //         cube.set_colour(GREEN_RGB);
    //     } else {
    //         cube.set_colour(WHITE_RGB);
    //     }
    // }

    camera->update(window, delta_time);
}

void Engine::render() {
    ChunkManager &chunk_manager = ChunkManager::get_instance();
    CameraManager &camera_manager = CameraManager::get_instance();
    TextureManager &texture_manager = TextureManager::get_instance();

    Camera *current_camera = camera_manager.get_camera();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    ShaderManager &shader_manager = ShaderManager::get_instance();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shader &scene = shader_manager.get_shader("scene");

    scene.use();

    scene.set_integer("u_block_texture_array", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_manager.get_texture_handle("blocks"));

    current_camera->upload_model_view_projection(scene);

    // for (Cube &cube : this->_cubes) {
    //     cube.render(scene);
    // }

    chunk_manager.render(scene);

    glDisable(GL_CULL_FACE);

    for (Camera *camera : camera_manager.get_cameras()) {
        if (camera == current_camera) {
            continue;
        }

        camera->get_frustum().render(scene);
    }
}

} // namespace engine
