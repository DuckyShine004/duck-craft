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

out vec4 o_colour;

const float face_shades[6] = float[6](
        1.0f, 0.5f,
        0.5f, 0.8f,
        0.5f, 0.8f
    );

vec4 get_gamma_correction(vec4 colour) {
    float gamma = 1.0f / 2.2f;

    vec3 gamma_colour = pow(colour.rgb, vec3(gamma));

    return vec4(gamma_colour, colour.a);
}

void main() {
    float face_shade = face_shades[f_face_index];

    vec4 colour = texture(u_block_texture_array, vec3(f_uv, float(f_texture_id)));

    // colour = get_gamma_correction(colour);

    // vec4 w_colour = vec4(1.0f);
    // o_colour = vec4(f_colour, 1.0f);
    // o_colour = vec4(w_colour.rgb * face_shade, w_colour.a);
    // o_colour = vec4(w_colour.rgb * f_ambient_occlusion * face_shade, w_colour.a);

    // INFO: Test lightmap
    // vec4 t_colour = vec4(1.0f);

    // o_colour = vec4(0.0f, t_colour.g * face_shade * f_sunlight, 0.0f, t_colour.a);

    // o_colour = vec4(colour.rgb * f_ambient_occlusion * face_shade, colour.a);

    // float brightness = face_shade * f_sunlight;
    //
    // vec3 constrast = vec3(0.1f);
    //
    // colour = vec4(constrast * (colour.rgb - vec3(0.5f)) + vec3(0.5f) + vec3(brightness), colour.a);

    // INFO: Saturation
    // float luminance = colour.r * 0.2125f + colour.g * 0.7153f + colour.b * 0.07121f;
    //
    // float saturation = 100.0f;
    //
    // colour = vec4(mix(vec3(luminance), colour.rgb, saturation), colour.a);

    // INFO: Sharpness

    // INFO: Without lightmap
    o_colour = vec4(colour.rgb * f_ambient_occlusion * face_shade, colour.a);

    // INFO: Render actual fragment here
    // o_colour = vec4(colour.rgb * f_ambient_occlusion * face_shade * f_sunlight, colour.a);
}
