#pragma once

#include <string>
#include <vector>

#include "logger/entry.hpp"

namespace logger {

class Logger {
  public:
    Logger(const Logger &) = delete;

    Logger &operator=(const Logger &) = delete;

    static Logger &get_instance();

    void log(Severity severity, const char *file, const char *function, int line, std::string message);

    void add_entry(Entry entry);

    void save();

  private:
    Logger();

    ~Logger();

    void initialise();

    void run();

    nlohmann::json _json;

    std::vector<Entry> _entries;

    std::string _log_path;
};

} // namespace logger
