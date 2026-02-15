#include <boost/core/allocator_access.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/world/chunk.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::shader;

using namespace engine::math::hash::vector;

namespace engine::world {

// TODO: Create debug struct for chunk mesh
Chunk::Chunk(int global_x, int global_y, int global_z) : global_x(global_x), global_y(global_y), global_z(global_z) {
    this->local_x = global_x >> config::CHUNK_SIZE_BITS;
    this->local_y = global_y >> config::CHUNK_SIZE_BITS;
    this->local_z = global_z >> config::CHUNK_SIZE_BITS;

    this->set_state(ChunkState::EMPTY);

    this->_dirty_borders_mask = 0U;
    this->set_is_terrain_generation_complete(false);
}

void Chunk::generate(Generator &generator, HeightMap &height_map) {
    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                Block &block = this->get_block(x, y, z);

                block.set_type(BlockType::GRASS);
            }
        }
    }
}

void Chunk::generate_mesh() {
    this->clear_mesh();

    for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
        FaceType face_type = static_cast<FaceType>(face_type_index);

        int number_of_blocks = static_cast<int>(BlockType::COUNT);

        for (int block_type_index = 0; block_type_index < number_of_blocks; ++block_type_index) {
            BlockType block_type = static_cast<BlockType>(block_type_index);

            this->merge_faces(block_type, face_type);
        }
    }

    int index_offset = 0;

    for (Face &face : this->_faces) {
        face.add_to_mesh(this->_mesh);

        this->_mesh.add_index(0 + index_offset);
        this->_mesh.add_index(1 + index_offset);
        this->_mesh.add_index(2 + index_offset);
        this->_mesh.add_index(2 + index_offset);
        this->_mesh.add_index(3 + index_offset);
        this->_mesh.add_index(0 + index_offset);

        index_offset += 4;
    }

    int total_faces = this->_faces.size();

    LOG_INFO("Faces: {}, Vertices: {}", total_faces, total_faces << 2);
}

// NOTE: Once mesh is uploaded, we can set state to render
void Chunk::upload_mesh() {
    this->_mesh.upload();

    this->set_state(ChunkState::RENDERING);
}

void Chunk::merge_faces(BlockType &block_type, FaceType &face_type) {
    if (face_type == FaceType::FRONT || face_type == FaceType::BACK) {
        this->merge_XY_faces(block_type, face_type);
    } else if (face_type == FaceType::TOP || face_type == FaceType::BOTTOM) {
        this->merge_XZ_faces(block_type, face_type);
    } else {
        this->merge_YZ_faces(block_type, face_type);
    }
}

void Chunk::merge_XY_faces(BlockType &block_type, FaceType &face_type) {
    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        std::uint32_t masks[config::CHUNK_SIZE];

        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            masks[y] = 0U;

            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                Block &block = this->get_block(x, y, z);

                if (block.get_type() == block_type && block.is_face_active(face_type)) {
                    masks[y] |= (1U << x);
                }
            }
        }

        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            while (masks[y]) {
                int x = std::countr_zero(masks[y]);

                int width = std::countr_one(masks[y] >> x);

                std::uint32_t occupancy_mask = (width < 32) ? (((1U << width) - 1) << x) : this->_FULL_MASK;

                int height = 0;

                while (y + height < config::CHUNK_SIZE) {
                    if ((masks[y + height] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    masks[y + height] &= ~occupancy_mask;

                    ++height;
                }

                int block_x = this->global_x + x;
                int block_y = this->global_y + y;
                int block_z = this->global_z + z;

                this->_faces.emplace_back(block_type, face_type, block_x, block_y, block_z, width, height, 0);
            }
        }
    }
}

void Chunk::merge_XZ_faces(BlockType &block_type, FaceType &face_type) {
    for (int y = 0; y < config::CHUNK_SIZE; ++y) {
        std::uint32_t masks[config::CHUNK_SIZE];

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            masks[z] = 0U;

            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                Block &block = this->get_block(x, y, z);

                if (block.get_type() == block_type && block.is_face_active(face_type)) {
                    masks[z] |= (1U << x);
                }
            }
        }

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            while (masks[z]) {
                int x = std::countr_zero(masks[z]);

                int width = std::countr_one(masks[z] >> x);

                std::uint32_t occupancy_mask = (width < 32) ? (((1U << width) - 1) << x) : this->_FULL_MASK;

                int depth = 0;

                while (z + depth < config::CHUNK_SIZE) {
                    if ((masks[z + depth] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    masks[z + depth] &= ~occupancy_mask;

                    ++depth;
                }

                int block_x = this->global_x + x;
                int block_y = this->global_y + y;
                int block_z = this->global_z + z;

                this->_faces.emplace_back(block_type, face_type, block_x, block_y, block_z, width, 0, depth);
            }
        }
    }
}

