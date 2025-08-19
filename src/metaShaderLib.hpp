
#include <stdlib.h>
#include <string>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// === Utility Function String Snippets === //

namespace shaderLib {
/////
// INDIVIDUAL ELEMENT STRUCTURE CONTAINS THE FOLLOWING COMPONENTS. the elements
// are appended to a vector of elements. this vector is looped through, adding
// all the code to the final shader string //
/////

struct ShaderElement {
  std::string structure;
  std::string texture;
  std::string symmetry;
  std::string layering;
  std::string colorUsage;
};
////
// STRUCTURE FOR THE WHOLE SHADER. use this to specify which code gets fetched
// from the shader string library
////
struct ShaderTemplate {
  bool hasBackground;
  std::string backgroundColor;
  std::string colorPalette;
  std::vector<std::string> globalUniforms;
  std::vector<ShaderElement> elements; // vector of elements
};

// a pair of strings: top-level helpers + main() statements
struct Emitted {
  std::string helpers; // GLSL functions/defs (top-level)
  std::string calls;   // lines to paste inside main()
};

// HEADER OF STANDARD MATH and begining of allolib compatible glsl files-
// POPULATE MORE //
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
// dynamically takes uniforms inputted from template
std::string getUniforms(const ShaderTemplate &tmpl) {
  std::ostringstream out;
  for (const auto &u : tmpl.globalUniforms) {
    out << "uniform float " << u << ";\n";
  }
  return out.str();
}

// Example emitter (waveGrid). Add others similarly.
Emitted emitElement(const ShaderElement &element, int idx) {
  Emitted out;
  if (element.structure == "waveGrid") {
    std::string fn = "elementMath_" + std::to_string(idx);
    std::string v = "v_" + std::to_string(idx);

    out.helpers += "// this is a wave grid function\n"
                   "float " +
                   fn +
                   "(vec2 p) {\n"
                   "  return sin(p.x + sin(p.y * 2.0) + sin(p.y * 0.43));\n"
                   "}\n";

    // color mix could branch on element.colorUsage *here in C++*,
    // but the emitted GLSL is always straight-line.
    out.calls += "float " + v + " = " + fn +
                 "(uv);\n"
                 "col = mix(vec3(0.8, 0.9, 1.0), vec3(0.2, 0.6, 0.9), " +
                 v + ");\n";
  }
  return out;
}

/// main() now accepts the stitched body
std::string getMainFunction(const ShaderTemplate &tmpl,
                            const std::string &injectedBody) {
  std::ostringstream m;
  m <<
      R"GLSL(
void main() {
    vec2 uv = vPos.xy;
    float t = u_time;
    vec3 col = vec3(0.0);

)GLSL";
  m << injectedBody;

  if (tmpl.hasBackground) {
    m << "    col = mix(col, " << tmpl.backgroundColor << ", 0.1);\n";
  }
  m << "    fragColor = vec4(col, 1.0);\n}\n";
  return m.str();
}

} // namespace shaderLib