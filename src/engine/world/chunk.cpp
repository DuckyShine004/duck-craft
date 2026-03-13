#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>

#include <fmt/format.h>

#include <queue>

#include "engine/world/world.hpp"
#include "engine/world/chunk.hpp"
#include "engine/world/block.hpp"
#include "engine/world/light.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::shader;

using namespace engine::entity;

namespace engine::world {

Chunk::Chunk(int global_x, int global_y, int global_z) : global_x(global_x), global_y(global_y), global_z(global_z), _aabb(global_x, global_y, global_z, config::CHUNK_SIZE, config::CHUNK_SIZE, config::CHUNK_SIZE) {
    this->local_x = global_x >> config::CHUNK_SIZE_BITS;
    this->local_y = global_y >> config::CHUNK_SIZE_BITS;
    this->local_z = global_z >> config::CHUNK_SIZE_BITS;

    std::fill(std::begin(this->neighbours), std::end(this->neighbours), nullptr);

    std::memset(this->_blocks, 0U, sizeof(this->_blocks));
    std::memset(this->_lights, 0U, sizeof(this->_lights));

    this->_state = 0U;

    this->_queued_tasks = 0U;
    this->_running_tasks = 0U;
}

void Chunk::generate(World &world) {
    HeightMap *height_map = world.find_height_map(this->local_x, this->local_z);

    if (height_map == nullptr) {
        LOG_WARN("Chunk at position ({},{},{}) did not generate, because height map does not exist", this->local_x, this->local_y, this->local_z);

        this->clear_running_task(ChunkTask::TERRAIN_GENERATION);

        return;
    }

    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                std::uint16_t &block = this->get_block(x, y, z);

                int height = height_map->get_height(x, z);

                int dx = this->global_x + x;
                int dy = this->global_y + y;
                int dz = this->global_z + z;

                if (world.generator->is_cave(dx, dy, dz)) {
                    continue;
                }

                if (dy == height) {
                    Block::set_type(block, BlockType::GRASS);
                } else if (dy < height && dy >= height - 3) {
                    Block::set_type(block, BlockType::DIRT);
                } else if (dy < height - 3) {
                    Block::set_type(block, BlockType::STONE);
                }
            }
        }
    }

    this->set_state(ChunkState::TERRAIN_GENERATED);

    this->clear_running_task(ChunkTask::TERRAIN_GENERATION);

    this->queue_tasks(ChunkTask::LIGHT_PROPAGATION);
}

