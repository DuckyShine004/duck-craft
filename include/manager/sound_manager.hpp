#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <sndfile.h>

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "manager/manager.hpp"

#include "engine/sound/buffer.hpp"
#include "engine/sound/source.hpp"

namespace manager {

class SoundManager final : public Manager {
  public:
    static SoundManager &get_instance();

    void initialise();

    void add_sound(const std::string &type, const std::string &path);

    void play(const std::string &type, const std::string &name);

    void play_music();

  private:
    static inline constexpr const char *_PARENT_DIRECTORY = "resources/sounds/";

    static inline constexpr int _MAX_SOURCES = 128;

    ALCdevice *_device;

    ALCcontext *_context;

    std::unique_ptr<engine::sound::Source> _music_source;

    std::unordered_map<std::string, std::unordered_map<std::string, engine::sound::Buffer>> _sounds;

    std::vector<engine::sound::Source> _sources;

    SoundManager();

    ~SoundManager();

    void initialise_music();
};

} // namespace manager
