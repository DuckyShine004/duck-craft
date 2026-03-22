#pragma once

#include <vector>

#include "engine/world/block_type.hpp"

namespace engine::world {

struct TreeInfo {
    int dx;
    int dy;
    int dz;

    engine::world::BlockType block_type;
};

struct Tree {
    int x;
    int y;
    int z;

    std::vector<TreeInfo> tree_infos;
};

} // namespace engine::world
