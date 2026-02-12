#include "manager/chunk_manager.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::world;

using namespace engine::shader;

namespace manager {

ChunkManager::ChunkManager() = default;

ChunkManager::~ChunkManager() = default;

ChunkManager &ChunkManager::get_instance() {
    static ChunkManager instance;

    return instance;
}

void ChunkManager::initialise() {
    this->_generator = std::make_shared<Generator>();
}

// TODO: Thread entire chunk generation, including heightmap and chunk map
void ChunkManager::generate_chunk(const glm::vec3 &position) {
    int global_x = static_cast<int>(position.x);
    int global_y = static_cast<int>(position.y);
    int global_z = static_cast<int>(position.z);

    int local_x = (global_x >= 0) ? global_x >> config::CHUNK_SIZE_BITS : -(-global_x >> config::CHUNK_SIZE_BITS);
    int local_y = (global_y >= 0) ? global_y >> config::CHUNK_SIZE_BITS : -(-global_y >> config::CHUNK_SIZE_BITS);
    int local_z = (global_z >= 0) ? global_y >> config::CHUNK_SIZE_BITS : -(-global_z >> config::CHUNK_SIZE_BITS);

    global_x = (local_x >= 0) ? local_x << config::CHUNK_SIZE_BITS : -(-local_x << config::CHUNK_SIZE_BITS);
    global_y = (local_y >= 0) ? local_y << config::CHUNK_SIZE_BITS : -(-local_y << config::CHUNK_SIZE_BITS);
    global_z = (local_z >= 0) ? local_z << config::CHUNK_SIZE_BITS : -(-local_z << config::CHUNK_SIZE_BITS);

    glm::ivec2 height_map_id(local_x, local_z);

    auto height_map_iterator = this->_height_maps.find(height_map_id);

    if (height_map_iterator == this->_height_maps.end()) {
        std::unique_ptr<HeightMap> height_map = std::make_unique<HeightMap>();

        height_map->generate(*this->_generator, global_x, global_z);

        auto [iterator, is_emplaced] = this->_height_maps.emplace(height_map_id, std::move(height_map));

        height_map_iterator = iterator;
    }

    glm::ivec3 chunk_id(local_x, local_y, local_z);

    auto chunk_iterator = this->_chunks.find(chunk_id);

    if (chunk_iterator == this->_chunks.end()) {
        std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(global_x, global_y, global_z);

        // TODO: Child thread chunk generation
        chunk->generate(*this->_generator, this->_chunks, *height_map_iterator->second);

        // TODO: Child thread mesh generation
        chunk->generate_mesh();

        auto [iterator, is_emplaced] = this->_chunks.emplace(chunk_id, std::move(chunk));

        // TODO: Child thread- cull neighbour meshes and regenerate mesh if needed
    }
}

void ChunkManager::render(Shader &shader) {
    for (auto &chunk_iterator : this->_chunks) {
        chunk_iterator.second.get()->render(shader);
    }
}

} // namespace manager
