#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_colour;

out vec3 f_fragment_position;

out vec3 f_normal;
out vec2 f_uv;

out vec3 f_colour;

void main() {
    vec4 world_space = u_model * vec4(v_position, 1.0f);

    f_fragment_position = world_space.xyz;

    f_normal = mat3(transpose(inverse(u_model))) * v_normal;
    f_uv = v_uv;

    f_colour = u_colour;

    gl_Position = u_projection * u_view * world_space;
}
