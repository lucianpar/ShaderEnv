#pragma once

#include <algorithm>
// #include <opencv2/core.hpp>
#include "../../shaderLib/shaderLibUtility.hpp"

  

namespace structures {

  using ShaderElement = shaderUtility::ShaderElement;
  using Emitted       = shaderUtility::Emitted;




// THE FOLLOWING FUNCTIONS RETRIEVE STRINGS FOR COMPONENETS OF AN ELEMENT ////
inline Emitted emitElementStructure(const ShaderElement &element, int elementIndex) {
  Emitted emmitedOutput;

  std::string functionName =
        element.structure + "_" + std::to_string(elementIndex);
  std::string val = "val_" + std::to_string(elementIndex);  // <-- standard name
  std::string uvi = "uv_" + std::to_string(elementIndex);   // uv specific to this element - important for element size and placement
  
  if (shaderUtility::isBlank(element.structure)) {
    std::cerr << "ERROR: Element " << elementIndex << " structure is empty " << std::endl;

  }

   const bool known =
       element.structure == "waveGrid"      ||
       element.structure == "noiseGrid"     ||
       element.structure == "circleField"   ||
       element.structure == "blob"          ||
       element.structure == "superformula"  ||
       element.structure == "lissajous"     ||
       element.structure == "lorenzAttractor" ||
       element.structure == "star" ||
       element.structure == "mandalaRadial"    ||
element.structure == "quasicrystal"     ||
element.structure == "voronoi"          ||
element.structure == "roseCurve"        ||
element.structure == "superellipse"     ||
element.structure == "phyllotaxis"      ||
element.structure == "julia"            ||
element.structure == "reactionDiffusion"||
element.structure == "branchNoise"
       ;

       if (!known) {
     std::cerr << "ERROR: Inputted structure " << elementIndex << " name does not match library" << std::endl;
  }
  else if (element.structure == "waveGrid") {
    

    emmitedOutput.helpers +=
        "// below is a wave grid function\n"
        "float " +
        functionName +
        "(vec2 p) {\n"
        "  return sin(p.x + sin(p.y * 2.0) + sin(p.y * 0.43));\n"
        "}\n";

    // color mix could branch on element.colorUsage *here in C++*,
    // but the emitted GLSL is always straight-line.

    emmitedOutput.calls +=
        // "float " + functionResult + " = " + functionName +
        // "(uv);\n"
        // "// Below mixes color to form the wave grid\n"
        // "col = mix(color1, color2, " +
        // functionResult +
        // ");\n"
        // "// Above is the end of wavegrid color mixing\n";
        "float " + val + " = " + functionName + "(" + uvi + ");\n";  // <- standard scalar name

  }

  else if (element.structure == "noiseGrid") {
    emmitedOutput.helpers +=
        "// Noise-based grid (pseudo-random)\n"
        "float " + functionName + "(vec2 p) {\n"
        "  return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);\n"
        "}\n";

    emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
  }

  else if (element.structure == "circleField") {
    emmitedOutput.helpers +=
        "// Circle field function\n"
        "float " + functionName + "(vec2 p) {\n"
        "  return length(p) - 0.5;\n"
        "}\n";

    emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
  }

  else if (element.structure == "blob") {
    emmitedOutput.helpers +=
        "// Organic blob shape with time-based wobble\n"
        "float " + functionName + "(vec2 p) {\n"
        "  float r = 0.5 + 0.1*sin(u_time + p.x*10.0) * cos(p.y*10.0);\n"
        "  return length(p) - r;\n"
        "}\n";

    emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
  }

  else if (element.structure == "superformula") {
    emmitedOutput.helpers +=
        "// Superformula-based shape\n"
        "float " + functionName + "(vec2 p) {\n"
        "  float m = 6.0;\n"
        "  float n1 = 0.3;\n"
        "  float n2 = 1.7;\n"
        "  float n3 = 1.7;\n"
        "  float a = 1.0, b = 1.0;\n"
        "  float phi = atan(p.y, p.x);\n"
        "  float r = pow(pow(abs(cos(m*phi/4.0)/a), n2) +\n"
        "                pow(abs(sin(m*phi/4.0)/b), n3), -1.0/n1);\n"
        "  return length(p) - r;\n"
        "}\n";

    emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
  }

  else if (element.structure == "lissajous") {
    emmitedOutput.helpers +=
        "// Lissajous curve pattern\n"
        "float " + functionName + "(vec2 p) {\n"
        "  float a = 3.0, b = 2.0;\n"
        "  float delta = PI/2.0;\n"
        "  return sin(a*p.x + delta) - sin(b*p.y);\n"
        "}\n";

    emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
  }

  else if (element.structure == "lorenzAttractor") {
    emmitedOutput.helpers +=
        "// Lorenz-like attractor projection\n"
        "float " + functionName + "(vec2 p) {\n"
        "  float sigma = 10.0;\n"
        "  float rho = 28.0;\n"
        "  float beta = 8.0/3.0;\n"
        "  vec3 v = vec3(p, 0.1);\n"
        "  for (int i = 0; i < 10; i++) {\n"
        "    vec3 dv;\n"
        "    dv.x = sigma * (v.y - v.x);\n"
        "    dv.y = v.x * (rho - v.z) - v.y;\n"
        "    dv.z = v.x * v.y - beta * v.z;\n"
        "    v += 0.01 * dv;\n"
        "  }\n"
        "  return length(v.xy);\n"
        "}\n";

    emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
  }

  else if (element.structure == "star") {
    emmitedOutput.helpers +=
        "// Star polygon pattern\n"
        "float " + functionName + "(vec2 p) {\n"
        "  float a = atan(p.y,p.x);\n"
        "  float r = cos(5.0*a) * 0.5 + 0.5;\n"
        "  return length(p) - r;\n"
        "}\n";

    emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
  }

  else if (element.structure == "mandalaRadial") {
  emmitedOutput.helpers +=
      "// Mandala-like radial kaleidoscope\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float sectors = 10.0;\n"
      "  float a = atan(p.y, p.x);\n"
      "  float r = length(p);\n"
      "  // fold angle into wedge and mirror for kaleidoscope\n"
      "  float wedge = TWO_PI / sectors;\n"
      "  a = mod(a, wedge);\n"
      "  a = abs(a - wedge * 0.5);\n"
      "  // ring pattern + angular sin modulation\n"
      "  float ring = 0.5 + 0.5 * sin(12.0 * r - u_time * 0.6);\n"
      "  float petals = 0.5 + 0.5 * sin(8.0 * a + r * 6.0);\n"
      "  return (ring * petals) - 0.5; // signed-ish value\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}

else if (element.structure == "quasicrystal") {
  emmitedOutput.helpers +=
      "// Quasicrystal from multiple rotated cos waves\n"
      "float " + functionName + "(vec2 p) {\n"
      "  const int N = 7; // number of directions\n"
      "  float sum = 0.0;\n"
      "  for (int i = 0; i < N; i++) {\n"
      "    float ang = (float(i) / float(N)) * PI; // half rotations avoid duplicates\n"
      "    vec2 dir = vec2(cos(ang), sin(ang));\n"
      "    sum += cos(dot(p * 6.0, dir) + u_time * 0.2);\n"
      "  }\n"
      "  sum /= float(N);\n"
      "  return sum; // in [-1,1]\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}
else if (element.structure == "voronoi") {
  emmitedOutput.helpers +=
      "// Voronoi / Worley F1 distance (cellular)\n"
      "float " + functionName + "(vec2 p) {\n"
      "  vec2 g = floor(p * 4.0);\n"
      "  vec2 f = fract(p * 4.0);\n"
      "  float d = 1e9;\n"
      "  for (int j = -1; j <= 1; j++) {\n"
      "    for (int i = -1; i <= 1; i++) {\n"
      "      vec2 o = vec2(i, j);\n"
      "      // hash: pseudo-random feature point inside cell\n"
      "      float n = fract(sin(dot(g + o, vec2(127.1, 311.7))) * 43758.5453);\n"
      "      float m = fract(sin(dot(g + o, vec2(269.5, 183.3))) * 43758.5453);\n"
      "      vec2 r = o + vec2(n, m) - f;\n"
      "      d = min(d, dot(r, r));\n"
      "    }\n"
      "  }\n"
      "  return 1.0 - sqrt(d); // brighter at cell centers\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}
else if (element.structure == "roseCurve") {
  emmitedOutput.helpers +=
      "// Rose (rhodonea) curve SDF-ish\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float k = 7.0; // petals (odd=k, even=2k)\n"
      "  float a = atan(p.y, p.x);\n"
      "  float r = length(p);\n"
      "  float target = 0.6 * abs(cos(k * a));\n"
      "  return (r - target); // near 0 on the curve\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}
else if (element.structure == "superellipse") {
  emmitedOutput.helpers +=
      "// Superellipse (squircle/rounded-rect family)\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float n = 4.0 + 2.0 * sin(u_time * 0.4); // exponent animates\n"
      "  vec2 a = vec2(0.7);\n"
      "  float v = pow(abs(p.x / a.x), n) + pow(abs(p.y / a.y), n);\n"
      "  return v - 1.0; // 0 at boundary\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}
else if (element.structure == "phyllotaxis") {
  emmitedOutput.helpers +=
      "// Phyllotaxis distribution, distance to nearest seed\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float N = 200.0; // seed count approximation\n"
      "  float phi = (3.14159265359 * (3.0 - sqrt(5.0))); // golden angle ~2.39996\n"
      "  float dmin = 1e9;\n"
      "  for (int i = 0; i < 180; i++) {\n"
      "    float fi = float(i);\n"
      "    float r = 0.015 * fi; // radial growth\n"
      "    float a = fi * phi + u_time * 0.1;\n"
      "    vec2 s = r * vec2(cos(a), sin(a));\n"
      "    dmin = min(dmin, length(p - s));\n"
      "  }\n"
      "  return 0.5 - dmin * 3.0; // bright at seed centers\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}
else if (element.structure == "julia") {
  emmitedOutput.helpers +=
      "// Julia set distance-ish field\n"
      "float " + functionName + "(vec2 p) {\n"
      "  vec2 z = p * 1.6;\n"
      "  // time-varying parameter c\n"
      "  vec2 c = vec2(-0.8 + 0.6 * sin(u_time * 0.23), 0.156 + 0.4 * cos(u_time * 0.19));\n"
      "  float m2 = 0.0;\n"
      "  float iter = 0.0;\n"
      "  for (int i = 0; i < 60; i++) {\n"
      "    z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;\n"
      "    m2 = dot(z, z);\n"
      "    iter += 1.0;\n"
      "    if (m2 > 16.0) break;\n"
      "  }\n"
      "  // map iterations to smooth value\n"
      "  return 1.0 - clamp(iter / 60.0, 0.0, 1.0);\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}
else if (element.structure == "reactionDiffusion") {
  emmitedOutput.helpers +=
      "// Faux reaction-diffusion: layered noise with temporal warp\n"
      "float hash(vec2 p){ return fract(sin(dot(p, vec2(127.1,311.7))) * 43758.5453); }\n"
      "float noise(vec2 p){ vec2 i=floor(p), f=fract(p);\n"
      "  float a=hash(i), b=hash(i+vec2(1,0)), c=hash(i+vec2(0,1)), d=hash(i+vec2(1,1));\n"
      "  vec2 u=f*f*(3.0-2.0*f);\n"
      "  return mix(mix(a,b,u.x), mix(c,d,u.x), u.y);\n"
      "}\n"
      "float fbm(vec2 p){ float s=0.0, a=0.5; for(int i=0;i<5;i++){ s+=a*noise(p); p*=2.02; a*=0.5;} return s; }\n"
      "float " + functionName + "(vec2 p) {\n"
      "  p *= 3.0;\n"
      "  float t = u_time * 0.2;\n"
      "  float u = fbm(p + vec2(t, -t));\n"
      "  float v = fbm(rot * (p * 1.9) + vec2(-t, t));\n"
      "  float w = fbm((p + vec2(u, v)) * 1.2);\n"
      "  float rd = smoothstep(0.35, 0.65, w) - 0.5;\n"
      "  return rd;\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}
else if (element.structure == "branchNoise") {
  emmitedOutput.helpers +=
      "// Branch-like field via angular warping + ridged fbm\n"
      "float n2hash(vec2 p){ return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }\n"
      "float n2(vec2 p){ vec2 i=floor(p), f=fract(p);\n"
      "  float a=n2hash(i), b=n2hash(i+vec2(1,0)), c=n2hash(i+vec2(0,1)), d=n2hash(i+vec2(1,1));\n"
      "  vec2 u=f*f*(3.0-2.0*f);\n"
      "  return mix(mix(a,b,u.x), mix(c,d,u.x), u.y);\n"
      "}\n"
      "float ridged(vec2 p){ float s=0.0, a=0.5; for(int i=0;i<5;i++){ s+=a*(1.0-abs(2.0*n2(p)-1.0)); p*=2.03; a*=0.5;} return s; }\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float r = length(p) + 1e-3;\n"
      "  float a = atan(p.y, p.x);\n"
      "  // create preferred growth directions\n"
      "  float forks = 5.0;\n"
      "  float dir = cos(a * forks) * 0.5 + 0.5;\n"
      "  float bark = ridged(p * vec2(2.0, 4.0) + vec2(0.0, u_time*0.15));\n"
      "  float veins = ridged(vec2(a * 1.5, r * 3.0));\n"
      "  float trunk = (0.35 / r) * dir; // stronger near center and branch angles\n"
      "  return trunk + 0.5 * veins + 0.3 * bark - 0.8;\n"
      "}\n";
  emmitedOutput.calls += "float " + val + " = " + functionName + "(" + uvi + ");\n";
}




  
  return emmitedOutput;
}









// STRUCTURE SIZE AND PLACEMENT 
// ---- Placement: defines uv_<i> from the global uv by translating to (cx, cy) ----
inline Emitted emitElementPlacement(const ShaderElement& element, int elementIndex) {
  Emitted out;
  double cx = 0.0, cy = 0.0;
  if (element.placementCoords.size() >= 2) {
    cx = std::max(-1.0, std::min(1.0, element.placementCoords[0]));
    cy = std::max(-1.0, std::min(1.0, element.placementCoords[1]));
  }
  const std::string uvi = "uv_" + std::to_string(elementIndex);

  if (cx == 0.0 && cy == 0.0) {
    out.calls += "vec2 " + uvi + " = uv;\n"; // always declares uv_i
  } else {
    out.calls += "// placement for element " + std::to_string(elementIndex) + "\n";
    out.calls += "vec2 " + uvi + " = uv - vec2("
              + std::to_string(cx) + ", " + std::to_string(cy) + ");\n";
  }
  return out;
}

inline Emitted emitElementSize(const ShaderElement& element, int elementIndex) {
  Emitted out;

  // read requested size; default to 1.0 if unset
  double s = (element.size == 0.0 ? 1.0 : static_cast<double>(element.size));

  // bound to [0.01, 1.0] without std::clamp
  if (s < 0.01) {
    std::cerr << "WARNING: Element " << elementIndex
              << " size < 0.01; clamped to 0.01" << std::endl;
    s = 0.01;
  } else if (s > 1.0) {
    std::cerr << "WARNING: Element " << elementIndex
              << " size > 1.0; clamped to 1.0" << std::endl;
    s = 1.0;
  }

  // no-op if full size
  if (s == 1.0) return out;

  const std::string uvi = "uv_" + std::to_string(elementIndex);
  out.calls += "// size for element " + std::to_string(elementIndex) + "\n";
  out.calls += uvi + " = " + uvi + " / " + std::to_string(s) + ";\n";
  return out;
}






}