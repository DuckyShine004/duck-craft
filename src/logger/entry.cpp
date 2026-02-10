#include <sstream>

#include "logger/entry.hpp"

#include "utility/file_utility.hpp"
#include "utility/colour_utility.hpp"
#include "utility/datetime_utility.hpp"

using namespace utility;

namespace logger {

Entry::Entry(Severity severity, const char *file, const char *function, int line, std::string message) : _severity(severity), _file(file), _function(function), _line(line), _message(message) {
    int severityLevel = static_cast<int>(severity);

    this->_severityName = _NAMES[severityLevel];

    this->_backgroundColour = ColourUtility::get_ansi_background_from_hex(_BACKGROUND_COLOURS[severityLevel]);

    this->_textColour = ColourUtility::get_ansi_foreground_from_hex(_TEXT_COLOURS[severityLevel]);

    this->_date = DatetimeUtility::get_datetime("%A %d %Y");

    this->_time = DatetimeUtility::get_datetime("%H:%M:%S");

    this->_filename = FileUtility::get_filename_from_path(file);

    this->_severityLevel = severityLevel;

    this->to_json();
}

nlohmann::json Entry::get_json() {
    return this->_json;
}

std::string Entry::to_string() {
    std::ostringstream oss;

    oss << "\033[1m" << this->_backgroundColour << this->_textColour << "[" << this->_severityName << "]" << "\033[0m";

    oss << " [" << this->_date << " ~ " << this->_time << "]";

    oss << " [" << this->_filename << " | " << this->_function << ":" << this->_line << "]\n";

    oss << "  " << this->_message;

    return oss.str();
}

void Entry::to_json() {
    this->_json["severity"] = this->_severityName;
    this->_json["level"] = this->_severityLevel;

    this->_json["date"] = this->_date;
    this->_json["time"] = this->_time;

    this->_json["file"] = this->_file;
    this->_json["function"] = this->_function;
    this->_json["line"] = this->_line;

    this->_json["message"] = this->_message;
}

} // namespace logger
