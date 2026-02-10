#version 330 core

in vec3 f_fragment_position;

in vec3 f_normal;
in vec2 f_uv;

in vec3 f_colour;

uniform sampler2DArray u_block_texture_array;

out vec4 o_colour;

void main() {
    vec4 colour = texture(u_block_texture_array, vec3(f_uv, 2.0f));

    // o_colour = vec4(f_colour, 1.0f);
    o_colour = colour;
    // o_colour = vec4(fract(f_uv), 0.0, 1.0);
}
