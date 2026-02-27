#include "manager/chunk_manager.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::world;

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
    this->_generator = std::make_shared<Generator>();
}

void ChunkManager::generate_chunk(const glm::vec3 &position) {
    int global_x = static_cast<int>(position.x);
    int global_y = static_cast<int>(position.y);
    int global_z = static_cast<int>(position.z);

    int local_x = global_x >> config::CHUNK_SIZE_BITS;
    int local_y = global_y >> config::CHUNK_SIZE_BITS;
    int local_z = global_z >> config::CHUNK_SIZE_BITS;

    global_x = local_x << config::CHUNK_SIZE_BITS;
    global_y = local_y << config::CHUNK_SIZE_BITS;
    global_z = local_z << config::CHUNK_SIZE_BITS;

    glm::ivec2 height_map_id(local_x, local_z);

    auto height_map_iterator = this->_height_maps.find(height_map_id);

    if (height_map_iterator == this->_height_maps.end()) {
        std::unique_ptr<HeightMap> height_map = std::make_unique<HeightMap>();

        height_map->generate(*this->_generator, global_x, global_z);

        auto [iterator, is_emplaced] = this->_height_maps.emplace(height_map_id, std::move(height_map));

        height_map_iterator = iterator;
    }

    HeightMap *height_map = height_map_iterator->second.get();

    glm::ivec3 chunk_id(local_x, local_y, local_z);

    if (!this->_chunks.contains(chunk_id)) {
        this->_chunks.insert({chunk_id, std::make_unique<Chunk>(global_x, global_y, global_z)});
    }

    this->_thread_pool->push([this, chunk_id, height_map]() {
        Chunk *chunk;

        this->_chunks.visit(chunk_id, [&](const auto &pair) {
            chunk = pair.second.get();
        });

        chunk->set_state(ChunkState::GENERATING_TERRAIN);

        chunk->generate(*this->_generator, *height_map);

        chunk->set_is_terrain_generation_complete(true);

        for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
            int nx = Face::I_NORMALS[face_type_index][0];
            int ny = Face::I_NORMALS[face_type_index][1];
            int nz = Face::I_NORMALS[face_type_index][2];

            glm::ivec3 adjacent_chunk_id(chunk->local_x + nx, chunk->local_y + ny, chunk->local_z + nz);

            Chunk *adjacent_chunk = nullptr;

            this->_chunks.visit(adjacent_chunk_id, [&](const auto &chunk_iterator) {
                adjacent_chunk = chunk_iterator.second.get();
            });

            if (adjacent_chunk == nullptr) {
                continue;
            }

            int opposite_face_type_index = (face_type_index & 1) ? face_type_index - 1 : face_type_index + 1;

            adjacent_chunk->set_dirty_border_state(opposite_face_type_index, true);
        }

        chunk->propagate_sunlight(this->_chunks, *height_map);

        chunk->set_state(ChunkState::OCCLUDING_FACES);

        chunk->occlude_faces(this->_chunks);

        if (chunk->has_dirty_borders()) {
            return;
        }

        chunk->set_state(ChunkState::GENERATING_MESH);

        chunk->generate_mesh(this->_chunks);

        if (chunk->has_dirty_borders()) {
            return;
        }

        chunk->set_state(ChunkState::UPLOADING_MESH);
    });
}

// TODO: Should instead iterate through chunks that are not culled by frustum
void ChunkManager::process_chunks() {
    this->_chunks.visit_all([&](const auto &chunk_iterator) {
        Chunk *chunk = chunk_iterator.second.get();

        // Check if the chunk generation is complete
        // NOTE: Maybe don't need CAS?
        if (chunk->is_terrain_generation_complete()) {
            // Check if the chunk has dirty borders due to incomplete face occlusion

            if (chunk->has_dirty_borders()) {
                if (!chunk->can_dirty_border_task_run()) {
                    return;
                }

                this->_thread_pool->push([this, chunk]() {
                    glm::ivec2 height_map_id(chunk->local_x, chunk->local_z);

                    HeightMap *height_map = this->_height_maps.at(height_map_id).get();

                    chunk->propagate_sunlight(this->_chunks, *height_map);

                    chunk->occlude_dirty_borders(this->_chunks);

                    if (!chunk->has_dirty_borders()) {
                        chunk->generate_mesh(this->_chunks);

                        chunk->set_state(ChunkState::UPLOADING_MESH);
                    }

                    chunk->set_is_dirty_border_task_running(false);
                });

                return;
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
    });
}

void ChunkManager::render(Shader &shader) {
    if (this->_chunks.empty()) {
        return;
    }

    this->_chunks.visit_all([&](const auto &chunk_iterator) {
        Chunk *chunk = chunk_iterator.second.get();

        if (chunk->get_state() == ChunkState::RENDERING) {
            chunk_iterator.second.get()->render(shader);
        }
    });
}

void ChunkManager::set_thread_pool(ThreadPool &thread_pool) {
    this->_thread_pool = &thread_pool;
}

} // namespace manager
