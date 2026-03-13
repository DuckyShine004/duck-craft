#version 330 core

#include "resources/shaders/common/fog.glsl"

in vec3 f_fragment_position;

in vec3 f_normal;
in vec2 f_uv;

in vec3 f_colour;

in float f_ambient_occlusion;
in float f_sunlight;

flat in uint f_texture_id;

flat in uint f_face_index;

uniform sampler2DArray u_block_texture_array;

uniform vec3 u_camera_position;

out vec4 o_colour;

const float face_shades[6] = float[6](
        1.0f, 0.5f,
        0.5f, 0.8f,
        0.5f, 0.8f
    );

float compute_fog(vec3 fragment_position, vec3 camera_position) {
    const float FOG_START = 10.0f;
    const float FOG_END = 50.0f;

    float distance = length(fragment_position - camera_position);

    float fog_factor = (FOG_END - distance) / (FOG_END - FOG_START);

    return clamp(fog_factor, 0.0f, 1.0f);
}

vec4 compute_saturation(vec4 colour) {
    const float SATURATION = 1.2f;

    float luminance = colour.r * 0.2125f + colour.g * 0.7153f + colour.b * 0.07121f;

    return vec4(mix(vec3(luminance), colour.rgb, SATURATION), colour.a);
}

vec4 get_gamma_correction(vec4 colour) {
    float gamma = 1.0f / 2.2f;

    vec3 gamma_colour = pow(colour.rgb, vec3(gamma));

    return vec4(gamma_colour, colour.a);
}

void main() {
    float face_shade = face_shades[f_face_index];

    vec4 colour = texture(u_block_texture_array, vec3(f_uv, float(f_texture_id)));

    // vec4 w_colour = vec4(1.0f);
    // o_colour = vec4(f_colour, 1.0f);
    // o_colour = vec4(w_colour.rgb * face_shade, w_colour.a);
    // o_colour = vec4(w_colour.rgb * f_ambient_occlusion * face_shade, w_colour.a);

    // INFO: Test lightmap
    // vec4 t_colour = vec4(1.0f);

    // o_colour = vec4(0.0f, t_colour.g * face_shade * f_sunlight, 0.0f, t_colour.a);

    // o_colour = vec4(colour.rgb * f_ambient_occlusion * face_shade, colour.a);

    // INFO: Sharpness

    // INFO: Without lightmap
    // colour = vec4(colour.rgb * f_ambient_occlusion * face_shade, colour.a);

    // INFO: Final fragment
    colour = vec4(colour.rgb * f_ambient_occlusion * face_shade * f_sunlight, colour.a);

    // INFO: Fog
    // vec3 fog_colour = vec3(0.5f, 0.5f, 0.5f);
    //
    // float fog = compute_fog(f_fragment_position, u_camera_position);
    //
    // colour = vec4(mix(fog_colour, colour.rgb, fog), colour.a);

    /* INFO: Constrast */
    // float brightness = face_shade * f_sunlight;
    //
    // vec3 constrast = vec3(0.1f);
    //
    // colour = vec4(constrast * (colour.rgb - vec3(0.5f)) + vec3(0.5f) + vec3(brightness), colour.a);

    /* INFO: Saturation */
    colour = compute_saturation(colour);

    /* INFO: Volumetric Fog */
    vec4 volumetric_fog = compute_volumetric_fog(f_fragment_position, u_camera_position);

    vec3 fog_colour = vec3(0.5f, 0.5f, 0.5f);

    colour = vec4(colour.rgb * volumetric_fog.a + volumetric_fog.rgb + (1.0f - volumetric_fog.a) * fog_colour, colour.a);

    /* INFO: Gamma Correction */
    // colour = get_gamma_correction(colour);

    o_colour = colour;
}