/* BUG: Lighting bug, not sure where... may need to revise SOA blog again... */
void Chunk::propagate_sunlight(World &world) {
    HeightMap *height_map = world.find_height_map(this->local_x, this->local_z);

    if (height_map == nullptr) {
        LOG_WARN("Chunk at position ({},{},{}) did not generate, because height map does not exist", this->local_x, this->local_y, this->local_z);

        this->clear_running_task(ChunkTask::LIGHT_PROPAGATION);

        return;
    }

    Chunk *top_chunk = this->get_neighbour_chunk_local(this->local_x, this->local_y + 1, this->local_z);

    bool is_top_chunk_loaded = (top_chunk != nullptr && top_chunk->is_state_set(ChunkState::TERRAIN_GENERATED));

    /* TODO: Create separate light node struct */
    struct LightNode {
        Chunk *chunk;

        int index;

        LightNode(Chunk *chunk, int index) : chunk(chunk), index(index) {
        }
    };

    std::queue<LightNode> queue;

    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int x = 0; x < config::CHUNK_SIZE; ++x) {
            std::uint8_t sunlight = 0U;

            if (is_top_chunk_loaded) {
                std::uint8_t bottom_sunlight = Light::get_sunlight(top_chunk->get_light(x, 0, z));

                if (bottom_sunlight > 0U) {
                    int top_voxel_id = this->get_voxel_id(x, config::CHUNK_SIZE - 1, z);

                    std::uint16_t &top_block = this->_blocks[top_voxel_id];

                    std::uint8_t top_sunlight = Light::get_sunlight(this->_lights[top_voxel_id]);

                    if (Block::get_type(top_block) == BlockType::EMPTY && top_sunlight < bottom_sunlight) {
                        Light::set_sunlight(this->_lights[top_voxel_id], bottom_sunlight);

                        queue.emplace(this, top_voxel_id);
                    }
                }
            } else {
                if (this->global_y + config::CHUNK_SIZE - 1 > height_map->get_height(x, z)) {
                    sunlight = 15U;
                }
            }

            if (sunlight == 0U) {
                continue;
            }

            for (int y = config::CHUNK_SIZE - 1; y >= 0; --y) {
                int voxel_id = this->get_voxel_id(x, y, z);

                std::uint16_t &block = this->_blocks[voxel_id];

                if (Block::get_type(block) > BlockType::EMPTY) {
                    break;
                }

                std::uint16_t &light = this->_lights[voxel_id];

                Light::set_sunlight(light, sunlight);

                queue.emplace(this, voxel_id);
            }
        }
    }

    while (!queue.empty()) {
        LightNode &node = queue.front();

        int index = node.index;

        Chunk *chunk = node.chunk;

        int x = index & (config::CHUNK_SIZE - 1);
        int y = (index >> config::CHUNK_SIZE_BITS) & (config::CHUNK_SIZE - 1);
        int z = (index >> config::CHUNK_SIZE_BITS2) & (config::CHUNK_SIZE - 1);

        std::uint8_t sunlight = Light::get_sunlight(chunk->get_light(index));

        queue.pop();

        if (sunlight == 0U) {
            continue;
        }

        for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
            FaceType face_type = static_cast<FaceType>(face_type_index);

            int adjacent_block_global_x = chunk->global_x + x + Face::I_NORMALS[face_type_index][0];
            int adjacent_block_global_y = chunk->global_y + y + Face::I_NORMALS[face_type_index][1];
            int adjacent_block_global_z = chunk->global_z + z + Face::I_NORMALS[face_type_index][2];

            int adjacent_chunk_local_x = adjacent_block_global_x >> config::CHUNK_SIZE_BITS;
            int adjacent_chunk_local_y = adjacent_block_global_y >> config::CHUNK_SIZE_BITS;
            int adjacent_chunk_local_z = adjacent_block_global_z >> config::CHUNK_SIZE_BITS;

            Chunk *adjacent_chunk = chunk->get_neighbour_chunk_local(adjacent_chunk_local_x, adjacent_chunk_local_y, adjacent_chunk_local_z);

            if (adjacent_chunk == nullptr || !adjacent_chunk->is_state_set(ChunkState::TERRAIN_GENERATED)) {
                continue;
            }

            int adjacent_block_local_x = adjacent_block_global_x & (config::CHUNK_SIZE - 1);
            int adjacent_block_local_y = adjacent_block_global_y & (config::CHUNK_SIZE - 1);
            int adjacent_block_local_z = adjacent_block_global_z & (config::CHUNK_SIZE - 1);

            int adjacent_voxel_id = adjacent_chunk->get_voxel_id(adjacent_block_local_x, adjacent_block_local_y, adjacent_block_local_z);

            std::uint16_t &adjacent_block = adjacent_chunk->get_block(adjacent_voxel_id);
            std::uint16_t &adjacent_light = adjacent_chunk->get_light(adjacent_voxel_id);

            if (Block::get_type(adjacent_block) > BlockType::EMPTY) {
                continue;
            }

            std::uint8_t adjacent_sunlight = sunlight - 1U;

            if (face_type == FaceType::BOTTOM && sunlight == 15U) {
                adjacent_sunlight = sunlight;
            }

            if (adjacent_sunlight <= 0U) {
                continue;
            }

            if (Light::get_sunlight(adjacent_light) >= adjacent_sunlight) {
                continue;
            }

            Light::set_sunlight(adjacent_light, adjacent_sunlight);

            queue.emplace(adjacent_chunk, adjacent_voxel_id);
        }
    }

    this->clear_running_task(ChunkTask::LIGHT_PROPAGATION);

    this->queue_tasks(ChunkTask::MESH_GENERATION);
}

