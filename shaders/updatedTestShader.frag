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
// Organic blob shape with time-based wobble
float blob_0(vec2 p) {
  float r = 0.5 + 0.1*sin(u_time + p.x*10.0) * cos(p.y*10.0);
  return length(p) - r;
}
// Organic blob shape with time-based wobble
float blob_1(vec2 p) {
  float r = 0.5 + 0.1*sin(u_time + p.x*10.0) * cos(p.y*10.0);
  return length(p) - r;
}
// texture: fbm tone-map (placeholder)
float fbmTone_1(float x){ return 0.5 + 0.5*sin(6.28318*x + 2.0*x); }

void main() {
    vec2 uv = vPos.xy;
    float t = u_time;
    vec3 col = vec3(0.0);

float val_0 = blob_0(uv);
// behavior: rotateUV(u_time) * speed=0.200000
float a = (u_time * 0.200000) * 0.5;
float c = cos(a), s = sin(a);
uv = mat2(c, -s, s, c) * uv;
// color usage: primary
col = mix(color0, color1, val_0);
// Apply 4-way symmetry to UVs
uv = abs(uv);
float val_1 = blob_1(uv);
// behavior: sineMod(u_time) * speed=0.800000
val_1 = 0.5 + 0.5 * sin((u_time * 0.800000) + 6.28318 * val_1);
val_1 = fbmTone_1(val_1);
// color usage: default (grayscale add)
col += vec3(val_1);
    col = mix(col, vec3(0.9, 0.03, 0.07), 0.1);
    fragColor = vec4(col, 1.0);
}
