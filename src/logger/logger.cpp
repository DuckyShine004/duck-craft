#include <iostream>

#include "logger/logger.hpp"

#include "utility/file_utility.hpp"
#include "utility/datetime_utility.hpp"

using namespace utility;

namespace logger {

Logger::Logger() {
    this->initialise();
}

Logger::~Logger() = default;

Logger &Logger::get_instance() {
    static Logger *instance = new Logger();

    return *instance;
}

void Logger::initialise() {
    std::string filename = DatetimeUtility::get_datetime("%d-%m-%Y_%H-%M-%S") + ".log";

    this->_log_path = ".cache/logs/" + filename;

    this->_json = nlohmann::json::array();

    FileUtility::create_file(this->_log_path);
}

void Logger::log(Severity severity, const char *file, const char *function, int line, std::string message) {
    Entry entry(severity, file, function, line, message);

    this->add_entry(entry);

    std::cout << entry.to_string() << std::endl;
}

void Logger::add_entry(Entry entry) {
    this->_entries.push_back(entry);

    this->_json.push_back(entry.get_json());

    this->save();
}

void Logger::save() {
    FileUtility::save_json(this->_json, this->_log_path);
}

} // namespace logger
