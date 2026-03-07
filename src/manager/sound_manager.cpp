#include <AL/alext.h>

#include "manager/sound_manager.hpp"

#include "utility/file_utility.hpp"
#include "utility/math_utility.hpp"

#include "logger/logger_macros.hpp"

using namespace engine::sound;

using namespace utility;

namespace manager {

SoundManager &SoundManager::get_instance() {
    static SoundManager instance;

    return instance;
}

SoundManager::SoundManager() {
    const ALCchar *device_name = alcGetString(0, ALC_DEFAULT_DEVICE_SPECIFIER);

    this->_device = alcOpenDevice(device_name);

    if (!this->_device) {
        throw("Failed to get sound device");
    }

    this->_context = alcCreateContext(this->_device, nullptr);

    if (!this->_context) {
        throw("Failed to set sound context");
    }

    ALCboolean is_context_current = alcMakeContextCurrent(this->_context);

    if (!is_context_current) {
        throw("Failed to make sound context current");
    }

    this->_music_source = std::make_unique<Source>();

    this->_sources.reserve(this->_MAX_SOURCES);

    for (int i = 0; i < this->_MAX_SOURCES; ++i) {
        this->_sources.emplace_back();
    }
}

SoundManager::~SoundManager() {
    alcMakeContextCurrent(nullptr);

    alcDestroyContext(this->_context);

    alcCloseDevice(this->_device);
}

void SoundManager::initialise() {
    this->initialise_music();
}

void SoundManager::initialise_music() {
    std::string directory = std::string(this->_PARENT_DIRECTORY) + "music";

    std::vector<std::string> music_paths = FileUtility::get_paths_in_directory(directory);

    for (std::string &music_path : music_paths) {
        this->add_sound("music", music_path);
    }
}

void SoundManager::add_sound(const std::string &type, const std::string &path) {
    if (!FileUtility::path_exists(path)) {
        LOG_ERROR("Sound file '{}' does not exist", path);
        return;
    }

    std::string basename = FileUtility::get_basename_from_path(path);

    LOG_INFO("Sound file: {}", path.c_str());

    Buffer &buffer = this->_sounds[type][basename];

    buffer.add_sound(path.c_str());
}

void SoundManager::play(const std::string &type, const std::string &name) {
    auto type_iterator = this->_sounds.find(type);

    if (type_iterator == this->_sounds.end()) {
        LOG_ERROR("Sound type '{}' not found", type);
        return;
    }

    std::unordered_map<std::string, Buffer> &buffers = type_iterator->second;

    auto buffer_iterator = buffers.find(name);

    if (buffer_iterator == buffers.end()) {
        LOG_ERROR("Sound name '{}' not found", name);
        return;
    }

    Buffer &buffer = buffer_iterator->second;

    bool found_source = false;

    for (int source_index = 0; source_index < this->_MAX_SOURCES && !found_source; ++source_index) {
        Source &source = this->_sources[source_index];

        if (!source.is_playing()) {
            source.play(buffer.id);

            found_source = true;
        }
    }

    if (!found_source) {
        LOG_WARN("No sources found. Skipping '{}'", buffer_iterator->first);
    }
}

void SoundManager::play_music() {
    if (this->_music_source->is_playing()) {
        return;
    }

    std::unordered_map<std::string, Buffer> &music_buffers = this->_sounds.at("music");

    const std::string &music_name = MathUtility::get_random_key_from_map(music_buffers);

    LOG_INFO("Now playing '{}'", music_name);

    Buffer &buffer = music_buffers.at(music_name);

    this->_music_source->play(buffer.id);
}

} // namespace manager
