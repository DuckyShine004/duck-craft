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

std::string StringUtility::slice_string(std::string &string, int start, int end) {
    int length = end - start + 1;

    return string.substr(start, length);
}

std::vector<std::string> StringUtility::split_string(std::string &string, const char &delimiter) {
    std::vector<std::string> tokens;

    std::stringstream buffer(string);

    std::string token;

    while (std::getline(buffer, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

} // namespace utility
