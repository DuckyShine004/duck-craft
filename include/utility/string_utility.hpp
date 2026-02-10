#pragma once

#include <glm/glm.hpp>

#include <string>

namespace utility {

class StringUtility {
  public:
    static std::string to_string(glm::vec3 &vector);

    static std::string to_upper(std::string &string);
};

} // namespace utility
