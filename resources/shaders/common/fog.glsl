#ifndef FOG_GLSL

#define FOG_GLSL

/* NOTE: Computes light scattering using the Henyey-Greenstein phase function
 *
 * https://bartwronski.com/wp-content/uploads/2014/08/bwronski_volumetric_fog_siggraph2014.pdf
 */
float compute_phase(float phase) {
    const float PI = radians(180.0f);

    const float PI_4 = 0.25f * PI;

    const float G = 0.3f;
    const float G2 = G * G;

    const float POWER = 1.5f;

    return PI_4 * ((1 - G2) / pow(1 + G2 - 2 * G * cos(phase), POWER));
}

#endif
