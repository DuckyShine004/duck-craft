#pragma once

#include <memory>

#include "manager/manager.hpp"

#include "engine/shader/shader.hpp"

namespace manager {

class ShaderManager final : public Manager {
  public:
    static ShaderManager &get_instance();

    void initialise();

    void use_shader(const std::string &name);

    engine::shader::Shader &get_shader(const std::string &name);

    engine::shader::Shader &get_active_shader();

  private:
    static inline constexpr const char *_DIRECTORY = "resources/shaders/";

    static inline constexpr const char *_VERTEX_SHADER_EXTENSION = ".vert";
    static inline constexpr const char *_FRAGMENT_SHADER_EXTENSION = ".frag";

    static inline constexpr const char *_INCLUDE_EXTENSION = ".glsl";

    std::unordered_map<std::string, std::unique_ptr<engine::shader::Shader>> _shaders;

    engine::shader::Shader *_shader;

    ShaderManager();

    ~ShaderManager();

    void add_shader(const std::string &path);
};

}; // namespace manager
