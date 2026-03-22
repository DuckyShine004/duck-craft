#define STB_IMAGE_IMPLEMENTATION

#include <fstream>

#include "external/stb/stb_image.h"

#include "external/magic_enum/magic_enum.hpp"

#include "engine/world/block_metadata.hpp"

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
    this->load_environment_textures();
}

GLuint TextureManager::get_texture_handle(const std::string &handle_name) {
    auto iterator = this->_texture_handles.find(handle_name);

    if (iterator == this->_texture_handles.end()) {
        throw std::runtime_error(fmt::format("Texture handle not found for handle name: {}", handle_name));
    }

    return iterator->second;
}

GLuint TextureManager::generate_texture_array_and_get_id(const std::string &handle_name, int width, int height, int layers) {
    GLuint texture_id;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    this->_texture_handles[handle_name] = texture_id;

    return texture_id;
}

void TextureManager::load_texture(std::string &texture_path, int texture_index, GLuint texture_handle) {
    LOG_INFO("Texture path: {}, ID: {}", texture_path, texture_index);

    int width;
    int height;
    int channels;

    unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (data == nullptr) {
        LOG_ERROR("Failed to load image: {}", stbi_failure_reason());
        return;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_handle);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture_index, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
}

void TextureManager::load_block_textures() {
    const int WIDTH = 16;
    const int HEIGHT = 16;

    std::string directory = std::string(this->_PARENT_DIRECTORY) + "block";

    std::regex pattern("[^\\s]+(?:\\.(?:png))$");

    int layers = FileUtility::get_number_of_files_in_directory(directory, pattern);

    GLuint texture_handle = this->generate_texture_array_and_get_id("block", WIDTH, HEIGHT, layers);

    std::unordered_map<std::string, int> index = this->get_index(directory);

    std::vector<std::string> texture_paths = FileUtility::get_paths_in_directory(directory);

    std::sort(texture_paths.begin(), texture_paths.end(), [&](const std::string &a, const std::string &b) -> bool {
        std::string a_basename = FileUtility::get_basename_from_path(a);
        std::string b_basename = FileUtility::get_basename_from_path(b);

        return index[a_basename] < index[b_basename];
    });

    int layer = 0;

    for (std::string &texture_path : texture_paths) {
        if (std::regex_match(texture_path, pattern)) {
            this->load_texture(texture_path, layer, texture_handle);

            ++layer;
        }
    }

    nlohmann::json metadata;

    FileUtility::load_json(metadata, directory + "/metadata.json");

    for (auto property : metadata) {
        int id = property.at("id").get<int>();

        BlockMetadata &block_metadata = BLOCK_METADATA[id];

        std::vector<std::string> flags = property.at("flags").get<std::vector<std::string>>();

        for (std::string &flag : flags) {
            block_metadata.flags |= magic_enum::enum_cast<BlockFlag>(StringUtility::to_upper(flag)).value();
        }

        std::string name = property.at("name").get<std::string>();

        if (name == "empty") {
            continue;
        }

        std::vector<std::string> textures = property.at("textures").get<std::vector<std::string>>();

        if (textures[0] == "all") {
            std::fill(std::begin(block_metadata.texture_ids), std::end(block_metadata.texture_ids), index[name]);
        } else {
            for (int i = 0; i < 6; ++i) {
                std::string texture_name = name + '_' + textures[i];

                block_metadata.texture_ids[i] = index[texture_name];
            }
        }
    }
}

void TextureManager::load_environment_textures() {
    const int WIDTH = 256;
    const int HEIGHT = 256;

    const int LAYERS = 1;

    GLuint texture_handle = this->generate_texture_array_and_get_id("environment", WIDTH, HEIGHT, LAYERS);

    std::string directory = std::string(this->_PARENT_DIRECTORY) + "environment";

    std::vector<std::string> environment_paths = FileUtility::get_paths_in_directory(directory);

    int layer = 0;

    for (std::string &environment_path : environment_paths) {
        this->load_texture(environment_path, layer, texture_handle);

        ++layer;
    }
}

std::unordered_map<std::string, int> TextureManager::get_index(const std::string &directory) {
    std::string path = directory + '/' + "index.txt";

    std::ifstream file(path, std::ios::binary);

    std::unordered_map<std::string, int> index;

    std::string token;

    std::stringstream buffer;

    int order = 0;

    while (std::getline(file, token)) {
        std::vector<std::string> textures = StringUtility::split_string(token, ' ');

        std::string &parent_texture_name = textures[0];

        std::size_t texture_count = textures.size();

        LOG_INFO("Parent texture name: {}", parent_texture_name);
        if (texture_count == 1) {
            index[parent_texture_name] = order++;

            continue;
        }

        for (int i = 1; i < texture_count; ++i) {
            std::string texture_name = parent_texture_name + '_' + textures[i];

            LOG_INFO("Texture name: {}", texture_name);

            index[texture_name] = order++;
        }
    }

    return index;
}

} // namespace manager
