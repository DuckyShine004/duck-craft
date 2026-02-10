#include "utility/file_utility.hpp"

#include <stack>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace utility {

std::string FileUtility::get_file_to_string(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}

std::string FileUtility::get_filename_from_path(const std::string &path) {
    std::filesystem::path full_path(path);

    std::string filename = full_path.filename().string();

    return filename;
}

std::string FileUtility::get_basename_from_path(const std::string &path) {
    std::filesystem::path full_path(path);

    std::string basename = full_path.stem().string();

    return basename;
}

std::string FileUtility::get_parent_directory(const std::string &path) {
    std::filesystem::path full_path(path);

    std::string parent_directory = full_path.parent_path().string();

    return parent_directory;
}

bool FileUtility::path_exists(const std::string &path) {
    return std::filesystem::exists(path);
}

void FileUtility::create_file(const std::string &path) {
    if (FileUtility::path_exists(path)) {
        return;
    }

    std::ofstream(path).close();
}

void FileUtility::load_json(nlohmann::json &json, const std::string &path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        return;
    }

    file >> json;
}

void FileUtility::save_json(const nlohmann::json &json, const std::string &path) {
    std::ofstream file(path);

    if (!file.is_open()) {
        return;
    }

    file << std::setw(2) << json;
}

// Assume starts with name/
std::vector<std::string> FileUtility::get_files_in_directory(std::string directory) {
    std::vector<std::string> files;

    std::stack<std::string> directories;

    directories.push(directory);

    while (!directories.empty()) {
        std::string directory = directories.top();

        directories.pop();

        if (!FileUtility::path_exists(directory)) {
            continue;
        }

        std::vector<std::string> paths_in_directory = FileUtility::get_paths_in_directory(directory);

        for (std::string path : paths_in_directory) {
            if (FileUtility::is_file(path)) {
                files.push_back(path);
            } else if (FileUtility::is_directory(path)) {
                directories.push(path);
            }
        }
    }

    return files;
}

std::vector<std::string> FileUtility::get_paths_in_directory(std::string directory) {
    std::vector<std::string> paths;

    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
        paths.push_back(entry.path().string());
    }

    return paths;
}

bool FileUtility::is_file(const std::string &path) {
    return std::filesystem::is_regular_file(path);
}

bool FileUtility::is_directory(const std::string &path) {
    return std::filesystem::is_directory(path);
}

} // namespace utility