void Chunk::generate_mesh() {
    this->clear_mesh();

    for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
        FaceType face_type = static_cast<FaceType>(face_type_index);

        int number_of_blocks = static_cast<int>(BlockType::COUNT);

        for (int block_type_index = 1; block_type_index < number_of_blocks; ++block_type_index) {
            BlockType block_type = static_cast<BlockType>(block_type_index);

            int texture_id = (block_type_index - 1) * 6 + face_type_index;

            this->merge_faces(block_type, face_type, texture_id);
        }
    }

    int index_offset = 0;

    for (Face &face : this->_faces) {
        face.add_to_mesh(this->_mesh, index_offset);

        index_offset += 4;
    }

    this->clear_running_task(ChunkTask::MESH_GENERATION);

    this->queue_tasks(ChunkTask::MESH_UPLOAD);
}

void Chunk::upload_mesh() {
    this->_mesh.upload();

    this->clear_mesh();

    this->clear_running_task(ChunkTask::MESH_UPLOAD);

    this->set_state(ChunkState::RENDERING);
}

void Chunk::merge_faces(BlockType &block_type, FaceType &face_type, int texture_id) {
    if (face_type == FaceType::FRONT || face_type == FaceType::BACK) {
        this->merge_XY_faces(block_type, face_type, texture_id);
    } else if (face_type == FaceType::TOP || face_type == FaceType::BOTTOM) {
        this->merge_XZ_faces(block_type, face_type, texture_id);
    } else {
        this->merge_YZ_faces(block_type, face_type, texture_id);
    }
}

void Chunk::merge_XY_faces(BlockType &block_type, FaceType &face_type, int texture_id) {
    int face_type_index = static_cast<int>(face_type);

    int nx = Face::I_NORMALS[face_type_index][0];
    int ny = Face::I_NORMALS[face_type_index][1];
    int nz = Face::I_NORMALS[face_type_index][2];

    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        std::uint32_t block_masks[config::CHUNK_SIZE];

        std::uint8_t ambient_occlusion_masks[config::CHUNK_SIZE][config::CHUNK_SIZE];

        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            block_masks[y] = 0U;

            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                std::uint16_t &block = this->get_block(x, y, z);

                ambient_occlusion_masks[y][x] = 0U;

                if (Block::get_type(block) != block_type) {
                    continue;
                }

                std::uint16_t *adjacent_block = this->get_neighbour_block(this->global_x + x + nx, this->global_y + y + ny, this->global_z + z + nz);

                if (adjacent_block != nullptr && Block::get_type(*adjacent_block) != BlockType::EMPTY) {
                    continue;
                }

                block_masks[y] |= (1U << x);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    ambient_occlusion_masks[y][x] |= (this->get_ambient_occlusion(face_type_index, vertex_index, x, y, z) & 0b11) << (vertex_index << 1);
                }
            }
        }

        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            while (block_masks[y]) {
                int x = std::countr_zero(block_masks[y]);

                std::uint8_t ambient_occlusion_mask = ambient_occlusion_masks[y][x];

                std::uint8_t sunlight = this->get_neighbour_sunlight(this->global_x + x + nx, this->global_y + y + ny, this->global_z + z + nz);

                int width = 1;

                while (x + width < config::CHUNK_SIZE) {
                    if (ambient_occlusion_masks[y][x + width] != ambient_occlusion_mask) {
                        break;
                    }

                    int adjacent_block_x = this->global_x + x + width + nx;
                    int adjacent_block_y = this->global_y + y + ny;
                    int adjacent_block_z = this->global_z + z + nz;

                    std::uint8_t adjacent_sunlight = this->get_neighbour_sunlight(adjacent_block_x, adjacent_block_y, adjacent_block_z);

                    if (adjacent_sunlight != sunlight) {
                        break;
                    }

                    if (!((block_masks[y] >> (x + width)) & 1U)) {
                        break;
                    }

                    ++width;
                }

                std::uint32_t occupancy_mask = (width < config::CHUNK_SIZE) ? (((1U << width) - 1) << x) : config::MASK32;

                int height = 0;

                while (y + height < config::CHUNK_SIZE) {
                    if ((block_masks[y + height] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    bool can_merge = true;

                    for (int dx = x; dx < x + width; ++dx) {
                        if (ambient_occlusion_masks[y + height][dx] != ambient_occlusion_mask) {
                            can_merge = false;

                            break;
                        }

                        int adjacent_block_x = this->global_x + dx + nx;
                        int adjacent_block_y = this->global_y + y + height + ny;
                        int adjacent_block_z = this->global_z + z + nz;

                        std::uint8_t adjacent_sunlight = this->get_neighbour_sunlight(adjacent_block_x, adjacent_block_y, adjacent_block_z);

                        if (adjacent_sunlight != sunlight) {
                            can_merge = false;

                            break;
                        }
                    }

                    if (!can_merge) {
                        break;
                    }

                    block_masks[y + height] &= ~occupancy_mask;

                    ++height;
                }

                int block_x = this->global_x + x;
                int block_y = this->global_y + y;
                int block_z = this->global_z + z;

                Face face(block_type, face_type, block_x, block_y, block_z, width, height, 0, texture_id);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    face.set_ambient_occlusion_state(vertex_index, ambient_occlusion_mask);

                    face.vertices[vertex_index].sunlight = sunlight;
                }

                this->_faces.emplace_back(std::move(face));
            }
        }
    }
}

