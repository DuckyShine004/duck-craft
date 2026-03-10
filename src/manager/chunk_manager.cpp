#define GLM_ENABLE_EXPERIMENTAL

#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>

#include "manager/chunk_manager.hpp"

#include "utility/math_utility.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::world;

using namespace engine::camera;

using namespace engine::shader;

using namespace engine::threading;

using namespace utility;

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

    if (chunk->get_state() != ChunkState::EMPTY) {
        return;
    }

    chunk->set_state(ChunkState::GENERATING_TERRAIN);

    this->_thread_pool->push([this, chunk_id]() {
        Chunk *chunk = this->_world->chunks[chunk_id].get();

        chunk->generate(*this->_world);

        chunk->set_is_terrain_generation_complete(true);

        for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
            int nx = Face::I_NORMALS[face_type_index][0];
            int ny = Face::I_NORMALS[face_type_index][1];
            int nz = Face::I_NORMALS[face_type_index][2];

            int dx = chunk->local_x + nx;
            int dy = chunk->local_y + ny;
            int dz = chunk->local_z + nz;

            Chunk *adjacent_chunk = this->_world->find_chunk(dx, dy, dz);

            if (adjacent_chunk == nullptr) {
                continue;
            }

            int opposite_face_type_index = (face_type_index & 1) ? face_type_index - 1 : face_type_index + 1;

            adjacent_chunk->set_dirty_border_state(opposite_face_type_index, true);
        }

        chunk->propagate_sunlight(*this->_world);

        chunk->set_state(ChunkState::OCCLUDING_FACES);

        chunk->occlude_faces(*this->_world);

        chunk->set_state(ChunkState::GENERATING_MESH);

        chunk->generate_mesh(*this->_world);

        chunk->set_state(ChunkState::UPLOADING_MESH);
    });

    // Chunk *chunk = this->_world->chunks[chunk_id].get();
    //
    // chunk->set_state(ChunkState::GENERATING_TERRAIN);
    //
    // chunk->generate(*this->_world);
    //
    // chunk->set_is_terrain_generation_complete(true);
    //
    // for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
    //     int nx = Face::I_NORMALS[face_type_index][0];
    //     int ny = Face::I_NORMALS[face_type_index][1];
    //     int nz = Face::I_NORMALS[face_type_index][2];
    //
    //     int dx = chunk->local_x + nx;
    //     int dy = chunk->local_y + ny;
    //     int dz = chunk->local_z + nz;
    //
    //     Chunk *adjacent_chunk = this->_world->find_chunk(dx, dy, dz);
    //
    //     if (adjacent_chunk == nullptr) {
    //         continue;
    //     }
    //
    //     int opposite_face_type_index = (face_type_index & 1) ? face_type_index - 1 : face_type_index + 1;
    //
    //     adjacent_chunk->set_dirty_border_state(opposite_face_type_index, true);
    // }
    //
    // chunk->propagate_sunlight(*this->_world);
    //
    // chunk->set_state(ChunkState::OCCLUDING_FACES);
    //
    // chunk->occlude_faces(*this->_world);
    //
    // if (chunk->has_dirty_borders()) {
    //     return;
    // }
    //
    // chunk->set_state(ChunkState::GENERATING_MESH);
    //
    // chunk->generate_mesh(*this->_world);
    //
    // if (chunk->has_dirty_borders()) {
    //     return;
    // }
    //
    // chunk->set_state(ChunkState::UPLOADING_MESH);
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

                this->generate_chunk_at_local_position(x, y, z);

                this->_loaded_chunk_ids.push_back(chunk_id);
            }
        }
    }

    /* NOTE: Sort by depth (distance to camera along view direction) */
    std::sort(this->_loaded_chunk_ids.begin(), this->_loaded_chunk_ids.end(), [&](const auto &chunk_a_id, const auto &chunk_b_id) -> bool {
        Chunk *chunk_a = this->_world->chunks[chunk_a_id].get();
        Chunk *chunk_b = this->_world->chunks[chunk_b_id].get();

        glm::vec3 chunk_a_centre = glm::vec3(chunk_a->global_x, chunk_a->global_y, chunk_a->global_z) + config::CHUNK_SIZE_HALF_F;
        glm::vec3 chunk_b_centre = glm::vec3(chunk_b->global_x, chunk_b->global_y, chunk_b->global_z) + config::CHUNK_SIZE_HALF_F;

        float depth_a = glm::dot(chunk_a_centre - camera->transform.position, camera->get_front());
        float depth_b = glm::dot(chunk_b_centre - camera->transform.position, camera->get_front());

        return depth_a < depth_b;
    });
}

void ChunkManager::process_chunks() {
    // LOG_INFO("Loaded chunk size: {}", this->_loaded_chunk_ids.size());

    for (std::uint32_t &chunk_id : this->_loaded_chunk_ids) {
        Chunk *chunk = this->_world->chunks[chunk_id].get();

        // NOTE: Maybe don't need CAS?
        if (chunk->is_terrain_generation_complete()) {
            // Check if the chunk has dirty borders due to incomplete face occlusion

            if (chunk->has_dirty_borders()) {
                // If we are already meshing, then we skip
                if (chunk->get_state() < ChunkState::UPLOADING_MESH) {
                    continue;
                }

                if (!chunk->can_dirty_border_task_run()) {
                    continue;
                }

                /* FIX: could pass chunk id, since we don't know when thread might execute, and may lead to dangling pointer */
                this->_thread_pool->push([this, chunk]() {
                    glm::ivec2 height_map_id(chunk->local_x, chunk->local_z);

                    chunk->propagate_sunlight(*this->_world);

                    chunk->occlude_dirty_borders(*this->_world);

                    if (!chunk->has_dirty_borders()) {
                        chunk->generate_mesh(*this->_world);

                        chunk->set_state(ChunkState::UPLOADING_MESH);
                    }

                    chunk->set_is_dirty_border_task_running(false);
                });
                // glm::ivec2 height_map_id(chunk->local_x, chunk->local_z);
                //
                // chunk->propagate_sunlight(*this->_world);
                //
                // chunk->occlude_dirty_borders(*this->_world);
                //
                // if (!chunk->has_dirty_borders()) {
                //     chunk->generate_mesh(*this->_world);
                //
                //     chunk->set_state(ChunkState::UPLOADING_MESH);
                // }
                //
                // chunk->set_is_dirty_border_task_running(false);
            }
        }

        ChunkState chunk_state = chunk->get_state();

        switch (chunk_state) {
            case ChunkState::UPLOADING_MESH:
                chunk->upload_mesh();
                break;
            default:
                break;
        }
    }
}

void ChunkManager::render(Shader &shader) {
    for (std::uint32_t &chunk_id : this->_loaded_chunk_ids) {
        Chunk *chunk = this->_world->chunks[chunk_id].get();

        if (chunk->get_state() == ChunkState::RENDERING) {
            chunk->render(shader);
        }

        chunk->get_aabb().render(shader);
    }
}

void ChunkManager::set_thread_pool(ThreadPool &thread_pool) {
    this->_thread_pool = &thread_pool;
}

} // namespace manager
