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
// Star polygon pattern
float star_0(vec2 p) {
  float a = atan(p.y,p.x);
  float r = cos(5.0*a) * 0.5 + 0.5;
  return length(p) - r;
}

void main() {
    vec2 uv = vPos.xy;
    float t = u_time;
    vec3 col = vec3(0.0);

// placement for element 0
vec2 uv_0 = uv - vec2(1.000000, 0.000000);
// size for element 0
uv_0 = uv_0 / 0.500000;
float val_0 = star_0(uv_0);
// behavior: sineMod(u_time) * speed=2.000000
val_0 = 0.5 + 0.5 * sin((u_time * 2.000000) + 6.28318 * val_0);
// color usage: primary
col = mix(color0, color1, val_0);
    col = mix(col, vec3(0.9, 0.03, 0.07), 0.1);
    fragColor = vec4(col, 1.0);
}
