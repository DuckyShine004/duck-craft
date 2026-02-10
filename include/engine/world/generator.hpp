#pragma once

#include <FastNoise/FastNoise.h>

namespace engine::world {

class Generator {
  public:
    Generator();

    int get_height(int x, int z);

  private:
    static inline constexpr int _SEED = (1 << 4) | (1 << 8) | (1 << 16);

    static inline constexpr int _OCTAVES = 4;

    static inline constexpr std::pair<float, float> _HEIGHT_LIMIT = {0.0f, 64.0f};

    FastNoise::SmartNode<FastNoise::FractalFBm> _fractal;
};

} // namespace engine::world
