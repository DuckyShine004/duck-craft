#pragma once

#include "engine/world/block_type.hpp"
#include "engine/world/block_flag.hpp"

namespace engine::world {

struct BlockMetadata {
    engine::world::BlockFlag flags = engine::world::BlockFlag::NONE;

    std::uint8_t texture_ids[6];
};

// clang-format off
// inline constexpr engine::world::BlockMetadata BLOCK_METADATA[static_cast<std::size_t>(engine::world::BlockType::COUNT)] = {
//     {engine::world::BlockFlag::TRANSPARENT},
//     {engine::world::BlockFlag::NONE | engine::world::BlockFlag::AMBIENT_OCCLUSION},
//     {engine::world::BlockFlag::NONE | engine::world::BlockFlag::AMBIENT_OCCLUSION},
//     {engine::world::BlockFlag::NONE | engine::world::BlockFlag::AMBIENT_OCCLUSION},
//     {engine::world::BlockFlag::TRANSPARENT | engine::world::BlockFlag::DIFFUSE_LIGHT},
//     {engine::world::BlockFlag::NONE | engine::world::BlockFlag::AMBIENT_OCCLUSION},
//     {engine::world::BlockFlag::NONE | engine::world::BlockFlag::AMBIENT_OCCLUSION},
//     {engine::world::BlockFlag::TRANSPARENT | engine::world::BlockFlag::DIFFUSE_LIGHT | engine::world::BlockFlag::AMBIENT_OCCLUSION},
//     {engine::world::BlockFlag::TRANSPARENT | engine::world::BlockFlag::FOLIAGE},
//     {engine::world::BlockFlag::TRANSPARENT | engine::world::BlockFlag::FOLIAGE},
//     {engine::world::BlockFlag::TRANSPARENT | engine::world::BlockFlag::FOLIAGE},
//     {engine::world::BlockFlag::TRANSPARENT | engine::world::BlockFlag::FOLIAGE},
// };
inline engine::world::BlockMetadata BLOCK_METADATA[static_cast<std::size_t>(engine::world::BlockType::COUNT)];
// clang-format on

} // namespace engine::world
