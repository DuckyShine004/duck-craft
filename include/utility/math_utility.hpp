#pragma once

#include <random>

#include <glm/glm.hpp>

namespace utility {

class MathUtility {
  public:
    static glm::vec3 get_random_vector3f(float lower, float upper);

    static float get_random_float(float lower, float upper);

    static bool get_random_bool();

  private:
    static inline std::mt19937 _RNG{std::random_device{}()};
};

} // namespace utility
