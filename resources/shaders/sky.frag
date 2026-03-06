#version 330 core

uniform mat4 u_inverse_view;
uniform mat4 u_inverse_projection;

uniform vec2 u_resolution;

out vec4 o_colour;

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution;
    vec2 ndc = uv * 2.0f - 1.0f;

    vec4 clip_space = vec4(ndc, 1.0f, 1.0f);

    vec4 view_space = u_inverse_projection * clip_space;
    view_space = vec4(view_space.xy, -1.0f, 0.0f);

    vec3 direction = normalize((u_inverse_view * view_space).xyz);

    // Gradient factor
    float h = clamp(direction.y * 0.5f + 0.5f, 0.0f, 1.0f);

    // Colours
    vec3 horizon = vec3(0.75, 0.85, 1.0);
    vec3 zenith = vec3(0.25, 0.45, 0.95);

    vec3 sky = mix(horizon, zenith, pow(h, 1.5));

    o_colour = vec4(sky, 1.0f);
}