void Chunk::merge_XZ_faces(BlockType &block_type, FaceType &face_type, int texture_id) {
    int face_type_index = static_cast<int>(face_type);

    int nx = Face::I_NORMALS[face_type_index][0];
    int ny = Face::I_NORMALS[face_type_index][1];
    int nz = Face::I_NORMALS[face_type_index][2];

    for (int y = 0; y < config::CHUNK_SIZE; ++y) {
        std::uint32_t block_masks[config::CHUNK_SIZE];

        std::uint8_t ambient_occlusion_masks[config::CHUNK_SIZE][config::CHUNK_SIZE];

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            block_masks[z] = 0U;

            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                std::uint16_t &block = this->get_block(x, y, z);

                ambient_occlusion_masks[z][x] = 0U;

                if (Block::get_type(block) != block_type) {
                    continue;
                }

                std::uint16_t *adjacent_block = this->get_neighbour_block(this->global_x + x + nx, this->global_y + y + ny, this->global_z + z + nz);

                if (adjacent_block != nullptr && Block::get_type(*adjacent_block) != BlockType::EMPTY) {
                    continue;
                }

                block_masks[z] |= (1U << x);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    ambient_occlusion_masks[z][x] |= (this->get_ambient_occlusion(face_type_index, vertex_index, x, y, z) & 0b11) << (vertex_index << 1);
                }
            }
        }

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            while (block_masks[z]) {
                int x = std::countr_zero(block_masks[z]);

                std::uint8_t ambient_occlusion_mask = ambient_occlusion_masks[z][x];

                std::uint8_t sunlight = this->get_neighbour_sunlight(this->global_x + x + nx, this->global_y + y + ny, this->global_z + z + nz);

                int width = 1;

                while (x + width < config::CHUNK_SIZE) {
                    if (ambient_occlusion_masks[z][x + width] != ambient_occlusion_mask) {
                        break;
                    }

                    int adjacent_block_x = this->global_x + x + width + nx;
                    int adjacent_block_y = this->global_y + y + ny;
                    int adjacent_block_z = this->global_z + z + nz;

                    std::uint8_t adjacent_sunlight = this->get_neighbour_sunlight(adjacent_block_x, adjacent_block_y, adjacent_block_z);

                    if (adjacent_sunlight != sunlight) {
                        break;
                    }

                    if (!((block_masks[z] >> (x + width)) & 1U)) {
                        break;
                    }

                    ++width;
                }

                std::uint32_t occupancy_mask = (width < config::CHUNK_SIZE) ? (((1U << width) - 1) << x) : config::MASK32;

                int depth = 0;

                while (z + depth < config::CHUNK_SIZE) {
                    if ((block_masks[z + depth] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    bool can_merge = true;

                    for (int dx = x; dx < x + width; ++dx) {
                        if (ambient_occlusion_masks[z + depth][dx] != ambient_occlusion_mask) {
                            can_merge = false;

                            break;
                        }

                        int adjacent_block_x = this->global_x + dx + nx;
                        int adjacent_block_y = this->global_y + y + ny;
                        int adjacent_block_z = this->global_z + z + depth + nz;

                        std::uint8_t adjacent_sunlight = this->get_neighbour_sunlight(adjacent_block_x, adjacent_block_y, adjacent_block_z);

                        if (adjacent_sunlight != sunlight) {
                            can_merge = false;

                            break;
                        }
                    }

                    if (!can_merge) {
                        break;
                    }

                    block_masks[z + depth] &= ~occupancy_mask;

                    ++depth;
                }

                int block_x = this->global_x + x;
                int block_y = this->global_y + y;
                int block_z = this->global_z + z;

                Face face(block_type, face_type, block_x, block_y, block_z, width, 0, depth, texture_id);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    face.set_ambient_occlusion_state(vertex_index, ambient_occlusion_mask);

                    face.vertices[vertex_index].sunlight = sunlight;
                }

                this->_faces.emplace_back(std::move(face));
            }
        }
    }
}

