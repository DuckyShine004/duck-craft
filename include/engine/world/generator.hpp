#pragma once

#include <FastNoise/FastNoise.h>

namespace engine::world {

class Generator {
  public:
    Generator();

    int get_height(int x, int z);

    bool is_cave(int x, int y, int z);

  private:
    static inline constexpr int _SEED = (1 << 4) | (1 << 8) | (1 << 16);

    static inline constexpr int _OCTAVES = 4;

    static inline constexpr std::pair<float, float> _HEIGHT_LIMIT = {0.0f, 64.0f};

    FastNoise::SmartNode<FastNoise::Generator> _height_noise;
    FastNoise::SmartNode<FastNoise::Generator> _cave_noise;

    void initialise_height_noise();

    void initialise_cave_noise();
};

} // namespace engine::world
