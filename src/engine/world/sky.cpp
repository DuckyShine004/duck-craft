#include "engine/world/sky.hpp"

#include "engine/shader/shader.hpp"

using namespace engine::shader;

namespace engine::world {

Sky::Sky() : _vao(0) {
    glGenVertexArrays(1, &this->_vao);
}

void Sky::render(Shader &shader) {
    glBindVertexArray(this->_vao);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);
}

} // namespace engine::world
