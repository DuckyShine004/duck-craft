#include <bit>
#include <cstring>

#include "engine/world/chunk.hpp"

#include "logger/entry.hpp"
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
    // Mesh top face
    this->_mesh.clear_vertices();

    int index_offset = 0;

    for (int face_type_index = 0; face_type_index < 6; ++face_type_index) {
        FaceType face_type = static_cast<FaceType>(face_type_index);

        int number_of_blocks = static_cast<int>(BlockType::COUNT);

        for (int block_type_index = 0; block_type_index < number_of_blocks; ++block_type_index) {
            BlockType block_type = static_cast<BlockType>(block_type_index);

            std::uint32_t block_masks[config::CHUNK_SIZE];

            for (int block_id = 0; block_id < config::CHUNK_SIZE2; ++block_id) {
                int block_y = 1;
                int block_x = 1;
            }
        }
    }

    this->merge_X_faces(index_offset);
    this->merge_Y_faces(index_offset);
    this->merge_Z_faces(index_offset);

    this->_mesh.upload();
}

void Chunk::merge_X_faces(int &index_offset) {
    for (int dx = 0; dx < config::CHUNK_SIZE; ++dx) {
        // this->merge_left_faces(index_offset, dx);
        // this->merge_right_faces(index_offset, dx);
    }
}

void Chunk::merge_Y_faces(int &index_offset) {
    for (int dy = 0; dy < config::CHUNK_SIZE; ++dy) {
        this->merge_top_faces(index_offset, dy);
        // this->merge_bottom_faces(index_offset, dy);
    }
}

void Chunk::merge_Z_faces(int &index_offset) {
    for (int dz = 0; dz < config::CHUNK_SIZE; ++dz) {
        // this->merge_front_faces(index_offset, dz);
        // this->merge_back_faces(index_offset, dz);
    }
}

