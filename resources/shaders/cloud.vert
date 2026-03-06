#version 330 core

layout(location = 0) in vec3 v_position;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 f_vertex_position;
out vec3 f_fragment_position;

void main() {
    vec4 world_space = u_model * vec4(v_position, 1.0f);

    f_vertex_position = v_position.xyz;
    f_fragment_position = world_space.xyz;

    gl_Position = u_projection * u_view * world_space;
}
