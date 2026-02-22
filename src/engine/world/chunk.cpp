#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>

#include <queue>

#include "engine/world/chunk.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::shader;

using namespace engine::math::hash::vector;

namespace engine::world {

Chunk::Chunk(int global_x, int global_y, int global_z) : global_x(global_x), global_y(global_y), global_z(global_z) {
    this->local_x = global_x >> config::CHUNK_SIZE_BITS;
    this->local_y = global_y >> config::CHUNK_SIZE_BITS;
    this->local_z = global_z >> config::CHUNK_SIZE_BITS;

    this->set_state(ChunkState::EMPTY);

    this->_dirty_borders_mask = 0U;

    this->set_is_dirty_border_task_running(false);

    this->set_is_terrain_generation_complete(false);
}

void Chunk::generate(Generator &generator, HeightMap &height_map) {
    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                Block &block = this->get_block(x, y, z);

                int height = height_map.get_height(x, z);

                int dx = this->global_x + x;
                int dy = this->global_y + y;
                int dz = this->global_z + z;

                float threshold = generator.get_cave_noise(dx, dy, dz);

                // LOG_INFO("Threshold: {}", threshold);

                if (threshold <= 0.1f || threshold >= 0.9f) {
                    continue;
                }

                // Generate grass block
                if (dy == height) {
                    block.set_type(BlockType::GRASS);
                } else if (dy < height && dy >= height - 3) {
                    block.set_type(BlockType::DIRT);
                } else if (dy < height - 3) {
                    block.set_type(BlockType::STONE);
                }
            }
        }
    }
}

// NOTE: Since chunks is being accessed concurrently during remesh, must also be part of remesh. Must also be calculated straight after generation, or as the first step of remesh
void Chunk::propagate_sunlight(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks) {
    glm::ivec3 top_chunk_position(this->local_x, this->local_y + 1, this->local_z);

    // Check if top chunk is loaded
    Chunk *top_chunk = nullptr;

    chunks.visit(top_chunk_position, [&](const auto &chunk_iterator) {
        top_chunk = chunk_iterator.second.get();
    });

    struct SunLightNode {
        // Block index
        int index;
        int light;

        SunLightNode(int index, int light) : index(index), light(light) {
        }
    };

    std::queue<SunLightNode> queue;

    // Top chunk is not generated or loaded yet
    if (top_chunk == nullptr || !top_chunk->is_terrain_generation_complete()) {
        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                int id = this->get_block_id(x, config::CHUNK_SIZE, z);

                Block &block = this->_blocks[id];

                if (block.get_type() > BlockType::EMPTY) {
                    continue;
                }

                queue.emplace(id, 15);
            }
        }
    } else {
    }
}

void Chunk::generate_mesh(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks) {
    this->clear_mesh();

    for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
        FaceType face_type = static_cast<FaceType>(face_type_index);

        int number_of_blocks = static_cast<int>(BlockType::COUNT);

        for (int block_type_index = 0; block_type_index < number_of_blocks; ++block_type_index) {
            BlockType block_type = static_cast<BlockType>(block_type_index);

            int texture_id = block_type_index * 6 + face_type_index;

            this->merge_faces(chunks, block_type, face_type, texture_id);
        }
    }

    int index_offset = 0;

    for (Face &face : this->_faces) {
        face.add_to_mesh(this->_mesh);

        face.add_indices(this->_mesh, index_offset);

        index_offset += 4;
    }

    int total_faces = this->_faces.size();

    // LOG_INFO("Faces: {}, Vertices: {}", total_faces, total_faces << 2);
}

void Chunk::upload_mesh() {
    this->_mesh.upload();

    this->set_state(ChunkState::RENDERING);
}

void Chunk::merge_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks, BlockType &block_type, FaceType &face_type, int texture_id) {
    if (face_type == FaceType::FRONT || face_type == FaceType::BACK) {
        this->merge_XY_faces(chunks, block_type, face_type, texture_id);
    } else if (face_type == FaceType::TOP || face_type == FaceType::BOTTOM) {
        this->merge_XZ_faces(chunks, block_type, face_type, texture_id);
    } else {
        this->merge_YZ_faces(chunks, block_type, face_type, texture_id);
    }
}

