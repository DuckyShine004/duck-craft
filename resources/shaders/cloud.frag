#version 330 core

in vec3 f_vertex_position;
in vec3 f_fragment_position;

uniform float u_time;

uniform vec3 u_camera_position;

uniform sampler2DArray u_cloud_texture_array;

out vec4 o_colour;

vec2 compute_uv(vec3 fragment_position, float time) {
    const float TEXTURE_SIZE = 256.0f;

    const float SCALE = 8.0f;
    const float SPEED = 0.002f;

    vec2 uv = fragment_position.xz * (1.0f / (TEXTURE_SIZE * SCALE));

    uv.x -= time * SPEED;

    return uv;
}

vec4 compute_transparency(vec4 colour) {
    const float TRANSPARENCY = 0.75f;

    colour.a *= TRANSPARENCY;

    return colour;
}

vec4 compute_square_fade(vec4 colour, vec3 vertex_position) {
    const float QUAD_SIZE = 1024.0f;

    const float FADE_START = QUAD_SIZE * 0.25f;
    const float FADE_END = QUAD_SIZE;

    float distance = max(abs(vertex_position.x), abs(vertex_position.z));

    float fade = 1.0f - smoothstep(FADE_START, FADE_END, distance);

    colour.a *= fade;

    return colour;
}

vec4 compute_circular_fade(vec4 colour, vec3 fragment_position, vec3 camera_position) {
    const float QUAD_SIZE = 1024.0f;

    const float FADE_START = QUAD_SIZE * 0.25f;
    const float FADE_END = QUAD_SIZE * 0.75;

    float distance = distance(fragment_position.xz, camera_position.xz);

    float fade = 1.0f - smoothstep(FADE_START, FADE_END, distance);

    colour.a *= fade;

    return colour;
}

void main() {
    vec2 uv = compute_uv(f_fragment_position, u_time);

    vec4 colour = texture(u_cloud_texture_array, vec3(uv, 0.0f));

    if (colour.a < 0.5f) {
        discard;
    }

    colour = compute_transparency(colour);

    colour = compute_circular_fade(colour, f_fragment_position, u_camera_position);

    o_colour = colour;
}
