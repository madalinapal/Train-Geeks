#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 3) in vec3 v_color;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform int   u_damaged;
uniform float u_seed;
uniform float u_time;

uniform float u_dipAmp;
uniform float u_sideAmp;
uniform float u_tiltAmp;

uniform float u_pulseAmp; 

out vec3 fragColor;

void main()
{
    vec3 pos = v_position;

    if (u_damaged == 1) {
        // pulse 0..1 (seed ca sa nu pulseze toate identic)
        float p = 0.5 + 0.5 * sin(u_time * 6.0 + u_seed * 10.0);

        // scale factor 1 .. (1 + u_pulseAmp)
        float s = 1.0 + u_pulseAmp * p;

        // 1 pulse pe GROSIME:
        pos.y = -0.5 + (pos.y + 0.5) * s;

        // - pe Z: in jurul centrului
        pos.z *= s;

        // 2 deformare 
        float u = pos.x + 0.5; // ~0..1

        float dip  = u_dipAmp  * sin(10.0 * u + u_seed);
        float side = u_sideAmp * sin(14.0 * u + u_seed * 1.7);

        pos.y += dip;
        pos.z += side;

        pos.y += tan(u_tiltAmp) * pos.z;
    }

    fragColor = v_color;
    gl_Position = Projection * View * Model * vec4(pos, 1.0);
}
