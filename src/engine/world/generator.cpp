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
    float scale = 200.0f;

    simplex->SetScale(scale);

    float min_height = 0.0f;
    float max_height = 128.0f;

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
    int height = this->_height_noise->GenSingle2D(x, y, this->_SEED);

    // LOG_INFO("Height: {}", height);

    return height;
}

// NOTE: Meaning we simply skip (or replace with empty block)
bool Generator::is_cave(int x, int y, int z) {
    float noise = this->_cave_noise->GenSingle3D(x, y, z, this->_SEED);

    constexpr float iso = 0.15f;
    constexpr float width = 0.08f;

    return std::abs(noise - iso) < width;
}

} // namespace engine::world
