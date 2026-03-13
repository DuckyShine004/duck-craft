#pragma once

#include <cstdint>

namespace engine::world::Light {

/* NOTE:
 * 0-7: sunlight
 * 8-15: block light
 */

inline constexpr std::uint16_t SUNLIGHT_MASK = 0x00FF;

inline constexpr std::uint8_t get_sunlight(const std::uint16_t &light) {
    return light & SUNLIGHT_MASK;
}

inline constexpr void set_sunlight(std::uint16_t &light, const std::uint8_t &sunlight) {
    light = (light & ~SUNLIGHT_MASK) | sunlight;
}

} // namespace engine::world::Light
