#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace utility {

class FileUtility {
  public:
    static std::string get_file_to_string(const std::string &filename);

    static std::string get_filename_from_path(const std::string &path);

    static std::string get_basename_from_path(const std::string &path);

    static std::string get_parent_directory(const std::string &path);

    static bool path_exists(const std::string &path);

    static void create_file(const std::string &path);

    static void load_json(nlohmann::json &json, const std::string &path);

    static void save_json(const nlohmann::json &json, const std::string &path);

    static std::vector<std::string> get_files_in_directory(std::string directory);

    static std::vector<std::string> get_paths_in_directory(std::string directory);

    static bool is_file(const std::string &path);

    static bool is_directory(const std::string &path);
};

} // namespace utility