void Chunk::merge_XY_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks, BlockType &block_type, FaceType &face_type, int texture_id) {
    int face_type_index = static_cast<int>(face_type);

    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        std::uint32_t block_masks[config::CHUNK_SIZE];

        std::uint8_t ambient_occlusion_masks[config::CHUNK_SIZE][config::CHUNK_SIZE];

        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            block_masks[y] = 0U;

            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                Block &block = this->get_block(x, y, z);

                ambient_occlusion_masks[y][x] = 0U;

                if (block.get_type() != block_type || !block.is_face_active(face_type)) {
                    continue;
                }

                block_masks[y] |= (1U << x);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    ambient_occlusion_masks[y][x] |= (this->get_ambient_occlusion(chunks, face_type_index, vertex_index, x, y, z) & 0b11) << (vertex_index << 1);
                }
            }
        }

        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            while (block_masks[y]) {
                int x = std::countr_zero(block_masks[y]);

                std::uint8_t ambient_occlusion_mask = ambient_occlusion_masks[y][x];

                int width = 1;

                while (x + width < config::CHUNK_SIZE) {
                    if (ambient_occlusion_masks[y][x + width] != ambient_occlusion_mask) {
                        break;
                    }

                    if (!((block_masks[y] >> (x + width)) & 1U)) {
                        break;
                    }

                    ++width;
                }

                std::uint32_t occupancy_mask = (width < config::CHUNK_SIZE) ? (((1U << width) - 1) << x) : this->_FULL_MASK;

                int height = 0;

                while (y + height < config::CHUNK_SIZE) {
                    if ((block_masks[y + height] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    bool is_ambient_occlusion_valid = true;

                    for (int dx = x; dx < x + width; ++dx) {
                        if (ambient_occlusion_masks[y + height][dx] != ambient_occlusion_mask) {
                            is_ambient_occlusion_valid = false;

                            break;
                        }
                    }

                    if (!is_ambient_occlusion_valid) {
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
                }

                this->_faces.emplace_back(std::move(face));
            }
        }
    }
}

void Chunk::merge_XZ_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks, BlockType &block_type, FaceType &face_type, int texture_id) {
    int face_type_index = static_cast<int>(face_type);

    for (int y = 0; y < config::CHUNK_SIZE; ++y) {
        std::uint32_t block_masks[config::CHUNK_SIZE];

        std::uint8_t ambient_occlusion_masks[config::CHUNK_SIZE][config::CHUNK_SIZE];

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            block_masks[z] = 0U;

            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                Block &block = this->get_block(x, y, z);

                ambient_occlusion_masks[z][x] = 0U;

                if (block.get_type() != block_type || !block.is_face_active(face_type)) {
                    continue;
                }

                block_masks[z] |= (1U << x);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    ambient_occlusion_masks[z][x] |= (this->get_ambient_occlusion(chunks, face_type_index, vertex_index, x, y, z) & 0b11) << (vertex_index << 1);
                }
            }
        }

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            while (block_masks[z]) {
                int x = std::countr_zero(block_masks[z]);

                std::uint8_t ambient_occlusion_mask = ambient_occlusion_masks[z][x];

                int width = 1;

                while (x + width < config::CHUNK_SIZE) {
                    if (ambient_occlusion_masks[z][x + width] != ambient_occlusion_mask) {
                        break;
                    }

                    if (!((block_masks[z] >> (x + width)) & 1U)) {
                        break;
                    }

                    ++width;
                }

                std::uint32_t occupancy_mask = (width < config::CHUNK_SIZE) ? (((1U << width) - 1) << x) : this->_FULL_MASK;

                int depth = 0;

                while (z + depth < config::CHUNK_SIZE) {
                    if ((block_masks[z + depth] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    bool is_ambient_occlusion_valid = true;

                    for (int dx = x; dx < x + width; ++dx) {
                        if (ambient_occlusion_masks[z + depth][dx] != ambient_occlusion_mask) {
                            is_ambient_occlusion_valid = false;

                            break;
                        }
                    }

                    if (!is_ambient_occlusion_valid) {
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
                }

                this->_faces.emplace_back(std::move(face));
            }
        }
    }
}

