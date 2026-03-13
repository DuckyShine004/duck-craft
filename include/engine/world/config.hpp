#pragma once

#include <cstdint>

namespace engine::world::config {

static inline constexpr int CHUNK_SIZE = 32;
static inline constexpr int CHUNK_SIZE2 = 1024;
static inline constexpr int CHUNK_SIZE3 = 32768;

static inline constexpr int CHUNK_SIZE_HALF = 16;

static inline constexpr float CHUNK_SIZE_HALF_F = 16.0f;

static inline constexpr int CHUNK_SIZE_BITS = 5;
static inline constexpr int CHUNK_SIZE_BITS2 = 10;

static inline constexpr int HORIZONTAL_RENDER_DISTANCE = 3;
static inline constexpr int VERTICAL_RENDER_DISTANCE = 1;

static inline constexpr std::uint32_t MASK32 = 0xFFFFFFFF;

} // namespace engine::world::config
