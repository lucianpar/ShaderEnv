#version 330 core

in vec3 vPos;
in vec2 vUV;
out vec4 fragColor;

#define PI 3.14159265359
#define TWO_PI 6.28318530718
const mat2 rot = mat2(0.5, 0.86, -0.86, 0.5);
uniform float u_time;
const vec3 color0 = vec3(0.8, 0.9, 1);
const vec3 color1 = vec3(0.2, 0.6, 0.9);
const vec3 color2 = vec3(0.9, 0.03, 0.07);
// below is a wave grid function
float waveGrid_0(vec2 p) {
  return sin(p.x + sin(p.y * 2.0) + sin(p.y * 0.43));
}

void main() {
    vec2 uv = vPos.xy;
    float t = u_time;
    vec3 col = vec3(0.0);

float waveGridVal_0 = waveGrid_0(uv);
// Below mixes color to form the wave grid
col = mix(color1, color2, waveGridVal_0);
// Above is the end of wavegrid color mixing
// Apply 4-way symmetry to UVs
uv = abs(uv);
    col = mix(col, vec3(0.9, 0.03, 0.07), 0.1);
    fragColor = vec4(col, 1.0);
}