void Chunk::merge_YZ_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks, BlockType &block_type, FaceType &face_type, int texture_id) {
    int face_type_index = static_cast<int>(face_type);

    for (int x = 0; x < config::CHUNK_SIZE; ++x) {
        std::uint32_t block_masks[config::CHUNK_SIZE];

        std::uint8_t ambient_occlusion_masks[config::CHUNK_SIZE][config::CHUNK_SIZE];

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            block_masks[z] = 0U;

            for (int y = 0; y < config::CHUNK_SIZE; ++y) {
                Block &block = this->get_block(x, y, z);

                ambient_occlusion_masks[z][y] = 0U;

                if (block.get_type() != block_type || !block.is_face_active(face_type)) {
                    continue;
                }

                block_masks[z] |= (1U << y);

                for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
                    ambient_occlusion_masks[z][y] |= (this->get_ambient_occlusion(chunks, face_type_index, vertex_index, x, y, z) & 0b11) << (vertex_index << 1);
                }
            }
        }

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            while (block_masks[z]) {
                int y = std::countr_zero(block_masks[z]);

                std::uint8_t ambient_occlusion_mask = ambient_occlusion_masks[z][y];

                int height = 1;

                while (y + height < config::CHUNK_SIZE) {
                    if (ambient_occlusion_masks[z][y + height] != ambient_occlusion_mask) {
                        break;
                    }

                    if (!((block_masks[z] >> (y + height)) & 1U)) {
                        break;
                    }

                    ++height;
                }

                std::uint32_t occupancy_mask = (height < config::CHUNK_SIZE) ? (((1U << height) - 1) << y) : this->_FULL_MASK;

                int depth = 0;

                while (z + depth < config::CHUNK_SIZE) {
                    if ((block_masks[z + depth] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    bool is_ambient_occlusion_valid = true;

                    for (int dy = y; dy < y + height; ++dy) {
                        if (ambient_occlusion_masks[z + depth][dy] != ambient_occlusion_mask) {
                            is_ambient_occlusion_valid = false;

                            break;
                        }
                    }

                    if (!is_ambient_occlusion_valid) {
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
                }

                this->_faces.emplace_back(std::move(face));
            }
        }
    }
}

void Chunk::occlude_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks) {
    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                Block &block = this->get_block(x, y, z);

                for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
                    int nx = Face::I_NORMALS[face_type_index][0];
                    int ny = Face::I_NORMALS[face_type_index][1];
                    int nz = Face::I_NORMALS[face_type_index][2];

                    int dx = x + nx;
                    int dy = y + ny;
                    int dz = z + nz;

                    int opposite_face_type_index = (face_type_index & 1) ? face_type_index - 1 : face_type_index + 1;

                    if (dx < 0 || dx >= config::CHUNK_SIZE || dy < 0 || dy >= config::CHUNK_SIZE || dz < 0 || dz >= config::CHUNK_SIZE) {
                        continue;
                    }

                    Block &adjacent_block = this->get_block(dx, dy, dz);

                    this->cull_face_based_on_adjacent_block(block, adjacent_block, face_type_index);
                }
            }
        }
    }

    this->occlude_border_faces(chunks);
}

void Chunk::occlude_border_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks) {
    for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
        int nx = Face::I_NORMALS[face_type_index][0];
        int ny = Face::I_NORMALS[face_type_index][1];
        int nz = Face::I_NORMALS[face_type_index][2];

        glm::ivec3 adjacent_chunk_position(this->local_x + nx, this->local_y + ny, this->local_z + nz);

        Chunk *adjacent_chunk = nullptr;

        chunks.visit(adjacent_chunk_position, [&](const auto &chunk_iterator) {
            adjacent_chunk = chunk_iterator.second.get();
        });

        if (adjacent_chunk == nullptr) {
            continue;
        }

        if (!adjacent_chunk->is_terrain_generation_complete()) {
            this->set_dirty_border_state(face_type_index, true);

            continue;
        }

        FaceType face_type = static_cast<FaceType>(face_type_index);

        if (face_type == FaceType::FRONT || face_type == FaceType::BACK) {
            this->occlude_XY_faces(*adjacent_chunk, face_type);
        } else if (face_type == FaceType::TOP || face_type == FaceType::BOTTOM) {
            this->occlude_XZ_faces(*adjacent_chunk, face_type);
        } else {
            this->occlude_YZ_faces(*adjacent_chunk, face_type);
        }
    }
}

