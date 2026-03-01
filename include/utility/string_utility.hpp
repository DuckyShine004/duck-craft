#pragma once

#include <glm/glm.hpp>

#include <string>

namespace utility {

class StringUtility {
  public:
    static std::string to_string(glm::vec3 &vector);

    static std::string to_upper(std::string &string);

    static std::string slice_string(std::string &string, int start, int end);

    static std::vector<std::string> split_string(std::string &string, const char &delimiter);
};

} // namespace utility
