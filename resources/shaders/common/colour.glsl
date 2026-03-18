#ifndef COLOUR_GLSL

#define COLOUR_GLSL

/* NOTE: The current post processing pipeline:
 * 1. Saturation
 * 2. Gamma correction
 */

/* NOTE: Computes the saturation for the current colour.
 *
 */
vec4 compute_saturation(vec4 colour, float saturation) {
    float luminance = colour.r * 0.2125f + colour.g * 0.7153f + colour.b * 0.07121f;

    return vec4(mix(vec3(luminance), colour.rgb, saturation), colour.a);
}

/* NOTE: Computes the gamma correction for the current colour.
 *
 */
vec4 compute_gamma_correction(vec4 colour, float exponent) {
    float gamma = 1.0f / exponent;

    vec3 gamma_colour = pow(colour.rgb, vec3(gamma));

    return vec4(gamma_colour, colour.a);
}

#endif
