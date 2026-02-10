#include <ctime>
#include <iomanip>
#include <sstream>

#include "utility/datetime_utility.hpp"

namespace utility {

std::string DatetimeUtility::get_datetime(const std::string &pattern) {
    std::time_t now = std::time(nullptr);

    std::tm time{};

    localtime_r(&now, &time);

    std::ostringstream datetime;

    datetime << std::put_time(&time, pattern.c_str());

    return datetime.str();
}

} // namespace utility