void Chunk::occlude_dirty_borders(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks) {
    uint8_t mask = this->get_dirty_borders_mask_and_reset();

    uint8_t dirty_borders_mask = 0U;

    while (mask) {
        int face_type_index = std::countr_zero(mask);

        mask &= (mask - 1);

        int nx = Face::I_NORMALS[face_type_index][0];
        int ny = Face::I_NORMALS[face_type_index][1];
        int nz = Face::I_NORMALS[face_type_index][2];

        glm::ivec3 adjacent_chunk_id(this->local_x + nx, this->local_y + ny, this->local_z + nz);

        Chunk *adjacent_chunk = nullptr;

        chunks.visit(adjacent_chunk_id, [&](const auto &chunk_iterator) {
            adjacent_chunk = chunk_iterator.second.get();
        });

        if (adjacent_chunk == nullptr) {
            continue;
        }

        if (!adjacent_chunk->is_terrain_generation_complete()) {
            dirty_borders_mask |= (1U << face_type_index);

            continue;
        }

        FaceType face_type = static_cast<FaceType>(face_type_index);

        if (face_type == FaceType::FRONT || face_type == FaceType::BACK) {
            this->occlude_XY_faces(*adjacent_chunk, face_type);
        } else if (face_type == FaceType::TOP || face_type == FaceType::BOTTOM) {
            this->occlude_XZ_faces(*adjacent_chunk, face_type);
        } else {
            this->occlude_YZ_faces(*adjacent_chunk, face_type);
        }
    }

    if (dirty_borders_mask) {
        this->_dirty_borders_mask.fetch_or(dirty_borders_mask, std::memory_order_release);
    }
}

void Chunk::occlude_XY_faces(Chunk &adjacent_chunk, const FaceType &face_type) {
    int z = (face_type == FaceType::BACK) ? 0 : config::CHUNK_SIZE - 1;

    int dz = config::CHUNK_SIZE - z - 1;

    int face_type_index = static_cast<int>(face_type);

    for (int y = 0; y < config::CHUNK_SIZE; ++y) {
        for (int x = 0; x < config::CHUNK_SIZE; ++x) {
            Block &block = this->get_block(x, y, z);
            Block &adjacent_block = adjacent_chunk.get_block(x, y, dz);

            this->cull_face_based_on_adjacent_block(block, adjacent_block, face_type_index);
        }
    }
}

void Chunk::occlude_XZ_faces(Chunk &adjacent_chunk, const FaceType &face_type) {
    int y = (face_type == FaceType::BOTTOM) ? 0 : config::CHUNK_SIZE - 1;

    int dy = config::CHUNK_SIZE - y - 1;

    int face_type_index = static_cast<int>(face_type);

    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int x = 0; x < config::CHUNK_SIZE; ++x) {
            Block &block = this->get_block(x, y, z);
            Block &adjacent_block = adjacent_chunk.get_block(x, dy, z);

            this->cull_face_based_on_adjacent_block(block, adjacent_block, face_type_index);
        }
    }
}

void Chunk::occlude_YZ_faces(Chunk &adjacent_chunk, const FaceType &face_type) {
    int x = (face_type == FaceType::LEFT) ? 0 : config::CHUNK_SIZE - 1;

    int dx = config::CHUNK_SIZE - x - 1;

    int face_type_index = static_cast<int>(face_type);

    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            Block &block = this->get_block(x, y, z);
            Block &adjacent_block = adjacent_chunk.get_block(dx, y, z);

            this->cull_face_based_on_adjacent_block(block, adjacent_block, face_type_index);
        }
    }
}

// NOTE: Need to add to logic once transparency is added
void Chunk::cull_face_based_on_adjacent_block(Block &block, Block &adjacent_block, int face_type_index) {
    if (adjacent_block.get_type() != BlockType::EMPTY) {
        block.set_face_state(face_type_index, false);
    } else {
        block.set_face_state(face_type_index, true);
    }
}

void Chunk::clear_mesh() {
    this->_mesh.clear();

    this->_faces.clear();
}

