#pragma once

#include <string>
#include <unordered_map>

#include "external/glad/glad.h"

#include "manager/manager.hpp"

namespace manager {

class TextureManager final : public Manager {
  public:
    static TextureManager &get_instance();

    void initialise();

    GLuint get_texture_handle(const std::string &handle_name);

  private:
    static inline constexpr const char *_PARENT_DIRECTORY = "resources/textures/";

    std::unordered_map<std::string, GLuint> _texture_handles;

    TextureManager();

    ~TextureManager();

    GLuint generate_texture_array_and_get_id(const std::string &handle_name, int width, int height, int layers);

    void load_texture(std::string &texture_path, int texture_index, GLuint texture_handle);

    void load_block_textures();

    void load_environment_textures();

    void sort_block_directory_by_type();

    std::unordered_map<std::string, int> get_index(const std::string &parent_directory);
};

} // namespace manager
