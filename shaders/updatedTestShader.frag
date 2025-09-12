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
// Phyllotaxis distribution, distance to nearest seed
float phyllotaxis_0(vec2 p) {
  float N = 200.0; // seed count approximation
  float phi = (3.14159265359 * (3.0 - sqrt(5.0))); // golden angle ~2.39996
  float dmin = 1e9;
  for (int i = 0; i < 180; i++) {
    float fi = float(i);
    float r = 0.015 * fi; // radial growth
    float a = fi * phi + u_time * 0.1;
    vec2 s = r * vec2(cos(a), sin(a));
    dmin = min(dmin, length(p - s));
  }
  return 0.5 - dmin * 3.0; // bright at seed centers
}
// Mandala-like radial kaleidoscope
float mandalaRadial_1(vec2 p) {
  float sectors = 10.0;
  float a = atan(p.y, p.x);
  float r = length(p);
  // fold angle into wedge and mirror for kaleidoscope
  float wedge = TWO_PI / sectors;
  a = mod(a, wedge);
  a = abs(a - wedge * 0.5);
  // ring pattern + angular sin modulation
  float ring = 0.5 + 0.5 * sin(12.0 * r - u_time * 0.6);
  float petals = 0.5 + 0.5 * sin(8.0 * a + r * 6.0);
  return (ring * petals) - 0.5; // signed-ish value
}
// texture: fbm tone-map (placeholder)
float fbmTone_1(float x){ return 0.5 + 0.5*sin(6.28318*x + 2.0*x); }
vec3 blendOverlay_1(vec3 b, vec3 s) {
  vec3 lo = 2.0 * b * s;
  vec3 hi = 1.0 - 2.0 * (1.0 - b) * (1.0 - s);
  return mix(lo, hi, step(0.5, b));
}

void main() {
    vec2 uv = vPos.xy;
    float t = u_time;
    vec3 col = vec3(0.0);

// Apply horizontal symmetry to UVs
uv.x = abs(uv.x);
// placement for element 0
vec2 uv_0 = uv - vec2(1.000000, 0.000000);
// size for element 0
uv_0 = uv_0 / 0.500000;
float val_0 = phyllotaxis_0(uv_0);
// behavior: sineMod(u_time) * speed=2.000000
val_0 = 0.5 + 0.5 * sin((u_time * 2.000000) + 6.28318 * val_0);
// texture: abs
val_0 = abs(val_0);
// color usage: primary -> layerCol_i
vec3 layerCol_0 = mix(color0, color1, val_0);
col = mix(col, col + layerCol_0, val_0);
// Apply 4-way symmetry to UVs
uv = abs(uv);
vec2 uv_1 = uv;
float val_1 = mandalaRadial_1(uv_1);
val_1 = fbmTone_1(val_1);
// color usage: primary -> layerCol_i
vec3 layerCol_1 = mix(color0, color1, val_1);
col = mix(col, blendOverlay_1(col, layerCol_1), val_1);
    col = mix(col, vec3(0.9, 0.03, 0.07), 0.1);
    fragColor = vec4(col, 1.0);
}
