#include "engine/world/generator.hpp"

namespace engine::world {

Generator::Generator() {
    FastNoise::SmartNode<FastNoise::Simplex> simplex = FastNoise::New<FastNoise::Simplex>();

    simplex->SetOutputMin(this->_HEIGHT_LIMIT.first);
    simplex->SetOutputMax(this->_HEIGHT_LIMIT.second);

    this->_fractal = FastNoise::New<FastNoise::FractalFBm>();

    this->_fractal->SetSource(simplex);

    this->_fractal->SetOctaveCount(this->_OCTAVES);
}

int Generator::get_height(int x, int y) {
    return this->_fractal->GenSingle2D(x, y, this->_SEED);
}

} // namespace engine::world
