
#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <sndfile.h>

namespace engine::sound {

class Buffer {
  public:
    ALuint id;

    Buffer();

    ~Buffer();

    void initialise();

    void add_sound(const char *path);

  private:
    bool set_sound_file(const char *path, SNDFILE *&sound_file, SF_INFO &sound_file_info);

    bool set_sound_format(ALenum &format, SNDFILE *sound_file, const SF_INFO &sound_file_info);

    float *get_memory_buffer(const SF_INFO &sound_file_info);

    bool set_number_of_frames(sf_count_t &number_of_frames, SNDFILE *sound_file, const SF_INFO &sound_file_info, float *memory_buffer);

    bool set_sound_buffer(SNDFILE *sound_file, const SF_INFO &sound_file_info, ALenum &format, sf_count_t &number_of_frames, float *memory_buffer);

    bool is_openal_error();

    const char *get_openal_error(ALenum error);
};

} // namespace engine::sound
