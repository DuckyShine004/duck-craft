#include "engine/world/generator.hpp"

namespace engine::world {

Generator::Generator() {
    this->initialise_height_noise();

    this->initialise_cave_noise();
}

void Generator::initialise_height_noise() {
    FastNoise::SmartNode<FastNoise::Simplex> simplex = FastNoise::New<FastNoise::Simplex>();

    simplex->SetOutputMin(this->_HEIGHT_LIMIT.first);
    simplex->SetOutputMax(this->_HEIGHT_LIMIT.second);

    this->_height_noise = FastNoise::New<FastNoise::FractalFBm>();

    this->_height_noise->SetSource(simplex);

    this->_height_noise->SetOctaveCount(this->_OCTAVES);
}

void Generator::initialise_cave_noise() {
    FastNoise::SmartNode<FastNoise::Simplex> simplex = FastNoise::New<FastNoise::Simplex>();

    this->_cave_noise = FastNoise::New<FastNoise::FractalFBm>();

    this->_cave_noise->SetSource(simplex);

    this->_cave_noise->SetOctaveCount(this->_OCTAVES);
}

int Generator::get_height(int x, int y) {
    return this->_height_noise->GenSingle2D(x, y, this->_SEED);
}

float Generator::get_cave_noise(int x, int y, int z) {
    float noise = this->_cave_noise->GenSingle3D(x, y, z, this->_SEED);

    return (noise + 1.0f) / 2.0f;
}

} // namespace engine::world
