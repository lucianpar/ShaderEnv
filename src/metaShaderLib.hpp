
#include <stdlib.h>
#include <string>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// === Utility Function String Snippets === //

namespace shaderLib {

struct ShaderElement {
  std::string structure;
  std::string texture;
  std::string symmetry;
  std::string layering;
  std::string colorUsage;
};

struct ShaderTemplate {
  bool hasBackground;
  std::string backgroundColor;
  std::string colorPalette;
  std::vector<std::string> globalUniforms;
  std::vector<ShaderElement> elements; // vector of elements
};

std::string getHeader() {
  return R"GLSL(#version 330 core

in vec3 vPos;
in vec2 vUV;
out vec4 fragColor;

#define PI 3.14159265359
#define TWO_PI 6.28318530718
const mat2 rot = mat2(0.5, 0.86, -0.86, 0.5);
)GLSL";
}

std::string getUniforms(const ShaderTemplate &tmpl) {
  std::ostringstream out;
  for (const auto &u : tmpl.globalUniforms) {
    out << "uniform float " << u << ";\n";
  }
  return out.str();
}

std::string getWaveFunction() {
  return R"GLSL(

float wave(vec2 p) {
    return sin(p.x + sin(p.y * 2.0) + sin(p.y * 0.43));
}
)GLSL";
}

std::string getMainFunction(const ShaderTemplate &tmpl) {
  return R"GLSL(

void main() {
    vec2 uv = vPos.xy;
    float t = u_time;
    vec3 col = vec3(0.0);

    // example element call:
    float v = wave(uv);
    col = mix(vec3(0.8, 0.9, 1.0), vec3(0.2, 0.6, 0.9), v);

)GLSL" +
         (tmpl.hasBackground
              ? "    col = mix(col, " + tmpl.backgroundColor + ", 0.1);\n"
              : "") +
         R"GLSL(
    fragColor = vec4(col, 1.0);
}
)GLSL";
}

} // namespace shaderLib