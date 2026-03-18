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

out vec3 f_fragment_position;

out vec3 f_normal;
out vec2 f_uv;

out float f_sunlight;

flat out uint f_texture_id;

flat out uint f_face_index;

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

void main() {
    vec4 world_space = u_model * vec4(v_position.x, v_position.y - 0.125f, v_position.z, 1.0f);

    f_fragment_position = world_space.xyz;

    f_normal = mat3(transpose(inverse(u_model))) * v_normal;
    f_uv = v_uv;

    f_face_index = get_face_index(v_normal);

    f_sunlight = float(max(v_sunlight, 1U)) / 15.0f;

    f_texture_id = v_texture_id;

    gl_Position = u_projection * u_view * world_space;
}