void Chunk::merge_YZ_faces(BlockType &block_type, FaceType &face_type, int texture_id) {
    int face_type_index = static_cast<int>(face_type);

    int nx = Face::I_NORMALS[face_type_index][0];
    int ny = Face::I_NORMALS[face_type_index][1];
    int nz = Face::I_NORMALS[face_type_index][2];

    for (int x = 0; x < config::CHUNK_SIZE; ++x) {
        std::uint32_t block_masks[config::CHUNK_SIZE];

        std::uint8_t ambient_occlusion_masks[config::CHUNK_SIZE][config::CHUNK_SIZE];

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            block_masks[z] = 0U;

            for (int y = 0; y < config::CHUNK_SIZE; ++y) {
                std::uint16_t &block = this->get_block(x, y, z);

                ambient_occlusion_masks[z][y] = 0U;

                if (Block::get_type(block) != block_type) {
                    continue;
                }

                std::uint16_t *adjacent_block = this->get_neighbour_block(this->global_x + x + nx, this->global_y + y + ny, this->global_z + z + nz);

                if (adjacent_block != nullptr && Block::get_type(*adjacent_block) != BlockType::EMPTY) {
                    continue;
                }

                block_masks[z] |= (1U << y);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    ambient_occlusion_masks[z][y] |= (this->get_ambient_occlusion(face_type_index, vertex_index, x, y, z) & 0b11) << (vertex_index << 1);
                }
            }
        }

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            while (block_masks[z]) {
                int y = std::countr_zero(block_masks[z]);

                std::uint8_t ambient_occlusion_mask = ambient_occlusion_masks[z][y];

                std::uint8_t sunlight = this->get_neighbour_sunlight(this->global_x + x + nx, this->global_y + y + ny, this->global_z + z + nz);

                int height = 1;

                while (y + height < config::CHUNK_SIZE) {
                    if (ambient_occlusion_masks[z][y + height] != ambient_occlusion_mask) {
                        break;
                    }

                    int adjacent_block_x = this->global_x + x + nx;
                    int adjacent_block_y = this->global_y + y + height + ny;
                    int adjacent_block_z = this->global_z + z + nz;

                    std::uint8_t adjacent_sunlight = this->get_neighbour_sunlight(adjacent_block_x, adjacent_block_y, adjacent_block_z);

                    if (adjacent_sunlight != sunlight) {
                        break;
                    }

                    if (!((block_masks[z] >> (y + height)) & 1U)) {
                        break;
                    }

                    ++height;
                }

                std::uint32_t occupancy_mask = (height < config::CHUNK_SIZE) ? (((1U << height) - 1) << y) : config::MASK32;

                int depth = 0;

                while (z + depth < config::CHUNK_SIZE) {
                    if ((block_masks[z + depth] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    bool can_merge = true;

                    for (int dy = y; dy < y + height; ++dy) {
                        if (ambient_occlusion_masks[z + depth][dy] != ambient_occlusion_mask) {
                            can_merge = false;

                            break;
                        }

                        int adjacent_block_x = this->global_x + x + nx;
                        int adjacent_block_y = this->global_y + dy + ny;
                        int adjacent_block_z = this->global_z + z + depth + nz;

                        std::uint8_t adjacent_sunlight = this->get_neighbour_sunlight(adjacent_block_x, adjacent_block_y, adjacent_block_z);

                        if (adjacent_sunlight != sunlight) {
                            can_merge = false;

                            break;
                        }
                    }

                    if (!can_merge) {
                        break;
                    }

                    block_masks[z + depth] &= ~occupancy_mask;

                    ++depth;
                }

                int block_x = this->global_x + x;
                int block_y = this->global_y + y;
                int block_z = this->global_z + z;

                Face face(block_type, face_type, block_x, block_y, block_z, 0, height, depth, texture_id);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    face.set_ambient_occlusion_state(vertex_index, ambient_occlusion_mask);

                    face.vertices[vertex_index].sunlight = sunlight;
                }

                this->_faces.emplace_back(std::move(face));
            }
        }
    }
}

