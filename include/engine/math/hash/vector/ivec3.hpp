#pragma once

#include <glm/glm.hpp>

#include <boost/container_hash/hash.hpp>

namespace engine::math::hash::vector {

struct IVec3Hash {
    std::size_t operator()(const glm::ivec3 &vector) const noexcept {
        std::size_t seed = 0;

        boost::hash_combine(seed, vector.x);
        boost::hash_combine(seed, vector.y);
        boost::hash_combine(seed, vector.z);

        return seed;
    }
};

struct IVec3Equal {
    bool operator()(const glm::ivec3 &vector, const glm::ivec3 &other) const noexcept {
        return vector == other;
    }
};

} // namespace engine::math::hash::vector
