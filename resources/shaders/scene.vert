#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 3) in uint v_ambient_occlusion_state;
layout(location = 4) in uint v_sunlight;

layout(location = 5) in uint v_texture_id;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_colour;

out vec3 f_fragment_position;

out vec3 f_normal;
out vec2 f_uv;

out vec3 f_colour;

out float f_ambient_occlusion;
out float f_sunlight;

flat out uint f_texture_id;

flat out uint f_face_index;

// PERF: Should probably pass face index to vertex shader
uint get_face_index(vec3 normal) {
    if (normal.y > 0.0f) {
        return 0U;
    }

    if (normal.y < 0.0f) {
        return 1U;
    }

    if (normal.x > 0.0f) {
        return 2U;
    }

    if (normal.x < 0.0f) {
        return 3U;
    }

    if (normal.z > 0.0f) {
        return 4U;
    }

    return 5U;
}

// TODO: Could try and test with other values for smoother AO

// NOTE: AO1
const float ambient_occlusion[4] = float[4](0.1f, 0.25f, 0.5f, 1.0f);

// NOTE: AO2
// const float ambient_occlusion[4] = float[4](0.5f, 0.65f, 0.8f, 1.0f);

void main() {
    vec4 world_space = u_model * vec4(v_position, 1.0f);

    f_fragment_position = world_space.xyz;

    f_normal = mat3(transpose(inverse(u_model))) * v_normal;
    f_uv = v_uv;

    f_colour = u_colour;

    f_face_index = get_face_index(v_normal);

    f_ambient_occlusion = ambient_occlusion[int(v_ambient_occlusion_state)];
    f_sunlight = float(max(v_sunlight, 1U)) / 15.0f;

    f_texture_id = v_texture_id;

    gl_Position = u_projection * u_view * world_space;
}
