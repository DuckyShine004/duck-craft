#include <random>

#include "utility/math_utility.hpp"

namespace utility {

glm::vec3 MathUtility::get_random_vector3f(float lower, float upper) {
    float x = MathUtility::get_random_float(lower, upper);
    float y = MathUtility::get_random_float(lower, upper);
    float z = MathUtility::get_random_float(lower, upper);

    return glm::vec3(x, y, z);
}

float MathUtility::get_random_float(float lower, float upper) {
    std::uniform_real_distribution<float> distribution(lower, upper);

    return distribution(MathUtility::_RNG);
}

bool MathUtility::get_random_bool() {
    std::bernoulli_distribution distribution(0.5f);

    return distribution(MathUtility::_RNG);
}

float MathUtility::get_distance2(float ax, float ay, float az, float bx, float by, float bz) {
    float dx = bx - ax;
    float dy = by - ay;
    float dz = bz - az;

    return dx * dx + dy * dy + dz * dz;
}

} // namespace utility
