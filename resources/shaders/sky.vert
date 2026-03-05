#version 330 core

const vec2 FULL_SCREEN_TRIANGLE_VERTICES[3] = vec2[3](
        vec2(-1, -1),
        vec2(3, -1),
        vec2(-1, 3)
    );

void main() {
    gl_Position = vec4(FULL_SCREEN_TRIANGLE_VERTICES[gl_VertexID], 0.0f, 1.0f);
}
