#pragma once

#include <string>

namespace utility {

class DatetimeUtility {
  public:
    static std::string get_datetime(const std::string &pattern);
};

} // namespace utility
