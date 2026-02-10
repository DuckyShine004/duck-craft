#pragma once

#include <cstdint>

namespace engine::world {

/* NOTE: positive side are divisible by 2 */
enum class FaceType : std::uint8_t {
    TOP = 0,
    BOTTOM,
    RIGHT,
    LEFT,
    FRONT,
    BACK,
};

} // namespace engine::world
