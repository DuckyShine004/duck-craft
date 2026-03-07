#pragma once

#include <random>

#include <glm/glm.hpp>

#include <stdexcept>

namespace utility {

class MathUtility {
  public:
    static glm::vec3 get_random_vector3f(float lower, float upper);

    static float get_random_float(float lower, float upper);

    static bool get_random_bool();

    template <typename M> static const typename M::key_type &get_random_key_from_map(const M &container) {
        if (container.empty()) {
            throw std::runtime_error("Container is empty");
        }

        std::uniform_int_distribution<int> distribution(0, container.size() - 1);

        int random_index = distribution(MathUtility::_RNG);

        auto iterator = container.begin();

        std::advance(iterator, random_index);

        return iterator->first;
    };

  private:
    static inline std::mt19937 _RNG{std::random_device{}()};
};

} // namespace utility
