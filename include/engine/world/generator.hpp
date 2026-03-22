#pragma once

#include <random>

#include <FastNoise/FastNoise.h>

#include "engine/world/tree.hpp"
#include "engine/world/config.hpp"
#include "engine/world/block_type.hpp"

namespace engine::world {

class Generator {
  public:
    Generator();

    int get_height(int x, int z);

    bool is_cave(int x, int y, int z);

    bool is_grass(int x, int y, int z);
    engine::world::BlockType get_grass(int x, int y, int z);

    bool is_flower(int x, int y, int z);
    engine::world::BlockType get_flower(int x, int y, int z);

    bool is_tree(int x, int y, int z);
    void create_tree(engine::world::Tree &tree, int x, int y, int z);

    engine::world::BlockType get_surface_block_type(int x, int y, int z);

  private:
    static inline constexpr int _OCTAVES = 4;

    static inline constexpr std::pair<float, float> _HEIGHT_LIMIT = {0.0f, 64.0f};

    FastNoise::SmartNode<FastNoise::Generator> _height_noise;
    FastNoise::SmartNode<FastNoise::Generator> _cave_noise;

    std::mt19937 _rng;

    void initialise_height_noise();

    void initialise_cave_noise();

    std::uint32_t get_seed(int x, int y, int z, std::uint32_t seed);
};

} // namespace engine::world
