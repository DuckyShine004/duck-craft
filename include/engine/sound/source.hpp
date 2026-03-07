#pragma once

#include "AL/al.h"

namespace engine::sound {

class Source {
  public:
    Source();

    ~Source();

    void play(const ALuint buffer_id);

    float get_position();

    bool is_playing();

  private:
    static inline constexpr float _GAIN = 1.0f;
    static inline constexpr float _PITCH = 1.0f;

    static inline constexpr float _POSITION[3] = {0, 0, 0};
    static inline constexpr float _VELOCITY[3] = {0, 0, 0};

    static inline constexpr bool _IS_LOOPING = false;

    ALuint _source_id;
    ALuint _buffer_id;
};

} // namespace engine::sound
