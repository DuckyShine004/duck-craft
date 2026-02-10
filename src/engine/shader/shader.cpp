#include <glm/gtc/type_ptr.hpp>

#include "external/glad/glad.h"

#include "engine/shader/shader.hpp"

#include "utility/file_utility.hpp"

#include "logger/logger_macros.hpp"

using namespace utility;

namespace engine::shader {

Shader::Shader() = default;

Shader::Shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path) : _vertex_shader_path(vertex_shader_path), _fragment_shader_path(fragment_shader_path) {
    this->initialise();
}

void Shader::initialise() {
    this->create_shaders();

    this->compile_shaders();

    this->attach_shaders();

    this->link_program();

    this->delete_shaders();
}

void Shader::use() {
    glUseProgram(this->_program);
}

void Shader::detach() {
    glUseProgram(0);
}

void Shader::create_shaders() {
    std::string vertex_shader_source = FileUtility::get_file_to_string(this->_vertex_shader_path);
    std::string fragment_shader_source = FileUtility::get_file_to_string(this->_fragment_shader_path);

    const char *vertex_shader_code = vertex_shader_source.c_str();
    const char *fragment_shader_code = fragment_shader_source.c_str();

    this->_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    this->_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(this->_vertex_shader, 1, &vertex_shader_code, nullptr);
    glShaderSource(this->_fragment_shader, 1, &fragment_shader_code, nullptr);
}

void Shader::compile_shaders() {
    glCompileShader(this->_vertex_shader);

    this->check_compilation_status(this->_vertex_shader);

    glCompileShader(this->_fragment_shader);

    this->check_compilation_status(this->_fragment_shader);
}

void Shader::attach_shaders() {
    this->_program = glCreateProgram();

    glAttachShader(this->_program, this->_vertex_shader);
    glAttachShader(this->_program, this->_fragment_shader);
}

void Shader::link_program() {
    glLinkProgram(this->_program);

    this->check_program_link_status();
}

void Shader::delete_shaders() {
    glDeleteShader(this->_vertex_shader);
    glDeleteShader(this->_fragment_shader);
}

void Shader::check_compilation_status(GLuint shader) {
    GLint status;

    GLchar info_log[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        glGetShaderInfoLog(shader, 512, nullptr, info_log);

        LOG_ERROR("Compile Error: \n {}", info_log);
    }
}

void Shader::check_program_link_status() {
    GLint status;

    GLchar info_log[512];

    glGetProgramiv(this->_program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        glGetProgramInfoLog(this->_program, 512, nullptr, info_log);

        LOG_ERROR("Link Error: \n {}", info_log);
    }
}

void Shader::set_float(const GLchar *name, float value) {
    GLint location = glGetUniformLocation(this->_program, name);

    glUniform1f(location, value);
}

void Shader::set_integer(const GLchar *name, int value) {
    GLint location = glGetUniformLocation(this->_program, name);

    glUniform1i(location, value);
}

void Shader::set_vector2f(const GLchar *name, float x, float y) {
    GLint location = glGetUniformLocation(this->_program, name);

    glUniform2f(location, x, y);
}

void Shader::set_vector2f(const GLchar *name, glm::vec2 vector) {
    GLint location = glGetUniformLocation(this->_program, name);

    glUniform2f(location, vector.x, vector.y);
}

void Shader::set_vector3f(const GLchar *name, float x, float y, float z) {
    GLint location = glGetUniformLocation(this->_program, name);

    glUniform3f(location, x, y, z);
}

void Shader::set_vector3f(const GLchar *name, glm::vec3 vector) {
    GLint location = glGetUniformLocation(this->_program, name);

    glUniform3f(location, vector.x, vector.y, vector.z);
}

void Shader::set_vector3f(const GLchar *name, const float (&array)[3]) {
    GLint location = glGetUniformLocation(this->_program, name);

    glUniform3f(location, array[0], array[1], array[2]);
}

void Shader::set_matrix4fv(const GLchar *name, glm::mat4 matrix) {
    GLint location = glGetUniformLocation(this->_program, name);

    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

} // namespace engine::shader