void Chunk::merge_left_faces(int &index_offset, int dx) {
    bool visited[config::CHUNK_SIZE][config::CHUNK_SIZE];

    std::memset(visited, false, sizeof(visited));

    // NOTE: Order matters, ZY would be more performant due to locality
    // However, for the sake of not confusing myself, we use YZ convention
    for (int dy = 0; dy < config::CHUNK_SIZE; ++dy) {
        for (int dz = 0; dz < config::CHUNK_SIZE; ++dz) {
            if (visited[dy][dz]) {
                continue;
            }

            int id = this->get_block_id(dx, dy, dz);

            if (!this->_blocks[id].is_face_active(FaceType::LEFT)) {
                continue;
            }

            BlockType &block_type = this->_blocks[id].get_type();

            if (block_type == BlockType::EMPTY) {
                continue;
            }

            int depth = 1;

            while (dz + depth < config::CHUNK_SIZE) {
                int block_id = this->get_block_id(dx, dy, dz + depth);

                Block &block = this->_blocks[block_id];

                if (block.get_type() != block_type || !block.is_face_active(FaceType::LEFT)) {
                    break;
                }

                ++depth;
            }

            int height = 1;

            while (dy + height < config::CHUNK_SIZE) {
                bool valid = true;

                for (int dd = 0; dd < depth; ++dd) {
                    int block_id = this->get_block_id(dx, dy + height, dz + dd);

                    Block &block = this->_blocks[block_id];

                    if (block.get_type() != block_type || !block.is_face_active(FaceType::LEFT)) {
                        valid = false;
                        break;
                    }
                }

                if (!valid) {
                    break;
                }

                ++height;
            }

            for (int ly = 0; ly < height; ++ly) {
                for (int lz = 0; lz < depth; ++lz) {
                    visited[dy + ly][dz + lz] = true;
                }
            }

            int x = this->global_x + dx;
            int y = this->global_y + dy;
            int z = this->global_z + dz;

            int texture_id = static_cast<int>(block_type);

            this->_mesh.add_vertex(x, y, z, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            this->_mesh.add_vertex(x, y, z + depth, -1.0f, 0.0f, 0.0f, depth, 0.0f);
            this->_mesh.add_vertex(x, y + height, z + depth, -1.0f, 0.0f, 0.0f, depth, height);
            this->_mesh.add_vertex(x, y + height, z, -1.0f, 0.0f, 0.0f, 0.0f, height);

            this->_mesh.add_index(0 + index_offset);
            this->_mesh.add_index(1 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(3 + index_offset);
            this->_mesh.add_index(0 + index_offset);

            index_offset += 4;
        }
    }
}

void Chunk::merge_right_faces(int &index_offset, int dx) {
    bool visited[config::CHUNK_SIZE][config::CHUNK_SIZE];

    std::memset(visited, false, sizeof(visited));

    for (int dy = 0; dy < config::CHUNK_SIZE; ++dy) {
        for (int dz = 0; dz < config::CHUNK_SIZE; ++dz) {
            if (visited[dy][dz]) {
                continue;
            }

            int id = this->get_block_id(dx, dy, dz);

            if (!this->_blocks[id].is_face_active(FaceType::RIGHT)) {
                continue;
            }

            BlockType &block_type = this->_blocks[id].get_type();

            if (block_type == BlockType::EMPTY) {
                continue;
            }

            int depth = 1;

            while (dz + depth < config::CHUNK_SIZE) {
                int block_id = this->get_block_id(dx, dy, dz + depth);

                Block &block = this->_blocks[block_id];

                if (block.get_type() != block_type || !block.is_face_active(FaceType::RIGHT)) {
                    break;
                }

                ++depth;
            }

            int height = 1;

            while (dy + height < config::CHUNK_SIZE) {
                bool valid = true;

                for (int dd = 0; dd < depth; ++dd) {
                    int block_id = this->get_block_id(dx, dy + height, dz + dd);

                    Block &block = this->_blocks[block_id];

                    if (block.get_type() != block_type || !block.is_face_active(FaceType::RIGHT)) {
                        valid = false;
                        break;
                    }
                }

                if (!valid) {
                    break;
                }

                ++height;
            }

            for (int ly = 0; ly < height; ++ly) {
                for (int lz = 0; lz < depth; ++lz) {
                    visited[dy + ly][dz + lz] = true;
                }
            }

            int x = this->global_x + dx;
            int y = this->global_y + dy;
            int z = this->global_z + dz;

            int texture_id = static_cast<int>(block_type);

            this->_mesh.add_vertex(x + 1.0f, y, z + depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            this->_mesh.add_vertex(x + 1.0f, y, z, 1.0f, 0.0f, 0.0f, depth, 0.0f);
            this->_mesh.add_vertex(x + 1.0f, y + height, z, 1.0f, 0.0f, 0.0f, depth, height);
            this->_mesh.add_vertex(x + 1.0f, y + height, z + depth, 1.0f, 0.0f, 0.0f, 0.0f, height);

            this->_mesh.add_index(0 + index_offset);
            this->_mesh.add_index(1 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(3 + index_offset);
            this->_mesh.add_index(0 + index_offset);

            index_offset += 4;
        }
    }
}

// void Chunk::merge_top_faces(int &index_offset, int dy) {
//     bool visited[config::CHUNK_SIZE][config::CHUNK_SIZE];
//
//     std::memset(visited, false, sizeof(visited));
//
//     for (int dz = 0; dz < config::CHUNK_SIZE; ++dz) {
//         for (int dx = 0; dx < config::CHUNK_SIZE; ++dx) {
//             if (visited[dz][dx]) {
//                 continue;
//             }
//
//             int id = this->get_block_id(dx, dy, dz);
//
//             if (!this->_blocks[id].is_face_active(FaceType::TOP)) {
//                 continue;
//             }
//
//             BlockType &block_type = this->_blocks[id].get_type();
//
//             if (block_type == BlockType::EMPTY) {
//                 continue;
//             }
//
//             int width = 1;
//
//             while (dx + width < config::CHUNK_SIZE) {
//                 int block_id = this->get_block_id(dx + width, dy, dz);
//
//                 Block &block = this->_blocks[block_id];
//
//                 if (block.get_type() != block_type || !block.is_face_active(FaceType::TOP)) {
//                     break;
//                 }
//
//                 ++width;
//             }
//
//             int depth = 1;
//
//             while (dz + depth < config::CHUNK_SIZE) {
//                 bool valid = true;
//
//                 for (int dw = 0; dw < width; ++dw) {
//                     int block_id = this->get_block_id(dx + dw, dy, dz + depth);
//
//                     Block &block = this->_blocks[block_id];
//
//                     if (block.get_type() != block_type || !block.is_face_active(FaceType::TOP)) {
//                         valid = false;
//                         break;
//                     }
//                 }
//
//                 if (!valid) {
//                     break;
//                 }
//
//                 ++depth;
//             }
//
//             for (int lz = 0; lz < depth; ++lz) {
//                 for (int lx = 0; lx < width; ++lx) {
//                     visited[dz + lz][dx + lx] = true;
//                 }
//             }
//
//             int x = this->global_x + dx;
//             int y = this->global_y + dy;
//             int z = this->global_z + dz;
//
//             this->_mesh.add_vertex(x, y + 1.0f, z + depth, 0.0, 1.0f, 0.0f, 0.0f, 0.0f);
//             this->_mesh.add_vertex(x + width, y + 1.0f, z + depth, 0.0f, 1.0f, 0.0f, width, 0.0f);
//             this->_mesh.add_vertex(x + width, y + 1.0f, z, 0.0f, 1.0f, 0.0f, width, depth);
//             this->_mesh.add_vertex(x, y + 1.0f, z, 0.0f, 1.0f, 0.0f, 0.0f, depth);
//
//             this->_mesh.add_index(0 + index_offset);
//             this->_mesh.add_index(1 + index_offset);
//             this->_mesh.add_index(2 + index_offset);
//             this->_mesh.add_index(2 + index_offset);
//             this->_mesh.add_index(3 + index_offset);
//             this->_mesh.add_index(0 + index_offset);
//
//             index_offset += 4;
//         }
//     }
// }

void Chunk::merge_top_faces(int &index_offset, int dy) {
    std::uint32_t masks[config::CHUNK_SIZE];

    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        masks[z] = 0U;

        for (int x = 0; x < config::CHUNK_SIZE; ++x) {
            int id = this->get_block_id(x, dy, z);

            if (this->_blocks[id].get_type() == BlockType::EMPTY) {
                continue;
            }

            masks[z] |= (1U << x);
        }
    }

    for (int z = 0; z < config::CHUNK_SIZE; ++z) {
        while (masks[z]) {
            // Get position of first block
            int x = std::countr_zero(masks[z]);

            // Get the number of consecutive ones
            int width = std::countr_one(masks[z] >> x);

            LOG_DEBUG("Mask width: {}", width);

            std::uint32_t max = 0xFFFFFFFF;

            // Create a occupancy mask for the consecutive blocks
            std::uint32_t mask = (width < 32) ? (((1U << width) - 1) << x) : max;

            // Find the maximum depth
            int depth = 0;

            while (z + depth < config::CHUNK_SIZE) {
                if ((masks[z + depth] & mask) != mask) {
                    break;
                }

                masks[z + depth] &= ~mask;

                ++depth;
            }

            int wx = this->global_x + x;
            int wy = this->global_y + dy;
            int wz = this->global_z + z;

            this->_mesh.add_vertex(wx, wy + 1.0f, wz + depth, 0.0, 1.0f, 0.0f, 0.0f, 0.0f);
            this->_mesh.add_vertex(wx + width, wy + 1.0f, wz + depth, 0.0f, 1.0f, 0.0f, width, 0.0f);
            this->_mesh.add_vertex(wx + width, wy + 1.0f, wz, 0.0f, 1.0f, 0.0f, width, depth);
            this->_mesh.add_vertex(wx, wy + 1.0f, wz, 0.0f, 1.0f, 0.0f, 0.0f, depth);

            this->_mesh.add_index(0 + index_offset);
            this->_mesh.add_index(1 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(3 + index_offset);
            this->_mesh.add_index(0 + index_offset);

            index_offset += 4;
        }
    }
}

void Chunk::merge_bottom_faces(int &index_offset, int dy) {
    bool visited[config::CHUNK_SIZE][config::CHUNK_SIZE];

    std::memset(visited, false, sizeof(visited));

    for (int dz = 0; dz < config::CHUNK_SIZE; ++dz) {
        for (int dx = 0; dx < config::CHUNK_SIZE; ++dx) {
            if (visited[dz][dx]) {
                continue;
            }

            int id = this->get_block_id(dx, dy, dz);

            if (!this->_blocks[id].is_face_active(FaceType::BOTTOM)) {
                continue;
            }

            BlockType &block_type = this->_blocks[id].get_type();

            if (block_type == BlockType::EMPTY) {
                continue;
            }

            int width = 1;

            while (dx + width < config::CHUNK_SIZE) {
                int block_id = this->get_block_id(dx + width, dy, dz);

                Block &block = this->_blocks[block_id];

                if (block.get_type() != block_type || !block.is_face_active(FaceType::BOTTOM)) {
                    break;
                }

                ++width;
            }

            int depth = 1;

            while (dz + depth < config::CHUNK_SIZE) {
                bool valid = true;

                for (int dw = 0; dw < width; ++dw) {
                    int block_id = this->get_block_id(dx + dw, dy, dz + depth);

                    Block &block = this->_blocks[block_id];

                    if (block.get_type() != block_type || !block.is_face_active(FaceType::BOTTOM)) {
                        valid = false;
                        break;
                    }
                }

                if (!valid) {
                    break;
                }

                ++depth;
            }

            for (int lz = 0; lz < depth; ++lz) {
                for (int lx = 0; lx < width; ++lx) {
                    visited[dz + lz][dx + lx] = true;
                }
            }

            int x = this->global_x + dx;
            int y = this->global_y + dy;
            int z = this->global_z + dz;

            int texture_id = static_cast<int>(block_type);

            this->_mesh.add_vertex(x + width, y, z + depth, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
            this->_mesh.add_vertex(x, y, z + depth, 0.0f, -1.0f, 0.0f, width, 0.0f);
            this->_mesh.add_vertex(x, y, z, 0.0f, -1.0f, 0.0f, width, depth);
            this->_mesh.add_vertex(x + width, y, z, 0.0f, -1.0f, 0.0f, 0.0f, depth);

            this->_mesh.add_index(0 + index_offset);
            this->_mesh.add_index(1 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(3 + index_offset);
            this->_mesh.add_index(0 + index_offset);

            index_offset += 4;
        }
    }
}

void Chunk::merge_front_faces(int &index_offset, int dz) {
    bool visited[config::CHUNK_SIZE][config::CHUNK_SIZE];

    std::memset(visited, false, sizeof(visited));

    for (int dy = 0; dy < config::CHUNK_SIZE; ++dy) {
        for (int dx = 0; dx < config::CHUNK_SIZE; ++dx) {
            if (visited[dy][dx]) {
                continue;
            }

            int id = this->get_block_id(dx, dy, dz);

            if (!this->_blocks[id].is_face_active(FaceType::FRONT)) {
                continue;
            }

            BlockType &block_type = this->_blocks[id].get_type();

            if (block_type == BlockType::EMPTY) {
                continue;
            }

            int width = 1;

            while (dx + width < config::CHUNK_SIZE) {
                int block_id = this->get_block_id(dx + width, dy, dz);

                Block &block = this->_blocks[block_id];

                if (block.get_type() != block_type || !block.is_face_active(FaceType::FRONT)) {
                    break;
                }

                ++width;
            }

            int height = 1;

            while (dy + height < config::CHUNK_SIZE) {
                bool valid = true;

                for (int dh = 0; dh < height; ++dh) {
                    int block_id = this->get_block_id(dx + width, dy + dh, dz);

                    Block &block = this->_blocks[block_id];

                    if (block.get_type() != block_type || !block.is_face_active(FaceType::FRONT)) {
                        valid = false;
                        break;
                    }
                }

                if (!valid) {
                    break;
                }

                ++height;
            }

            for (int ly = 0; ly < height; ++ly) {
                for (int lx = 0; lx < width; ++lx) {
                    visited[dy + ly][dx + lx] = true;
                }
            }

            int x = this->global_x + dx;
            int y = this->global_y + dy;
            int z = this->global_z + dz;

            int texture_id = static_cast<int>(block_type);

            // NOTE: dx is row, dz is col, meaning u = depth, v = width
            this->_mesh.add_vertex(x, y, z + 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
            this->_mesh.add_vertex(x + width, y, z + 1.0f, 0.0f, 0.0f, 1.0f, width, 0.0f);
            this->_mesh.add_vertex(x + width, y + height, z + 1.0f, 0.0f, 0.0f, 1.0f, width, height);
            this->_mesh.add_vertex(x, y + height, z + 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, height);

            this->_mesh.add_index(0 + index_offset);
            this->_mesh.add_index(1 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(3 + index_offset);
            this->_mesh.add_index(0 + index_offset);

            index_offset += 4;
        }
    }
}

void Chunk::merge_back_faces(int &index_offset, int dz) {
    bool visited[config::CHUNK_SIZE][config::CHUNK_SIZE];

    std::memset(visited, false, sizeof(visited));

    for (int dy = 0; dy < config::CHUNK_SIZE; ++dy) {
        for (int dx = 0; dx < config::CHUNK_SIZE; ++dx) {
            if (visited[dy][dx]) {
                continue;
            }

            int id = this->get_block_id(dx, dy, dz);

            if (!this->_blocks[id].is_face_active(FaceType::BACK)) {
                continue;
            }

            BlockType &block_type = this->_blocks[id].get_type();

            if (block_type == BlockType::EMPTY) {
                continue;
            }

            int width = 1;

            while (dx + width < config::CHUNK_SIZE) {
                int block_id = this->get_block_id(dx + width, dy, dz);

                Block &block = this->_blocks[block_id];

                if (block.get_type() != block_type || !block.is_face_active(FaceType::BACK)) {
                    break;
                }

                ++width;
            }

            int height = 1;

            while (dy + height < config::CHUNK_SIZE) {
                bool valid = true;

                for (int dh = 0; dh < height; ++dh) {
                    int block_id = this->get_block_id(dx + width, dy + dh, dz);

                    Block &block = this->_blocks[block_id];

                    if (block.get_type() != block_type || !block.is_face_active(FaceType::BACK)) {
                        valid = false;
                        break;
                    }
                }

                if (!valid) {
                    break;
                }

                ++height;
            }

            for (int ly = 0; ly < height; ++ly) {
                for (int lx = 0; lx < width; ++lx) {
                    visited[dy + ly][dx + lx] = true;
                }
            }

            int x = this->global_x + dx;
            int y = this->global_y + dy;
            int z = this->global_z + dz;

            int texture_id = static_cast<int>(block_type);

            // NOTE: dx is row, dz is col, meaning u = depth, v = width
            this->_mesh.add_vertex(x + width, y, z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
            this->_mesh.add_vertex(x, y, z, 0.0f, 0.0f, -1.0f, width, 0.0f);
            this->_mesh.add_vertex(x, y + height, z, 0.0f, 0.0f, -1.0f, width, height);
            this->_mesh.add_vertex(x + width, y + height, z, 0.0f, 0.0f, -1.0f, 0.0f, height);

            this->_mesh.add_index(0 + index_offset);
            this->_mesh.add_index(1 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(2 + index_offset);
            this->_mesh.add_index(3 + index_offset);
            this->_mesh.add_index(0 + index_offset);

            index_offset += 4;
        }
    }
}

void Chunk::cull_block_faces() {
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
