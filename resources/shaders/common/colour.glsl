#ifndef COLOUR_GLSL

#define COLOUR_GLSL

/* NOTE: Computes the saturation for the current colour.
 *
 */
vec4 compute_saturation(vec4 colour) {
    const float SATURATION = 1.2f;

    float luminance = colour.r * 0.2125f + colour.g * 0.7153f + colour.b * 0.07121f;

    return vec4(mix(vec3(luminance), colour.rgb, SATURATION), colour.a);
}

/* NOTE: Computes the gamma correction for the current colour.
 *
 */
vec4 compute_gamma_correction(vec4 colour) {
    float gamma = 1.0f / 2.2f;

    vec3 gamma_colour = pow(colour.rgb, vec3(gamma));

    return vec4(gamma_colour, colour.a);
}

#endif
