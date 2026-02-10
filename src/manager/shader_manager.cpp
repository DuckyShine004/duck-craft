#include "manager/shader_manager.hpp"

#include "utility/file_utility.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::shader;

using namespace utility;

namespace manager {

ShaderManager::ShaderManager() : _shader(nullptr) {
}

ShaderManager::~ShaderManager() = default;

ShaderManager &ShaderManager::get_instance() {
    static ShaderManager instance;

    return instance;
}

void ShaderManager::initialise() {
    std::vector<std::string> paths = FileUtility::get_files_in_directory(this->_DIRECTORY);

    for (const std::string &path : paths) {
        this->add_shader(path);
    }
}

void ShaderManager::use_shader(const std::string &name) {
    auto iterator = this->_shaders.find(name);

    if (iterator == this->_shaders.end()) {
        LOG_ERROR("Shader '{}' not found", name);
        std::terminate();
    }

    this->_shader = iterator->second.get();

    this->_shader->use();
}

Shader &ShaderManager::get_shader(const std::string &name) {
    auto iterator = this->_shaders.find(name);

    if (iterator == this->_shaders.end()) {
        LOG_ERROR("Shader '{}' not found", name);
        std::terminate();
    }

    return *iterator->second;
}

Shader &ShaderManager::get_active_shader() {
    if (!this->_shader) {
        LOG_ERROR("No active shader is set. use_shader() must be called first");
        std::terminate();
    }

    return *this->_shader;
}

void ShaderManager::add_shader(const std::string &path) {
    std::string basename = FileUtility::get_basename_from_path(path);
    std::string parent_directory = FileUtility::get_parent_directory(path);
    std::string shader_path = parent_directory + '/' + basename;

    if (this->_shaders.find(basename) != this->_shaders.end()) {
        LOG_WARN("Shader '{}' is already loaded, skipping...", path);
        return;
    }

    std::string vertex_shader_path = shader_path + this->_VERTEX_SHADER_EXTENSION;
    std::string fragment_shader_path = shader_path + this->_FRAGMENT_SHADER_EXTENSION;

    std::unique_ptr<Shader> shader = std::make_unique<Shader>(vertex_shader_path, fragment_shader_path);

    this->_shaders.emplace(basename, std::move(shader));
}

}; // namespace manager
