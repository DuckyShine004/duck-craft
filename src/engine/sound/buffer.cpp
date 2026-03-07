#include <AL/alext.h>

#include "engine/sound/buffer.hpp"

#include "logger/logger_macros.hpp"

namespace engine::sound {

Buffer::Buffer() {
}

Buffer::~Buffer() {
}

void Buffer::add_sound(const char *path) {
    SNDFILE *sound_file;

    SF_INFO sound_file_info;

    if (!this->set_sound_file(path, sound_file, sound_file_info)) {
        return;
    }

    ALenum format;

    if (!this->set_sound_format(format, sound_file, sound_file_info)) {
        return;
    }

    float *memory_buffer = this->get_memory_buffer(sound_file_info);

    sf_count_t number_of_frames;

    if (!this->set_number_of_frames(number_of_frames, sound_file, sound_file_info, memory_buffer)) {
        return;
    }

    if (!this->set_sound_buffer(sound_file, sound_file_info, format, number_of_frames, memory_buffer)) {
        return;
    }

    LOG_INFO("Sound added to buffer successfully: {}", path);
}

bool Buffer::set_sound_file(const char *path, SNDFILE *&sound_file, SF_INFO &sound_file_info) {
    sound_file = sf_open(path, SFM_READ, &sound_file_info);

    if (sound_file == nullptr) {
        LOG_ERROR("Could not open audio in {}: {}", path, sf_strerror(sound_file));

        return false;
    }

    auto max_frames = INT_MAX / (sizeof(float) * sound_file_info.channels);

    if (sound_file_info.frames < 1 || sound_file_info.frames > max_frames) {
        LOG_ERROR("Bad sample count in {} {}", path, sound_file_info.frames);

        sf_close(sound_file);

        return false;
    }

    return true;
}

bool Buffer::set_sound_format(ALenum &format, SNDFILE *sound_file, const SF_INFO &sound_file_info) {
    switch (sound_file_info.channels) {
        case 1:
            format = AL_FORMAT_MONO_FLOAT32;

            return true;
        case 2:
            format = AL_FORMAT_STEREO_FLOAT32;

            return true;
        case 3:
            if (sf_command(sound_file, SFC_WAVEX_GET_AMBISONIC, nullptr, 0) == SF_AMBISONIC_B_FORMAT) {
                format = AL_FORMAT_BFORMAT2D_FLOAT32;

                return true;
            }

            break;
        case 4:
            if (sf_command(sound_file, SFC_WAVEX_GET_AMBISONIC, nullptr, 0) == SF_AMBISONIC_B_FORMAT) {
                format = AL_FORMAT_BFORMAT3D_FLOAT32;

                return true;
            }

            break;
    }

    format = AL_NONE;

    LOG_ERROR("Unsupported channel count: {}", sound_file_info.channels);

    sf_close(sound_file);

    return false;
}

float *Buffer::get_memory_buffer(const SF_INFO &sound_file_info) {
    size_t memoryBufferSize = (size_t)(sound_file_info.frames * sound_file_info.channels) * sizeof(float);

    return static_cast<float *>(malloc(memoryBufferSize));
}

bool Buffer::set_sound_buffer(SNDFILE *sound_file, const SF_INFO &sound_file_info, ALenum &format, sf_count_t &number_of_frames, float *memory_buffer) {
    ALsizei bytes = (ALsizei)(number_of_frames * sound_file_info.channels) * (ALsizei)sizeof(float);

    this->id = 0;

    alGenBuffers(1, &this->id);
    alBufferData(this->id, format, memory_buffer, bytes, sound_file_info.samplerate);

    if (this->is_openal_error()) {
        if (this->id && alIsBuffer(this->id)) {
            alDeleteBuffers(1, &this->id);
        }

        return false;
    }

    free(memory_buffer);

    sf_close(sound_file);

    return true;
}

bool Buffer::set_number_of_frames(sf_count_t &number_of_frames, SNDFILE *sound_file, const SF_INFO &sound_file_info, float *memory_buffer) {
    number_of_frames = sf_readf_float(sound_file, memory_buffer, sound_file_info.frames);

    if (number_of_frames < 1) {
        free(memory_buffer);

        sf_close(sound_file);

        LOG_ERROR("Failed to read sound file frames");

        return false;
    }

    return true;
}

bool Buffer::is_openal_error() {
    ALenum error = alGetError();

    if (error != AL_NO_ERROR) {
        LOG_ERROR("OpenAL Error: {}", this->get_openal_error(error));
        return true;
    }

    return false;
}

const char *Buffer::get_openal_error(ALenum error) {
    switch (error) {
        case AL_NO_ERROR:
            return "AL_NO_ERROR";
        case AL_INVALID_NAME:
            return "AL_INVALID_NAME";
        case AL_INVALID_ENUM:
            return "AL_INVALID_ENUM";
        case AL_INVALID_VALUE:
            return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION:
            return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY:
            return "AL_OUT_OF_MEMORY";
        default:
            return "UNKNOWN_OPENAL_ERROR";
    }
}

} // namespace engine::sound
