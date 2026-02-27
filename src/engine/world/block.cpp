#include "engine/world/block.hpp"

namespace engine::world {

Block::Block() : _type(BlockType::EMPTY), _face_mask(this->_ALL_FACES), _sunlight(0U) {
}

void Block::set_type(const BlockType &type) {
    this->_type = type;
}

BlockType &Block::get_type() {
    return this->_type;
}

bool Block::is_face_active(const FaceType &type) {
    return this->_face_mask & (1U << static_cast<std::uint8_t>(type));
}

void Block::set_face_state(const FaceType &type, bool state) {
    if (state) {
        this->_face_mask |= (1U << static_cast<std::uint8_t>(type));
    } else {
        this->_face_mask &= ~(1U << static_cast<std::uint8_t>(type));
    }
}

void Block::set_face_state(int face_type_index, bool state) {
    if (state) {
        this->_face_mask |= (1U << face_type_index);
    } else {
        this->_face_mask &= ~(1U << face_type_index);
    }
}

void Block::set_sunlight(std::uint8_t sunlight) {
    this->_sunlight = sunlight;
}

std::uint8_t Block::get_sunlight() {
    return this->_sunlight;
}

} // namespace engine::world
