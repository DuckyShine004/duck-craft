#pragma once

#include "external/glad/glad.h"

#include "engine/model/vertex.hpp"
#include "engine/model/topology.hpp"

namespace engine::model {

class Mesh {
  public:
    void upload();

    void render(engine::model::Topology topology);

    void add_vertex(Vertex &vertex);
    void add_vertex(glm::vec3 &position);
    void add_vertex(float x, float y, float z);
    void add_vertex(float x, float y, float z, float u, float v);
    void add_vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v);

    void add_index(GLuint index);

    template <std::size_t N> void add_vertices(const float (&vertices)[N][3]) {
        for (int i = 0; i < N; ++i) {
            float x = vertices[i][0];
            float y = vertices[i][1];
            float z = vertices[i][2];

            this->_vertices.emplace_back(x, y, z);
        }
    }

    template <std::size_t N> void add_vertices(const float (&vertices)[N][3], const float (&uvs)[N][2]) {
        for (int i = 0; i < N; ++i) {
            float x = vertices[i][0];
            float y = vertices[i][1];
            float z = vertices[i][2];

            float u = uvs[i][0];
            float v = uvs[i][1];

            this->_vertices.emplace_back(x, y, z, u, v);
        }
    }

    template <std::size_t N> void add_vertices(const float (&vertices)[N][3], const float (&normals)[N][3], const float (&uvs)[N][2]) {
        for (int i = 0; i < N; ++i) {
            float x = vertices[i][0];
            float y = vertices[i][1];
            float z = vertices[i][2];

            float nx = normals[i][0];
            float ny = normals[i][1];
            float nz = normals[i][2];

            float u = uvs[i][0];
            float v = uvs[i][1];

            this->_vertices.emplace_back(x, y, z, nx, ny, nz, u, v);
        }
    }

    template <std::size_t N> void add_indices(const GLuint (&indices)[N]) {
        this->_indices.insert(this->_indices.end(), std::begin(indices), std::end(indices));
    }

    std::vector<engine::model::Vertex> &get_vertices();

    std::vector<GLuint> &get_indices();

    unsigned long get_vertices_size();

    unsigned long get_indices_size();

    void clear_vertices();

  private:
    static inline constexpr unsigned int _POSITION_ATTRIBUTE = 0;
    static inline constexpr unsigned int _NORMAL_ATTRIBUTE = 1;
    static inline constexpr unsigned int _UV_ATTRIBUTE = 2;

    GLuint _vao;
    GLuint _vbo;
    GLuint _ibo;

    std::vector<engine::model::Vertex> _vertices;

    std::vector<GLuint> _indices;

    void draw_lines();

    void draw_triangles();
};

} // namespace engine::model
