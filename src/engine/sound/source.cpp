#include "engine/sound/source.hpp"

namespace engine::sound {

Source::Source() : _source_id(0), _buffer_id(0) {
    alGenSources(1, &this->_source_id);

    alSourcef(this->_source_id, AL_GAIN, this->_GAIN);
    alSourcef(this->_source_id, AL_PITCH, this->_PITCH);

    alSource3f(this->_source_id, AL_POSITION, this->_POSITION[0], this->_POSITION[1], this->_POSITION[2]);
    alSource3f(this->_source_id, AL_VELOCITY, this->_VELOCITY[0], this->_VELOCITY[1], this->_VELOCITY[2]);

    alSourcei(this->_source_id, AL_LOOPING, this->_IS_LOOPING);
    alSourcei(this->_source_id, AL_BUFFER, this->_buffer_id);
}

Source::~Source() {
    alDeleteSources(1, &this->_source_id);
}

void Source::play(const ALuint buffer_id) {
    if (buffer_id != this->_buffer_id) {
        this->_buffer_id = buffer_id;

        alSourcei(this->_source_id, AL_BUFFER, (ALint)this->_buffer_id);
    }

    alSourcePlay(this->_source_id);
}

float Source::get_position() {
    ALfloat position = 0.0f;

    alGetSourcef(this->_source_id, AL_SEC_OFFSET, &position);

    return position * 1000.0f;
}

bool Source::is_playing() {
    ALint state;

    alGetSourcei(this->_source_id, AL_SOURCE_STATE, &state);

    return state == AL_PLAYING;
}

} // namespace engine::sound
