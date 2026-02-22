#pragma once

#include <FastNoise/FastNoise.h>

namespace engine::world {

class Generator {
  public:
    Generator();

    int get_height(int x, int z);

    float get_cave_noise(int x, int y, int z);

  private:
    static inline constexpr int _SEED = (1 << 4) | (1 << 8) | (1 << 16);

    static inline constexpr int _OCTAVES = 4;

    static inline constexpr std::pair<float, float> _HEIGHT_LIMIT = {0.0f, 64.0f};

    FastNoise::SmartNode<FastNoise::FractalFBm> _height_noise;
    FastNoise::SmartNode<FastNoise::FractalFBm> _cave_noise;

    void initialise_height_noise();

    void initialise_cave_noise();
};

} // namespace engine::world
