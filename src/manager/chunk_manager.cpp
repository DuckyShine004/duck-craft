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

    HeightMap *height_map_pointer = height_map_iterator->second.get();

    glm::ivec3 chunk_id(local_x, local_y, local_z);

    if (!this->_chunks.contains(chunk_id)) {
        this->_chunks.insert({chunk_id, std::make_unique<Chunk>(global_x, global_y, global_z)});
    }

    this->_thread_pool->push([this, chunk_id, height_map_pointer]() {
        Chunk *chunk_pointer;

        this->_chunks.visit(chunk_id, [&](const auto &pair) {
            chunk_pointer = pair.second.get();
        });

        chunk_pointer->set_state(ChunkState::GENERATING_TERRAIN);

        chunk_pointer->generate(*this->_generator, *height_map_pointer);

        chunk_pointer->set_state(ChunkState::OCCLUDING_FACES);

        chunk_pointer->occlude_faces(this->_chunks);

        chunk_pointer->set_state(ChunkState::GENERATING_MESH);

        chunk_pointer->generate_mesh();

        chunk_pointer->set_state(ChunkState::UPLOADING_MESH);
    });
}

// TODO: Should instead iterate through chunks that are not culled by frustum
void ChunkManager::process_chunks() {
    this->_chunks.visit_all([](const auto &chunk_iterator) {
        Chunk *chunk = chunk_iterator.second.get();

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
