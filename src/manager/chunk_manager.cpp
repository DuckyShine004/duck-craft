#define GLM_ENABLE_EXPERIMENTAL

#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>

#include "manager/chunk_manager.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::world;

using namespace engine::camera;

using namespace engine::shader;

using namespace engine::threading;

namespace manager {

ChunkManager::ChunkManager() : _thread_pool(nullptr) {
}

ChunkManager::~ChunkManager() = default;

ChunkManager &ChunkManager::get_instance() {
    static ChunkManager instance;

    return instance;
}

void ChunkManager::initialise() {
    this->_world = std::make_unique<World>();

    const int HORIZONTAL_DISTANCE = (config::HORIZONTAL_RENDER_DISTANCE << 1) + 1;
    const int VERTICAL_DISTANCE = (config::VERTICAL_RENDER_DISTANCE << 1) + 1;

    std::size_t max_loaded_chunks_load = HORIZONTAL_DISTANCE * HORIZONTAL_DISTANCE * VERTICAL_DISTANCE;

    this->_loaded_chunk_ids.reserve(max_loaded_chunks_load);
}

void ChunkManager::generate_chunk_at_global_position(const glm::vec3 &position) {
    int global_x = static_cast<int>(position.x);
    int global_y = static_cast<int>(position.y);
    int global_z = static_cast<int>(position.z);

    int local_x = global_x >> config::CHUNK_SIZE_BITS;
    int local_y = global_y >> config::CHUNK_SIZE_BITS;
    int local_z = global_z >> config::CHUNK_SIZE_BITS;

    this->generate_chunk_at_local_position(local_x, local_y, local_z);
}

void ChunkManager::generate_chunk_at_local_position(int local_x, int local_y, int local_z) {
    int global_x = local_x << config::CHUNK_SIZE_BITS;
    int global_y = local_y << config::CHUNK_SIZE_BITS;
    int global_z = local_z << config::CHUNK_SIZE_BITS;

    this->_world->try_emplace_height_map(local_x, local_z, global_x, global_z);

    std::uint32_t chunk_id = this->_world->try_emplace_chunk_id(local_x, local_y, local_z, global_x, global_y, global_z);

    Chunk *chunk = this->_world->chunks[chunk_id].get();

    if (!chunk->is_task_running(ChunkTask::TERRAIN_GENERATION) && !chunk->is_state_set(ChunkState::TERRAIN_GENERATED)) {
        chunk->queue_tasks(ChunkTask::TERRAIN_GENERATION);
    }
}

void ChunkManager::load_chunks(Camera *camera) {
    this->_loaded_chunk_ids.clear();

    int local_x = static_cast<int>(camera->transform.position.x) >> config::CHUNK_SIZE_BITS;
    int local_y = static_cast<int>(camera->transform.position.y) >> config::CHUNK_SIZE_BITS;
    int local_z = static_cast<int>(camera->transform.position.z) >> config::CHUNK_SIZE_BITS;

    for (int z = local_z - config::HORIZONTAL_RENDER_DISTANCE; z <= local_z + config::HORIZONTAL_RENDER_DISTANCE; ++z) {
        for (int y = local_y - config::VERTICAL_RENDER_DISTANCE; y <= local_y + config::VERTICAL_RENDER_DISTANCE; ++y) {
            for (int x = local_x - config::HORIZONTAL_RENDER_DISTANCE; x <= local_x + config::HORIZONTAL_RENDER_DISTANCE; ++x) {
                int global_x = x << config::CHUNK_SIZE_BITS;
                int global_y = y << config::CHUNK_SIZE_BITS;
                int global_z = z << config::CHUNK_SIZE_BITS;

                std::uint32_t chunk_id = this->_world->try_emplace_chunk_id(x, y, z, global_x, global_y, global_z);

                Chunk *chunk = this->_world->chunks[chunk_id].get();

                if (!camera->get_frustum().intersect(chunk->get_aabb())) {
                    continue;
                }

                this->_world->try_emplace_height_map(x, z, global_x, global_z);

                if (!chunk->is_task_running(ChunkTask::TERRAIN_GENERATION) && !chunk->is_state_set(ChunkState::TERRAIN_GENERATED)) {
                    chunk->queue_tasks(ChunkTask::TERRAIN_GENERATION);
                }

                this->_loaded_chunk_ids.push_back(chunk_id);
            }
        }
    }

    glm::vec3 camera_position = camera->transform.position;

    /* NOTE: Sort by depth (distance to camera along view direction), also camera could be moving, so we create a copy of the position first */
    std::sort(this->_loaded_chunk_ids.begin(), this->_loaded_chunk_ids.end(), [&](const auto &chunk_a_id, const auto &chunk_b_id) -> bool {
        Chunk *chunk_a = this->_world->chunks[chunk_a_id].get();
        Chunk *chunk_b = this->_world->chunks[chunk_b_id].get();

        glm::vec3 chunk_a_centre = glm::vec3(chunk_a->global_x, chunk_a->global_y, chunk_a->global_z) + config::CHUNK_SIZE_HALF_F;
        glm::vec3 chunk_b_centre = glm::vec3(chunk_b->global_x, chunk_b->global_y, chunk_b->global_z) + config::CHUNK_SIZE_HALF_F;

        float depth_a = glm::dot(chunk_a_centre - camera_position, camera->get_front());
        float depth_b = glm::dot(chunk_b_centre - camera_position, camera->get_front());

        return depth_a > depth_b;
    });
}

void ChunkManager::process_chunks(Camera *camera) {
    for (std::uint32_t &chunk_id : this->_loaded_chunk_ids) {
        Chunk *chunk = this->_world->chunks[chunk_id].get();

        if (chunk->is_task_queue_empty()) {
            continue;
        }

        if (chunk->can_generate_terrain()) {
            chunk->set_running_task(ChunkTask::TERRAIN_GENERATION);

            this->_thread_pool->push([this, chunk]() {
                chunk->generate_terrain(*this->_world);
            });
        } else if (chunk->can_propagate_sunlight()) {
            chunk->set_running_task(ChunkTask::LIGHT_PROPAGATION);

            this->_thread_pool->push([this, chunk]() {
                chunk->propagate_sunlight(*this->_world);

                for (Chunk *neighbour : chunk->neighbours) {
                    if (neighbour == nullptr) {
                        continue;
                    }

                    neighbour->queue_tasks(ChunkTask::MESH_GENERATION);
                }
            });
        } else if (chunk->can_generate_mesh()) {
            chunk->set_running_task(ChunkTask::MESH_GENERATION);

            this->_thread_pool->push([this, chunk, camera]() {
                chunk->generate_mesh(*camera);
            });
        } else if (chunk->can_upload_mesh()) {
            chunk->set_running_task(ChunkTask::MESH_UPLOAD);

            chunk->upload_mesh();
        }

        chunk->process_dirty_neighbours_sunlight();
    }
}

void ChunkManager::render_water(Shader &shader) {
    for (std::uint32_t &chunk_id : this->_loaded_chunk_ids) {
        Chunk *chunk = this->_world->chunks[chunk_id].get();

        if (chunk->is_state_set(ChunkState::RENDERING)) {
            chunk->render_water(shader);
        }
    }
}

void ChunkManager::render_opaque(Shader &shader) {
    for (std::uint32_t &chunk_id : this->_loaded_chunk_ids) {
        Chunk *chunk = this->_world->chunks[chunk_id].get();

        if (chunk->is_state_set(ChunkState::RENDERING)) {
            chunk->render_opaque(shader);
        }

        chunk->get_aabb().render(shader);
    }
}

void ChunkManager::render_transparent(Shader &shader) {
    for (std::uint32_t &chunk_id : this->_loaded_chunk_ids) {
        Chunk *chunk = this->_world->chunks[chunk_id].get();

        if (chunk->is_state_set(ChunkState::RENDERING)) {
            chunk->render_transparent(shader);
        }
    }
}

void ChunkManager::set_thread_pool(ThreadPool &thread_pool) {
    this->_thread_pool = &thread_pool;
}

} // namespace manager
