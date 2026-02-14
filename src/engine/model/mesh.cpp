#include "engine/model/mesh.hpp"

#include "engine/model/vertex.hpp"

#include <cstddef>

using namespace engine::model;

namespace engine::model {

Mesh::Mesh() : _vao(0), _vbo(0), _ibo(0) {
}

Mesh::~Mesh() {
    // if (this->_vao) {
    //     glDeleteVertexArrays(1, &this->_vao);
    //
    //     this->_vao = 0;
    // }
    //
    // if (this->_vbo) {
    //     glDeleteBuffers(1, &this->_vbo);
    //
    //     this->_vbo = 0;
    // }
    //
    // if (this->_ibo) {
    //     glDeleteBuffers(1, &this->_ibo);
    //
    //     this->_ibo = 0;
    // }
}

void Mesh::upload() {
    if (this->_vao == 0) {
        glGenVertexArrays(1, &this->_vao);
        glGenBuffers(1, &this->_vbo);
        glGenBuffers(1, &this->_ibo);

        glBindVertexArray(this->_vao);

        glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_ibo);

        glEnableVertexAttribArray(this->_POSITION_ATTRIBUTE);
        glEnableVertexAttribArray(this->_NORMAL_ATTRIBUTE);
        glEnableVertexAttribArray(this->_UV_ATTRIBUTE);

        glVertexAttribPointer(this->_POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
        glVertexAttribPointer(this->_NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
        glVertexAttribPointer(this->_UV_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glBindVertexArray(this->_vao);

    glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
    glBufferData(GL_ARRAY_BUFFER, this->get_vertices_size(), this->_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->get_indices_size(), this->_indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::render(Topology topology) {
    switch (topology) {
        case Topology::TRIANGLE:
            draw_triangles();
            break;
        case Topology::LINE:
            draw_lines();
            break;
    }
}

void Mesh::draw_lines() {
    glBindVertexArray(this->_vao);
    glDrawElements(GL_LINES, this->_indices.size(), GL_UNSIGNED_INT, (void *)0);
    glBindVertexArray(0);
}

void Mesh::draw_triangles() {
    glBindVertexArray(this->_vao);
    glDrawElements(GL_TRIANGLES, this->_indices.size(), GL_UNSIGNED_INT, (void *)0);
    glBindVertexArray(0);
}

void Mesh::add_vertex(Vertex &vertex) {
    this->_vertices.push_back(vertex);
}

void Mesh::add_vertex(glm::vec3 &position) {
    this->_vertices.emplace_back(position);
}

void Mesh::add_vertex(float x, float y, float z) {
    this->_vertices.emplace_back(x, y, z);
}

void Mesh::add_vertex(float x, float y, float z, float u, float v) {
    this->_vertices.emplace_back(x, y, z, u, v);
}

void Mesh::add_vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) {
    this->_vertices.emplace_back(x, y, z, nx, ny, nz, u, v);
}

void Mesh::add_index(GLuint index) {
    this->_indices.push_back(index);
}

std::vector<Vertex> &Mesh::get_vertices() {
    return this->_vertices;
}

std::vector<GLuint> &Mesh::get_indices() {
    return this->_indices;
}

unsigned long Mesh::get_vertices_size() {
    return this->_vertices.size() * sizeof(Vertex);
}

unsigned long Mesh::get_indices_size() {
    return this->_indices.size() * sizeof(GLuint);
}

void Mesh::clear_vertices() {
    this->_vertices.clear();
}

void Mesh::clear() {
    this->_vertices.clear();

    this->_indices.clear();
}

} // namespace engine::model