int Chunk::get_ambient_occlusion(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks, int face_type_index, int vertex_index, int x, int y, int z) {
    int side_values[3];

    for (int side_index = 0; side_index < 3; ++side_index) {
        glm::ivec3 side_position(this->global_x + x, this->global_y + y, this->global_z + z);

        side_position.x += this->_BLOCK_OFFSETS[face_type_index][vertex_index][side_index][0];
        side_position.y += this->_BLOCK_OFFSETS[face_type_index][vertex_index][side_index][1];
        side_position.z += this->_BLOCK_OFFSETS[face_type_index][vertex_index][side_index][2];

        glm::ivec3 side_global_chunk_position(this->global_x, this->global_y, this->global_z);

        if (side_position.x < this->global_x) {
            side_global_chunk_position.x -= config::CHUNK_SIZE;
        } else if (side_position.x >= this->global_x + config::CHUNK_SIZE) {
            side_global_chunk_position.x += config::CHUNK_SIZE;
        }

        if (side_position.y < this->global_y) {
            side_global_chunk_position.y -= config::CHUNK_SIZE;
        } else if (side_position.y >= this->global_y + config::CHUNK_SIZE) {
            side_global_chunk_position.y += config::CHUNK_SIZE;
        }

        if (side_position.z < this->global_z) {
            side_global_chunk_position.z -= config::CHUNK_SIZE;
        } else if (side_position.z >= this->global_z + config::CHUNK_SIZE) {
            side_global_chunk_position.z += config::CHUNK_SIZE;
        }

        glm::ivec3 side_local_chunk_position = side_global_chunk_position >> config::CHUNK_SIZE_BITS;

        Chunk *side_chunk = nullptr;

        chunks.visit(side_local_chunk_position, [&](const auto &chunk_iterator) {
            side_chunk = chunk_iterator.second.get();
        });

        if (side_chunk == nullptr || !side_chunk->is_terrain_generation_complete()) {
            side_values[side_index] = 0;

            continue;
        }

        glm::ivec3 block_position = side_position - side_global_chunk_position;

        Block &side_block = side_chunk->get_block(block_position);

        side_values[side_index] = (side_block.get_type() == BlockType::EMPTY) ? 0 : 1;
    }

    if (side_values[0] && side_values[1]) {
        return 0;
    }

    return 3 - (side_values[0] + side_values[1] + side_values[2]);
}

void Chunk::render(Shader &shader) {
    if (this->get_state() != ChunkState::RENDERING) {
        return;
    }

    glm::mat4 model(1.0f);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", utility::ColourUtility::get_high_precision_RGB(common::WHITE_RGB));

    this->_mesh.render(this->_TOPOLOGY);
}

int Chunk::get_block_id(int x, int y, int z) {
    return x + (y << config::CHUNK_SIZE_BITS) + (z << config::CHUNK_SIZE_BITS2);
}

Block &Chunk::get_block(int x, int y, int z) {
    int id = this->get_block_id(x, y, z);

    return this->_blocks[id];
}

Block &Chunk::get_block(glm::ivec3 &position) {
    int id = this->get_block_id(position.x, position.y, position.z);

    return this->_blocks[id];
}

ChunkState Chunk::get_state() {
    return this->_state.load(std::memory_order_acquire);
}

void Chunk::set_state(const ChunkState &state) {
    this->_state.store(state, std::memory_order_release);
}

std::uint8_t Chunk::get_dirty_borders_mask_and_reset() {
    return this->_dirty_borders_mask.exchange(0U, std::memory_order_acq_rel);
}

bool Chunk::has_dirty_borders() {
    return this->_dirty_borders_mask.load(std::memory_order_acquire) > 0U;
}

void Chunk::set_dirty_border_state(int face_type_index, bool state) {
    if (state) {
        this->_dirty_borders_mask.fetch_or(1U << face_type_index, std::memory_order_release);
    } else {
        this->_dirty_borders_mask.fetch_and(~(1U << face_type_index), std::memory_order_release);
    }
}

void Chunk::set_is_terrain_generation_complete(bool is_terrain_generation_complete) {
    this->_is_terrain_generation_complete.store(is_terrain_generation_complete, std::memory_order_release);
}

void Chunk::set_is_dirty_border_task_running(bool is_dirty_border_task_running) {
    this->_is_dirty_border_task_running.store(is_dirty_border_task_running, std::memory_order_release);
}

bool Chunk::is_terrain_generation_complete() {
    return this->_is_terrain_generation_complete.load(std::memory_order_acquire);
}

bool Chunk::can_dirty_border_task_run() {
    bool can_dirty_border_task_run = false;

    // return this->_is_dirty_border_task_running.load(std::memory_order_acquire);
    return this->_is_dirty_border_task_running.compare_exchange_strong(can_dirty_border_task_run, true, std::memory_order_acq_rel);
}

} // namespace engine::world
