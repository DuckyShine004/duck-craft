#version 330 core

#extension GL_GOOGLE_include_directive : require

#include "common/fog.glsl"
#include "common/colour.glsl"

in vec3 f_fragment_position;

in vec3 f_normal;
in vec2 f_uv;

in vec3 f_colour;

in float f_ambient_occlusion;
in float f_sunlight;

flat in uint f_texture_id;

flat in uint f_face_index;

uniform sampler2DArray u_block_texture_array;

uniform float u_gamma;
uniform float u_saturation;

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

void main() {
    vec4 colour = texture(u_block_texture_array, vec3(f_uv, float(f_texture_id)));

    float face_shade = face_shades[f_face_index];

    // DEBUG: Test lightmap
    // o_colour = vec4(0.0f, face_shade * f_sunlight, 0.0f, 1.0f);

    // INFO: Sharpness

    // INFO: Without lightmap
    // colour = vec4(colour.rgb * f_ambient_occlusion * face_shade, colour.a);

    // INFO: Final fragment
    colour = vec4(colour.rgb * f_ambient_occlusion * face_shade * f_sunlight, colour.a);

    /* INFO: Volumetric Fog */
    vec4 volumetric_fog = compute_volumetric_fog(f_fragment_position, u_camera_position);

    vec3 fog_colour = vec3(0.5f, 0.5f, 0.5f);

    colour = vec4(colour.rgb * volumetric_fog.a + volumetric_fog.rgb + (1.0f - volumetric_fog.a) * fog_colour, colour.a);

    /* INFO: Constrast */
    // float brightness = face_shade * f_sunlight;
    //
    // vec3 constrast = vec3(0.1f);
    //
    // colour = vec4(constrast * (colour.rgb - vec3(0.5f)) + vec3(0.5f) + vec3(brightness), colour.a);

    /* INFO: Saturation */
    colour = compute_saturation(colour, u_saturation);

    /* INFO: Gamma Correction */
    colour = compute_gamma_correction(colour, u_gamma);

    o_colour = colour;
}