void Chunk::merge_YZ_faces(BlockType &block_type, FaceType &face_type) {
    for (int x = 0; x < config::CHUNK_SIZE; ++x) {
        std::uint32_t masks[config::CHUNK_SIZE];

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            masks[z] = 0U;

            for (int y = 0; y < config::CHUNK_SIZE; ++y) {
                Block &block = this->get_block(x, y, z);

                if (block.get_type() == block_type && block.is_face_active(face_type)) {
                    masks[z] |= (1U << y);
                }
            }
        }

        for (int z = 0; z < config::CHUNK_SIZE; ++z) {
            while (masks[z]) {
                int y = std::countr_zero(masks[z]);

                int height = std::countr_one(masks[z] >> y);

                std::uint32_t occupancy_mask = (height < 32) ? (((1U << height) - 1) << y) : this->_FULL_MASK;

                int depth = 0;

                while (z + depth < config::CHUNK_SIZE) {
                    if ((masks[z + depth] & occupancy_mask) != occupancy_mask) {
                        break;
                    }

                    masks[z + depth] &= ~occupancy_mask;

                    ++depth;
                }

                int block_x = this->global_x + x;
                int block_y = this->global_y + y;
                int block_z = this->global_z + z;

                this->_faces.emplace_back(block_type, face_type, block_x, block_y, block_z, 0, height, depth);
            }
        }
    }
}

// BUG: Caused by internal hashmap corruption (inserts may trigger rehashes >:( )
// Better design could be to only occlude blocks within chunk? Mark neighbour chunks for remesh if adjacent chunks have been updated
// // NOTE: Only used by chunk generation, won't be used at a later stage...
void Chunk::occlude_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> &chunks) {
    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                Block &block = this->get_block(x, y, z);

                for (int face_type_index = 0; face_type_index < Face::NUMBER_OF_FACES; ++face_type_index) {
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

    // PERF: Separate chunk border occlusion from local chunk occlusionS
    // NOTE: Also only used by chunk generation for now...
    this->occlude_border_faces(chunks);
}

void Chunk::occlude_border_faces(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>, engine::math::hash::vector::IVec3Hash, engine::math::hash::vector::IVec3Equal> &chunks) {
    for (int face_type_index = 0; face_type_index < Face::NUMBER_OF_FACES; ++face_type_index) {
        int nx = Face::I_NORMALS[face_type_index][0];
        int ny = Face::I_NORMALS[face_type_index][1];
        int nz = Face::I_NORMALS[face_type_index][2];

        glm::ivec3 adjacent_chunk_id(this->local_x + nx, this->local_y + ny, this->local_z + nz);

        Chunk *adjacent_chunk = nullptr;

        chunks.visit(adjacent_chunk_id, [&](const auto &chunk_iterator) {
            adjacent_chunk = chunk_iterator.second.get();
        });

        if (adjacent_chunk == nullptr || !adjacent_chunk->is_terrain_generation_complete()) {
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

void Chunk::occlude_dirty_borders(boost::unordered::concurrent_flat_map<glm::ivec3, std::unique_ptr<engine::world::Chunk>, engine::math::hash::vector::IVec3Hash, engine::math::hash::vector::IVec3Equal> &chunks) {
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

        if (adjacent_chunk == nullptr || !adjacent_chunk->is_terrain_generation_complete()) {
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
void Chunk::cull_face_based_on_adjacent_block(Block &block_a, Block &block_b, int face_type_index) {
    if (block_b.get_type() != BlockType::EMPTY) {
        block_a.set_face_state(face_type_index, false);
    } else {
        block_a.set_face_state(face_type_index, true);
    }
}

void Chunk::clear_mesh() {
    this->_mesh.clear();

    this->_faces.clear();
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

bool Chunk::is_terrain_generation_complete() {
    return this->_is_terrain_generation_complete.load(std::memory_order_acquire);
}

} // namespace engine::world
