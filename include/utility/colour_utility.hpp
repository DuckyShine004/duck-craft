#pragma once

#include <array>
#include <string>

#include <glm/glm.hpp>

namespace utility {

class ColourUtility {
  public:
    static std::string get_ansi_foreground_from_hex(const std::string &hex);

    static std::string get_ansi_background_from_hex(const std::string &hex);

    static glm::vec3 get_high_precision_RGB(const float (&array)[3]);

  private:
    static std::array<int, 3> get_RGB_from_hex(const std::string &hex);
};

} // namespace utility
