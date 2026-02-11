#include <bit>
#include <cstring>

#include "engine/world/chunk.hpp"

#include "utility/colour_utility.hpp"

#include "common/constant.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::shader;

namespace engine::world {

Chunk::Chunk(int global_x, int global_y, int global_z) : global_x(global_x), global_y(global_y), global_z(global_z) {
    this->local_x = global_x >> config::CHUNK_SIZE_BITS;
    this->local_y = global_y >> config::CHUNK_SIZE_BITS;
    this->local_z = global_z >> config::CHUNK_SIZE_BITS;
}

void Chunk::generate(Generator &generator, HeightMap &height_map) {
    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        for (int y = 0; y < config::CHUNK_SIZE; ++y) {
            for (int x = 0; x < config::CHUNK_SIZE; ++x) {
                int id = this->get_block_id(x, y, z);

                this->_blocks[id].set_type(BlockType::GRASS);
            }
        }
    }

    this->cull_block_faces();
}

// TODO: Greedy meshing
// PERF: Binary meshing
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

    int total_faces = 0;

    for (Face &face : this->_faces) {
        face.add_to_mesh(this->_mesh);

        this->_mesh.add_index(0 + index_offset);
        this->_mesh.add_index(1 + index_offset);
        this->_mesh.add_index(2 + index_offset);
        this->_mesh.add_index(2 + index_offset);
        this->_mesh.add_index(3 + index_offset);
        this->_mesh.add_index(0 + index_offset);

        ++total_faces;

        index_offset += 4;
    }

    LOG_DEBUG("Faces: {}, Vertices: {}", total_faces, total_faces << 2);

    this->_mesh.upload();
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
                int id = this->get_block_id(x, y, z);

                if (this->_blocks[id].get_type() == block_type) {
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
                int id = this->get_block_id(x, y, z);

                if (this->_blocks[id].get_type() == block_type) {
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
                int id = this->get_block_id(x, y, z);

                if (this->_blocks[id].get_type() == block_type) {
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

void Chunk::cull_block_faces() {
}

void Chunk::clear_mesh() {
    this->_mesh.clear_vertices();

    this->_faces.clear();
}

void Chunk::render(Shader &shader) {
    glm::mat4 model(1.0f);

    shader.set_matrix4fv("u_model", model);

    shader.set_vector3f("u_colour", utility::ColourUtility::get_high_precision_RGB(common::WHITE_RGB));

    this->_mesh.render(this->_TOPOLOGY);
}

int Chunk::get_block_id(int x, int y, int z) {
    return x + (y << config::CHUNK_SIZE_BITS) + (z << config::CHUNK_SIZE_BITS2);
}

} // namespace engine::world
