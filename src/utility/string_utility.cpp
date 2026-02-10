#include <sstream>

#include "utility/string_utility.hpp"

namespace utility {

std::string StringUtility::to_string(glm::vec3 &vector) {
    std::ostringstream oss;

    oss << '(';

    return oss.str();
}

std::string StringUtility::to_upper(std::string &string) {
    std::string upper = "";

    for (char &c : string) {
        upper += std::toupper(c);
    }

    return upper;
}

} // namespace utility
