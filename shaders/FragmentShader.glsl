#version 330 core

in vec3 fragColor;
out vec4 out_color;

uniform int u_damaged;
uniform float u_time;  

void main()
{
    vec3 col = fragColor;

    if (u_damaged == 1) {
        vec3 highlight = vec3(1.0, 0.25, 0.05);
        float vPulse = 0.5 + 0.5 * sin(u_time * 6.0); // 0..1
        float strength = 0.75 * vPulse;
        col = mix(col, highlight, strength);
    }

    out_color = vec4(col, 1.0);
}