void Chunk::clear_mesh() {
    this->_mesh.clear();

    this->_faces.clear();
}

int Chunk::get_ambient_occlusion(int face_type_index, int vertex_index, int x, int y, int z) {
    int side_values[3];

    for (int side_index = 0; side_index < 3; ++side_index) {
        int side_block_x = this->global_x + x + this->_BLOCK_OFFSETS[face_type_index][vertex_index][side_index][0];
        int side_block_y = this->global_y + y + this->_BLOCK_OFFSETS[face_type_index][vertex_index][side_index][1];
        int side_block_z = this->global_z + z + this->_BLOCK_OFFSETS[face_type_index][vertex_index][side_index][2];

        std::uint16_t *side_block = this->get_neighbour_block(side_block_x, side_block_y, side_block_z);

        if (side_block == nullptr) {
            side_values[side_index] = 0;

            continue;
        }

        side_values[side_index] = (Block::get_type(*side_block) == BlockType::EMPTY) ? 0 : 1;
    }

    if (side_values[0] && side_values[1]) {
        return 0;
    }

    return 3 - (side_values[0] + side_values[1] + side_values[2]);
}

void Chunk::render(Shader &shader) {
    glm::mat4 model(1.0f);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", utility::ColourUtility::get_high_precision_RGB(common::WHITE_RGB));

    this->_mesh.render(this->_TOPOLOGY);
}

int Chunk::get_voxel_id(int x, int y, int z) {
    return x + (y << config::CHUNK_SIZE_BITS) + (z << config::CHUNK_SIZE_BITS2);
}

std::uint16_t &Chunk::get_block(int x, int y, int z) {
    int id = this->get_voxel_id(x, y, z);

    return this->_blocks[id];
}

std::uint16_t &Chunk::get_block(int index) {
    return this->_blocks[index];
}

AABB &Chunk::get_aabb() {
    return this->_aabb;
}

std::uint16_t *Chunk::get_neighbour_block(int global_x, int global_y, int global_z) {
    int local_x = global_x >> config::CHUNK_SIZE_BITS;
    int local_y = global_y >> config::CHUNK_SIZE_BITS;
    int local_z = global_z >> config::CHUNK_SIZE_BITS;

    int offset_x = local_x - this->local_x + 1;
    int offset_y = local_y - this->local_y + 1;
    int offset_z = local_z - this->local_z + 1;

    int block_x = global_x & (config::CHUNK_SIZE - 1);
    int block_y = global_y & (config::CHUNK_SIZE - 1);
    int block_z = global_z & (config::CHUNK_SIZE - 1);

    if (offset_x == 1 && offset_y == 1 && offset_z == 1) {
        return &this->get_block(block_x, block_y, block_z);
    }

    if (offset_x < 0 || offset_x > 2 || offset_y < 0 || offset_y > 2 || offset_z < 0 || offset_z > 2) {
        LOG_WARN("Chunk neighbour at position ({},{},{}) not found", local_x, local_y, local_z);

        return nullptr;
    }

    int neighbour_id = offset_x + offset_y * 3 + offset_z * 9;

    Chunk *neighbour = this->neighbours[neighbour_id];

    if (neighbour == nullptr) {
        return nullptr;
    }

    return &neighbour->get_block(block_x, block_y, block_z);
}

