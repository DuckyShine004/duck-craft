#include "engine/world/block.hpp"

namespace engine::world {

Block::Block() : _type(BlockType::EMPTY), _face_mask(this->_ALL_FACES) {
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

} // namespace engine::world
