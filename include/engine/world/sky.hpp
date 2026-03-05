#pragma once

#include "engine/shader/shader.hpp"

namespace engine::world {

class Sky {
  public:
    Sky();

    void render(engine::shader::Shader &shader);

  private:
    GLuint _vao;
};

} // namespace engine::world
