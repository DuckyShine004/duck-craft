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

    GLuint generate_texture_handle(const std::string &handle_name);

    void load_texture(std::string &texture_path, int texture_index, GLuint texture_handle);

    void load_block_textures();

    void sort_block_directory_by_type();
};

} // namespace manager
