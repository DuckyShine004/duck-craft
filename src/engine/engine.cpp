#include <FastNoise/FastNoise.h>

#include "engine/engine.hpp"

#include "engine/shader/shader.hpp"

#include "manager/chunk_manager.hpp"
#include "manager/sound_manager.hpp"
#include "manager/camera_manager.hpp"
#include "manager/shader_manager.hpp"
#include "manager/display_manager.hpp"
#include "manager/texture_manager.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::world;

using namespace engine::shader;

using namespace engine::entity;

using namespace engine::camera;

using namespace manager;

namespace engine {

void Engine::initialise() {
    this->_time = 0.0f;

    this->_sky = std::make_unique<Sky>();
    this->_cloud = std::make_unique<Cloud>();

    ChunkManager &chunk_manager = ChunkManager::get_instance();

    CameraManager &camera_manager = CameraManager::get_instance();

    chunk_manager.set_thread_pool(this->_thread_pool);

    camera_manager.add_camera("player");
    camera_manager.add_camera("debug");

    /* NOTE: Current camera is set to 'player' */
    camera_manager.set_camera("player");
}

void Engine::update(GLFWwindow *window, float delta_time) {
    /* NOTE: Get the player camera */
    CameraManager &camera_manager = CameraManager::get_instance();

    SoundManager &sound_manager = SoundManager::get_instance();

    // sound_manager.play_music();

    Camera *camera = camera_manager.get_camera();

    camera->update(window, delta_time);

    /* NOTE: Load chunks */
    ChunkManager &chunk_manager = ChunkManager::get_instance();

    Camera *player_camera = camera_manager.get_camera("player");

    chunk_manager.load_chunks(player_camera);

    this->_cloud->update(camera->transform.position);

    chunk_manager.process_chunks(player_camera);

    this->_time += delta_time;
}

void Engine::render() {
    ChunkManager &chunk_manager = ChunkManager::get_instance();
    CameraManager &camera_manager = CameraManager::get_instance();
    TextureManager &texture_manager = TextureManager::get_instance();
    DisplayManager &display_manager = DisplayManager::get_instance();

    float display_width = display_manager.get_width();
    float display_height = display_manager.get_height();

    Camera *current_camera = camera_manager.get_camera();

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    ShaderManager &shader_manager = ShaderManager::get_instance();

    /* NOTE: Render sky */
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shader &sky = shader_manager.get_shader("sky");

    sky.use();

    sky.set_vector2f("u_resolution", display_width, display_height);

    current_camera->upload_inverse_view_projection(sky);

    this->_sky->render(sky);

    /* NOTE: Render OPAQUE voxels */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);

    Shader &scene = shader_manager.get_shader("scene");

    scene.use();

    scene.set_integer("u_block_texture_array", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_manager.get_texture_handle("block"));

    current_camera->upload_model_view_projection(scene);
    current_camera->upload_position(scene);

    chunk_manager.render_opaque(scene);

    /* NOTE: Render TRANSPARENT voxels */
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    chunk_manager.render_transparent(scene);

    /* NOTE: Render clouds */
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader &cloud = shader_manager.get_shader("cloud");

    cloud.use();

    cloud.set_float("u_time", this->_time);

    cloud.set_integer("u_cloud_texture_array", 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_manager.get_texture_handle("environment"));

    current_camera->upload_model_view_projection(cloud);
    current_camera->upload_position(cloud);

    this->_cloud->render(cloud);

    /* DEBUG: Render camera frustum */
    // glDisable(GL_CULL_FACE);
    //
    // Shader &debug = shader_manager.get_shader("debug");
    //
    for (Camera *camera : camera_manager.get_cameras()) {
        if (camera == current_camera) {
            continue;
        }

        camera->get_frustum().render(scene);
    }
}

} // namespace engine
