#define STB_IMAGE_IMPLEMENTATION

#include <iostream>

#include "external/stb/stb_image.h"

#include "external/magic_enum/magic_enum.hpp"

#include "engine/world/face_type.hpp"
#include "engine/world/block_type.hpp"

#include "manager/texture_manager.hpp"

#include "utility/file_utility.hpp"
#include "utility/string_utility.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::world;

using namespace utility;

namespace manager {

TextureManager::TextureManager() = default;

TextureManager::~TextureManager() = default;

TextureManager &TextureManager::get_instance() {
    static TextureManager instance;

    return instance;
}

void TextureManager::initialise() {
    stbi_set_flip_vertically_on_load(true);

    this->load_block_textures();
}

GLuint TextureManager::get_texture_handle(const std::string &handle_name) {
    auto iterator = this->_texture_handles.find(handle_name);

    if (iterator == this->_texture_handles.end()) {
        throw std::runtime_error(fmt::format("Texture handle not found for handle name: {}", handle_name));
    }

    return iterator->second;
}

GLuint TextureManager::generate_texture_handle(const std::string &handle_name) {
    GLuint texture_id;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // TODO: 1x6 layer for now
    int layers = 6;

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 16, 16, layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    this->_texture_handles[handle_name] = texture_id;

    return texture_id;
}

void TextureManager::load_texture(std::string &texture_path, int texture_index, GLuint texture_handle) {
    LOG_INFO("Texture path: {}", texture_path);

    int width;
    int height;
    int channels;

    unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (data == nullptr) {
        LOG_ERROR("Failed to load image: {}", stbi_failure_reason());
        return;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_handle);

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture_index, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
}

void TextureManager::load_block_textures() {
    GLuint texture_handle = this->generate_texture_handle("blocks");

    std::string directory = std::string(this->_PARENT_DIRECTORY) + "blocks";

    std::vector<std::string> block_directories = FileUtility::get_paths_in_directory(directory);

    std::sort(block_directories.begin(), block_directories.end(), [](const std::string &a, const std::string &b) -> bool {
        std::string a_basename = FileUtility::get_basename_from_path(a);
        std::string b_basename = FileUtility::get_basename_from_path(b);

        std::string a_enum_name = StringUtility::to_upper(a_basename);
        std::string b_enum_name = StringUtility::to_upper(b_basename);

        BlockType a_block_type = magic_enum::enum_cast<BlockType>(a_enum_name).value();
        BlockType b_block_type = magic_enum::enum_cast<BlockType>(b_enum_name).value();

        int a_value = static_cast<int>(a_block_type);
        int b_value = static_cast<int>(b_block_type);

        return a_value < b_value;
    });

    int layer = 0;

    for (std::string &block_directory : block_directories) {
        std::vector<std::string> texture_paths = FileUtility::get_paths_in_directory(block_directory);

        std::sort(texture_paths.begin(), texture_paths.end(), [](const std::string &a, const std::string &b) -> bool {
            std::string a_basename = FileUtility::get_basename_from_path(a);
            std::string b_basename = FileUtility::get_basename_from_path(b);

            std::string a_enum_name = StringUtility::to_upper(a_basename);
            std::string b_enum_name = StringUtility::to_upper(b_basename);

            FaceType a_face_type = magic_enum::enum_cast<FaceType>(a_enum_name).value();
            FaceType b_face_type = magic_enum::enum_cast<FaceType>(b_enum_name).value();

            int a_value = static_cast<int>(a_face_type);
            int b_value = static_cast<int>(b_face_type);

            return a_value < b_value;
        });

        for (std::string &texture_path : texture_paths) {
            this->load_texture(texture_path, layer, texture_handle);

            ++layer;
        }
    }
}

} // namespace manager
