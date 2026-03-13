#ifndef FOG_GLSL

#define FOG_GLSL

/* NOTE: Computes light scattering using the Henyey-Greenstein phase function
 *
 * https://bartwronski.com/wp-content/uploads/2014/08/bwronski_volumetric_fog_siggraph2014.pdf
 */
float compute_phase(vec3 ray_direction, vec3 light_direction) {
    const float PI = radians(180.0f);

    const float INV_4PI = 1.0f / (4.0f * PI);

    const float G = 0.6f;
    const float G2 = G * G;

    const float POWER = 1.5f;

    vec3 V = -ray_direction;
    vec3 L = normalize(-light_direction);

    float cos_theta = clamp(dot(V, L), -1.0f, 1.0f);

    return INV_4PI * ((1.0f - G2) / pow(1.0f + G2 - 2.0f * G * cos_theta, POWER));
}

/* NOTE: Computes the fog density at a given ray position. In this case, we'll use height-based fog equation
 * to model fog density.
 */
// float compute_fog_density(vec3 ray_position) {
//     const float base_density = 0.04f;
//     const float fog_base_y = 0.0f;
//     const float falloff = 0.12f;
//
//     float h = ray_position.y - fog_base_y;
//     float d = base_density * exp(-falloff * max(h, 0.0f));
//
//     return clamp(d, 0.0f, 1.0f);
// }

/* NOTE: Computes the fog density at a given ray position. In this case, it uses a spherical shape, with quadratic dampening.
 *
 */
float compute_fog_density(vec3 ray_position, vec3 camera_position) {
    const float FOG_DENSITY = 0.5f;

    const float RADIUS = 50.0f;

    float distance = length(ray_position - camera_position);

    float t = clamp(distance / RADIUS, 0.0f, 1.0f);

    return FOG_DENSITY * t * t;
}

/* NOTE: Computes the light visibility at a given ray position.
 *
 */
float compute_light_visibility(vec3 ray_position) {
    return 0.5f;
}

/* NOTE: Computes the volumetric fog colour. Note that currently, there is no sunlight visibility, meaning we assume that
 * every fragment is equally lit, even though that is not the case.
 */
vec4 compute_volumetric_fog(vec3 fragment_position, vec3 camera_position) {
    /* Absorption coefficient */
    const float SIGMA_A = 0.02f;

    /* Scattering coefficient */
    const float SIGMA_S = 0.04f;

    vec3 ray_direction = normalize(fragment_position - camera_position);

    float ray_length = length(fragment_position - camera_position);

    float step_size = ray_length / 64.0f;

    vec3 ray_position = camera_position;

    vec3 fog_colour = vec3(0.0f);

    float total_transmittance = 1.0f;

    /* NOTE: We use light colour = white for now... */
    vec3 light_colour = vec3(1.0f);

    /* NOTE: We use light direction of x = -1, y = -1 for now... */
    vec3 light_direction = vec3(1.0f, 1.0f, 0.0f);

    for (int step = 0; step < 64; ++step) {
        ray_position += ray_direction * step_size;

        float density = compute_fog_density(ray_position, camera_position);

        float transmittance = exp(-(SIGMA_A + SIGMA_S) * density * step_size);

        float phase = compute_phase(ray_direction, light_direction);

        /* TODO: Someday do this lol... FOR GOD RAYS! */
        // float light_visibility = compute_light_visibility(ray_position);

        vec3 light_scattered = light_colour * phase * SIGMA_S * density * step_size;

        fog_colour += light_scattered * total_transmittance;

        total_transmittance *= transmittance;

        if (total_transmittance < 0.01f) {
            break;
        }
    }

    return vec4(fog_colour, total_transmittance);
}

#endif
