#include "engine/world/config.hpp"
#include "engine/world/generator.hpp"

#include "logger/logger_macros.hpp"

namespace engine::world {

Generator::Generator() {
    this->initialise_height_noise();

    this->initialise_cave_noise();
}

void Generator::initialise_height_noise() {
    FastNoise::SmartNode<FastNoise::Simplex> simplex = FastNoise::New<FastNoise::Simplex>();

    // 1 / 0.01(s) = f
    float scale = 100.0f;

    simplex->SetScale(scale);

    float min_height = 0.0f;
    float max_height = 64.0f;

    simplex->SetOutputMin(min_height);
    simplex->SetOutputMax(max_height);

    FastNoise::SmartNode<FastNoise::FractalFBm> fbm = FastNoise::New<FastNoise::FractalFBm>();

    fbm->SetSource(simplex);

    fbm->SetOctaveCount(this->_OCTAVES);

    this->_height_noise = simplex;

    // this->_height_noise = FastNoise::New<FastNoise::FractalFBm>();
    //
    // this->_height_noise->SetSource(simplex);
    //
    // this->_height_noise->SetOctaveCount(this->_OCTAVES);
}

void Generator::initialise_cave_noise() {
    // FastNoise::SmartNode<FastNoise::Simplex> simplex = FastNoise::New<FastNoise::Simplex>();
    //
    // float scale = 50.0f;
    //
    // simplex->SetScale(scale);
    //
    // this->_cave_noise = FastNoise::New<FastNoise::FractalFBm>();
    //
    // this->_cave_noise->SetSource(simplex);
    //
    // this->_cave_noise->SetOctaveCount(this->_OCTAVES);

    // TEST: here
    // --- Cave density (what you already have, but tuned a bit) ---
    // Cave density source (what actually returns scalar values)
    auto caveBase = FastNoise::New<FastNoise::Simplex>();
    caveBase->SetScale(150.0f);

    auto caveFbm = FastNoise::New<FastNoise::FractalFBm>();
    caveFbm->SetSource(caveBase);
    caveFbm->SetOctaveCount(4);

    // Pick a concrete warp node (DomainWarp is abstract)
    auto warp = FastNoise::New<FastNoise::DomainWarpGradient>();
    warp->SetWarpAmplitude(60.0f);
    warp->SetScale(150.0f);
    // warp->SetScale(150.0f);

    // Warp uses cave density as its Source (i.e., sample caveFbm at warped coords)
    warp->SetSource(caveFbm);

    // Optional: make the warp itself fractal (progressive is usually best for “flowy”)
    auto warpFractal = FastNoise::New<FastNoise::DomainWarpFractalProgressive>();
    warpFractal->SetSource(warp);
    warpFractal->SetOctaveCount(2);
    warpFractal->SetGain(0.5f);
    warpFractal->SetLacunarity(2.0f);

    // Store the final node (type is SmartNode<Generator>, so this is fine)
    this->_cave_noise = warpFractal;
}

int Generator::get_height(int x, int y) {
    int height = this->_height_noise->GenSingle2D(x, y, config::SEED);

    // LOG_INFO("Height: {}", height);

    return height;
}

// NOTE: Meaning we simply skip (or replace with empty block)
bool Generator::is_cave(int x, int y, int z) {
    float noise = this->_cave_noise->GenSingle3D(x, y, z, config::SEED);

    constexpr float iso = 0.15f;
    constexpr float width = 0.08f;

    return std::abs(noise - iso) < width;
}

bool Generator::is_grass(int x, int y, int z) {
    std::uint32_t seed = this->get_seed(x, y, z, config::SEED);

    this->_rng.seed(seed);

    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    float threshold = distribution(this->_rng);

    return threshold <= 0.2f;
}

BlockType Generator::get_grass(int x, int y, int z) {
    std::uint32_t seed = this->get_seed(x, y, z, config::SEED ^ 0xA5A5A5A5u);

    this->_rng.seed(seed);

    constexpr BlockType flower_types[2] = {
        BlockType::SHORT_GRASS,
        BlockType::TALL_GRASS,
    };

    std::uniform_int_distribution<int> distribution(0, 1);

    int index = distribution(this->_rng);

    return flower_types[index];
}

bool Generator::is_flower(int x, int y, int z) {
    std::uint32_t seed = this->get_seed(x, y, z, config::SEED);

    this->_rng.seed(seed);

    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    float threshold = distribution(this->_rng);

    return threshold <= 0.025f;
}

BlockType Generator::get_flower(int x, int y, int z) {
    std::uint32_t seed = this->get_seed(x, y, z, config::SEED ^ 0xA5A5A5A5u);

    this->_rng.seed(seed);

    constexpr BlockType flower_types[2] = {
        BlockType::DANDELION,
        BlockType::ROSE,
    };

    std::uniform_int_distribution<int> distribution(0, 1);

    int index = distribution(this->_rng);

    return flower_types[index];
}

bool Generator::is_tree(int x, int y, int z) {
    if (this->get_surface_block_type(x, y, z) != BlockType::GRASS) {
        return false;
    }

    std::uint32_t seed = this->get_seed(x, y, z, config::SEED);

    int threshold = seed % 1000;

    return threshold <= 20;
}

void Generator::create_tree(Tree &tree, int x, int y, int z) {
    std::uint32_t seed = this->get_seed(x, y, z, config::SEED ^ 0xA5A5A5A5u);

    int height = 5 + (seed % 3);

    tree.x = x;
    tree.y = y;
    tree.z = z;

    for (int dy = 1; dy <= height; ++dy) {
        tree.tree_infos.emplace_back(0, dy, 0, BlockType::OAK_LOG);
    }

    for (int dy = 0; dy < 2; ++dy) {
        int _y = height - 2 + dy;

        for (int dz = -2; dz <= 2; ++dz) {
            for (int dx = -2; dx <= 2; ++dx) {
                if (dx == -2 && dz == -2) {
                    continue;
                }

                if (dx == -2 && dz == 2) {
                    continue;
                }

                if (dx == 2 && dz == -2) {
                    continue;
                }

                if (dx == 2 && dz == 2) {
                    continue;
                }

                if (dx == 0 && dz == 0) {
                    continue;
                }

                tree.tree_infos.emplace_back(dx, _y, dz, BlockType::OAK_LEAVES);
            }
        }
    }

    for (int dy = 0; dy < 2; ++dy) {
        int _y = height + dy;

        for (int dz = -1; dz <= 1; ++dz) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == -1 && dz == -1) {
                    continue;
                }

                if (dx == -1 && dz == 1) {
                    continue;
                }

                if (dx == 1 && dz == -1) {
                    continue;
                }

                if (dx == 1 && dz == 1) {
                    continue;
                }

                if (dx == 0 && dz == 0 && _y <= height) {
                    continue;
                }

                tree.tree_infos.emplace_back(dx, _y, dz, BlockType::OAK_LEAVES);
            }
        }
    }
}

std::uint32_t Generator::get_seed(int x, int y, int z, std::uint32_t seed) {
    std::uint32_t h = seed;

    h ^= static_cast<std::uint32_t>(x) * 0x9E3779B1U;
    h ^= static_cast<std::uint32_t>(y) * 0x85EBCA77U;
    h ^= static_cast<std::uint32_t>(z) * 0xC2B2AE3DU;

    h ^= h >> 16;
    h *= 0x85EBCA6BU;

    h ^= h >> 13;
    h *= 0xC2B2AE35U;

    h ^= h >> 16;

    return h;
}

BlockType Generator::get_surface_block_type(int x, int y, int z) {
    if (y <= 10) {
        return BlockType::WATER;
    }

    if (this->is_cave(x, y, z)) {
        return BlockType::EMPTY;
    }

    if (y <= 13) {
        return BlockType::SAND;
    }

    return BlockType::GRASS;
}

} // namespace engine::world
