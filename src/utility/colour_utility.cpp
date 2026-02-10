#include "utility/colour_utility.hpp"

namespace utility {

std::string ColourUtility::get_ansi_foreground_from_hex(const std::string &hex) {
    std::array<int, 3> rgb = ColourUtility::get_RGB_from_hex(hex);

    char buffer[32];

    std::snprintf(buffer, sizeof(buffer), "\033[38;2;%d;%d;%dm", rgb[0], rgb[1], rgb[2]);

    return std::string(buffer);
}

std::string ColourUtility::get_ansi_background_from_hex(const std::string &hex) {
    std::array<int, 3> rgb = ColourUtility::get_RGB_from_hex(hex);

    char buffer[32];

    std::snprintf(buffer, sizeof(buffer), "\033[48;2;%d;%d;%dm", rgb[0], rgb[1], rgb[2]);

    return std::string(buffer);
}

glm::vec3 ColourUtility::get_high_precision_RGB(const float (&array)[3]) {
    const float R = array[0] / 255.0f;
    const float G = array[1] / 255.0f;
    const float B = array[2] / 255.0f;

    return glm::vec3(R, G, B);
}

std::array<int, 3> ColourUtility::get_RGB_from_hex(const std::string &hex) {
    std::string digits = hex.substr(1);

    std::array<int, 3> rgb;

    rgb[0] = std::stoi(digits.substr(0, 2), nullptr, 16);
    rgb[1] = std::stoi(digits.substr(2, 2), nullptr, 16);
    rgb[2] = std::stoi(digits.substr(4, 2), nullptr, 16);

    return rgb;
}

} // namespace utility
