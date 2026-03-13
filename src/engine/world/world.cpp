#include "engine/world/world.hpp"

namespace engine::world {

World::World() : _global_chunk_id(0U) {
    this->generator = std::make_shared<Generator>();
}

HeightMap *World::try_emplace_height_map(int chunk_local_x, int chunk_local_z, int chunk_global_x, int chunk_global_z) {
    glm::ivec2 position(chunk_local_x, chunk_local_z);

    auto height_map_iterator = this->height_maps.find(position);

    if (height_map_iterator == this->height_maps.end()) {
        std::unique_ptr<HeightMap> height_map = std::make_unique<HeightMap>();

        height_map->generate(*this->generator, chunk_global_x, chunk_global_z);

        auto [iterator, is_emplaced] = this->height_maps.emplace(position, std::move(height_map));

        height_map_iterator = iterator;
    }

    return height_map_iterator->second.get();
}

std::uint32_t World::try_emplace_chunk_id(int chunk_local_x, int chunk_local_y, int chunk_local_z, int chunk_global_x, int chunk_global_y, int chunk_global_z) {
    glm::ivec3 position(chunk_local_x, chunk_local_y, chunk_local_z);

    auto chunk_id_iterator = this->chunk_ids.find(position);

    bool is_chunk_id_emplaced = false;

    if (chunk_id_iterator == this->chunk_ids.end()) {
        this->chunks.emplace_back(std::make_unique<Chunk>(chunk_global_x, chunk_global_y, chunk_global_z));

        auto [iterator, is_emplaced] = this->chunk_ids.emplace(position, this->_global_chunk_id);

        ++this->_global_chunk_id;

        chunk_id_iterator = iterator;

        is_chunk_id_emplaced = is_emplaced;
    }

    if (is_chunk_id_emplaced) {
        this->set_chunk_neighbours(chunk_local_x, chunk_local_y, chunk_local_z);
    }

    return chunk_id_iterator->second;
}

HeightMap *World::find_height_map(int chunk_local_x, int chunk_local_z) {
    glm::ivec2 position(chunk_local_x, chunk_local_z);

    auto iterator = this->height_maps.find(position);

    if (iterator == this->height_maps.end()) {
        return nullptr;
    }

    return iterator->second.get();
}

Chunk *World::find_chunk(int chunk_local_x, int chunk_local_y, int chunk_local_z) {
    glm::ivec3 position(chunk_local_x, chunk_local_y, chunk_local_z);

    auto iterator = this->chunk_ids.find(position);

    if (iterator == this->chunk_ids.end()) {
        return nullptr;
    }

    return this->chunks[iterator->second].get();
}

std::uint16_t *World::find_global_block(int global_x, int global_y, int global_z) {
    int chunk_local_x = global_x >> config::CHUNK_SIZE_BITS;
    int chunk_local_y = global_y >> config::CHUNK_SIZE_BITS;
    int chunk_local_z = global_z >> config::CHUNK_SIZE_BITS;

    Chunk *chunk = this->find_chunk(chunk_local_x, chunk_local_y, chunk_local_z);

    if (chunk == nullptr) {
        return nullptr;
    }

    int block_x = global_x & (config::CHUNK_SIZE - 1);
    int block_y = global_y & (config::CHUNK_SIZE - 1);
    int block_z = global_z & (config::CHUNK_SIZE - 1);

    return &chunk->get_block(block_x, block_y, block_z);
}

void World::set_chunk_neighbours(int chunk_local_x, int chunk_local_y, int chunk_local_z) {
    Chunk *chunk = this->find_chunk(chunk_local_x, chunk_local_y, chunk_local_z);

    if (chunk == nullptr) {
        return;
    }

    for (int dz = -1; dz <= 1; ++dz) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0 && dz == 0) {
                    continue;
                }

                int neighbour_x = chunk_local_x + dx;
                int neighbour_y = chunk_local_y + dy;
                int neighbour_z = chunk_local_z + dz;

                Chunk *neighbour_chunk = this->find_chunk(neighbour_x, neighbour_y, neighbour_z);

                if (neighbour_chunk == nullptr) {
                    continue;
                }

                int to_neighbour_x = dx + 1;
                int to_neighbour_y = dy + 1;
                int to_neighbour_z = dz + 1;

                int neighbour_id = to_neighbour_x + to_neighbour_y * 3 + to_neighbour_z * 9;

                chunk->neighbours[neighbour_id] = neighbour_chunk;

                int to_chunk_x = -dx + 1;
                int to_chunk_y = -dy + 1;
                int to_chunk_z = -dz + 1;

                int chunk_id = to_chunk_x + to_chunk_y * 3 + to_chunk_z * 9;

                neighbour_chunk->neighbours[chunk_id] = chunk;
            }
        }
    }
}

} // namespace engine::world
