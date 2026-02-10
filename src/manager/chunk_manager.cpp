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

// TODO: Thread
void ChunkManager::generate_chunk(const glm::vec3 &position) {
    int global_x = static_cast<int>(position.x);
    int global_y = static_cast<int>(position.y);
    int global_z = static_cast<int>(position.z);

    int local_x = global_x >> config::CHUNK_SIZE_BITS;
    int local_y = global_y >> config::CHUNK_SIZE_BITS;
    int local_z = global_z >> config::CHUNK_SIZE_BITS;

    int height_map_id = this->get_height_map_local_id(local_x, local_z);

    auto height_map_iterator = this->_height_maps.find(height_map_id);

    if (height_map_iterator == this->_height_maps.end()) {
        std::unique_ptr<HeightMap> height_map = std::make_unique<HeightMap>();

        height_map->generate(*this->_generator, global_x, global_z);

        auto [iterator, is_emplaced] = this->_height_maps.emplace(height_map_id, std::move(height_map));

        height_map_iterator = iterator;
    }

    int chunk_id = this->get_chunk_local_id(local_x, local_y, local_z);

    auto chunk_iterator = this->_chunks.find(chunk_id);

    if (chunk_iterator == this->_chunks.end()) {
        std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(global_x, global_y, global_z);

        chunk->generate(*this->_generator, *height_map_iterator->second);

        chunk->generate_mesh();

        auto [iterator, is_emplaced] = this->_chunks.emplace(chunk_id, std::move(chunk));
    }
}

// void ChunkManager::generate_chunk_local(int x, int y, int z) {
//     int height_map_id =
//
//     int chunk_id = this->get_chunk_local_id(x,y,z);
//
//     auto iterator = this->_chunks.find(chunk_id);
//
//     if(iterator == this->_chunks.end()) {
//         std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
//     }
// }

void ChunkManager::render(Shader &shader) {
    this->_chunks[0]->render(shader);
}

// Chunk &ChunkManager::get_chunk(glm::vec3 &position) {
//     int global_x = static_cast<int>(position.x);
//     int global_y = static_cast<int>(position.y);
//     int global_z = static_cast<int>(position.z);
//
//     int local_x = global_x >> config::CHUNK_SIZE_BITS;
//     int local_y = global_y >> config::CHUNK_SIZE_BITS;
//     int local_z = global_z >> config::CHUNK_SIZE_BITS;
//
//     int id = local_x + (local_y << config::CHUNK_SIZE_BITS) + (local_z << config::CHUNK_SIZE_BITS2);
//
//     return *this->_chunks[id];
// }

Chunk &ChunkManager::get_chunk_local(int local_chunk_x, int local_chunk_y, int local_chunk_z) {
    int id = local_chunk_x + (local_chunk_y << config::CHUNK_SIZE_BITS) + (local_chunk_z << config::CHUNK_SIZE_BITS2);

    return *this->_chunks[id];
}

int ChunkManager::get_chunk_local_id(int local_chunk_x, int local_chunk_y, int local_chunk_z) {
    return local_chunk_x + (local_chunk_y << config::CHUNK_SIZE_BITS) + (local_chunk_z << config::CHUNK_SIZE_BITS2);
}

HeightMap &ChunkManager::get_height_map_local(int local_chunk_x, int local_chunk_z) {
    int id = local_chunk_x + (local_chunk_z << config::CHUNK_SIZE_BITS);

    return *this->_height_maps[id];
}

int ChunkManager::get_height_map_local_id(int local_chunk_x, int local_chunk_z) {
    return local_chunk_x + (local_chunk_z << config::CHUNK_SIZE_BITS);
}

// Chunk *ChunkManager::get_block(int x, int y, int z) {
//     if (x < 0 || y < 0 || z < 0) {
//         return nullptr;
//     }
//
//     int local_x = x >> Chunk::SIZE_BITS;
//     int local_y = y >> Chunk::SIZE_BITS;
//     int local_z = z >> Chunk::SIZE_BITS;
//
//     int id = local_x + (local_y << Chunk::SIZE_BITS) + (local_z << Chunk::SIZE_BITS2);
//
//     if (id >= 128) {
//         return nullptr;
//     }
//
//     Chunk *chunk = this->_chunks[id];
//
//     if (chunk == nullptr) {
//         return nullptr;
//     }
// }

} // namespace manager