Chunk *Chunk::get_neighbour_chunk_local(int local_x, int local_y, int local_z) {
    int offset_x = local_x - this->local_x + 1;
    int offset_y = local_y - this->local_y + 1;
    int offset_z = local_z - this->local_z + 1;

    if (offset_x == 1 && offset_y == 1 && offset_z == 1) {
        return this;
    }

    if (offset_x < 0 || offset_x > 2 || offset_y < 0 || offset_y > 2 || offset_z < 0 || offset_z > 2) {
        LOG_WARN("Chunk neighbour at position ({},{},{}) not found", local_x, local_y, local_z);

        return nullptr;
    }

    int neighbour_id = offset_x + offset_y * 3 + offset_z * 9;

    return this->neighbours[neighbour_id];
}

std::uint8_t Chunk::get_neighbour_sunlight(int global_x, int global_y, int global_z) {
    int local_x = global_x >> config::CHUNK_SIZE_BITS;
    int local_y = global_y >> config::CHUNK_SIZE_BITS;
    int local_z = global_z >> config::CHUNK_SIZE_BITS;

    int offset_x = local_x - this->local_x + 1;
    int offset_y = local_y - this->local_y + 1;
    int offset_z = local_z - this->local_z + 1;

    int light_x = global_x & (config::CHUNK_SIZE - 1);
    int light_y = global_y & (config::CHUNK_SIZE - 1);
    int light_z = global_z & (config::CHUNK_SIZE - 1);

    if (offset_x == 1 && offset_y == 1 && offset_z == 1) {
        return Light::get_sunlight(this->get_light(light_x, light_y, light_z));
    }

    if (offset_x < 0 || offset_x > 2 || offset_y < 0 || offset_y > 2 || offset_z < 0 || offset_z > 2) {
        LOG_WARN("Chunk neighbour at position ({},{},{}) not found", local_x, local_y, local_z);

        return 0U;
    }

    int neighbour_id = offset_x + offset_y * 3 + offset_z * 9;

    Chunk *neighbour = this->neighbours[neighbour_id];

    if (neighbour == nullptr) {
        return 0U;
    }

    return Light::get_sunlight(neighbour->get_light(light_x, light_y, light_z));
}

std::uint16_t &Chunk::get_light(int x, int y, int z) {
    int light_id = x + (y << config::CHUNK_SIZE_BITS) + (z << config::CHUNK_SIZE_BITS2);

    return this->_lights[light_id];
}

std::uint16_t &Chunk::get_light(int index) {
    return this->_lights[index];
}

bool Chunk::is_state_set(const ChunkState &state) {
    return this->_state.load(std::memory_order_acquire) & static_cast<std::uint8_t>(state);
}

void Chunk::set_state(const ChunkState &state) {
    this->_state.fetch_or(static_cast<std::uint8_t>(state), std::memory_order_acq_rel);
}

void Chunk::clear_state(const ChunkState &state) {
    this->_state.fetch_and(~static_cast<std::uint8_t>(state), std::memory_order_acq_rel);
}

bool Chunk::is_task_running(const ChunkTask &task) {
    return this->_running_tasks.load(std::memory_order_acquire) & static_cast<std::uint8_t>(task);
}

bool Chunk::can_run_task(const ChunkTask &task) {
    return this->is_task_queued(task) && !this->is_task_running(task);
}

void Chunk::set_running_task(const ChunkTask &task) {
    this->clear_queued_task(task);

    this->_running_tasks.fetch_or(static_cast<std::uint8_t>(task), std::memory_order_acq_rel);
}

void Chunk::clear_running_task(const ChunkTask &task) {
    this->_running_tasks.fetch_and(~static_cast<std::uint8_t>(task), std::memory_order_acq_rel);
}

bool Chunk::is_task_queued(const ChunkTask &task) {
    return this->_queued_tasks.load(std::memory_order_acquire) & static_cast<std::uint8_t>(task);
}

bool Chunk::is_task_queue_empty() {
    return this->_queued_tasks.load(std::memory_order_acquire) == 0U;
}

void Chunk::clear_queued_task(const ChunkTask &task) {
    this->_queued_tasks.fetch_and(~static_cast<std::uint8_t>(task), std::memory_order_acq_rel);
}

} // namespace engine::world
