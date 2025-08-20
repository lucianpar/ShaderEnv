
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

using Color = std::vector<float>;         // alias for "std::vector<float>". "using" seems to helpful for readability
using ColorPalette = std::vector<Color>;  // alias for "std::vector<std::vector<float>>" 

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
  ColorPalette colorPalette;
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
std::string getColorPalette(const ShaderTemplate &tmpl) {
    std::ostringstream out;

    for (int i = 0; i < static_cast<int>(tmpl.colorPalette.size()); ++i) {
        const auto &c = tmpl.colorPalette[i];
        if (c.size() == 3) {
            out << "const vec3 color" << i
                << " = vec3(" << c[0] << ", "
                              << c[1] << ", "
                              << c[2] << ");\n";
        }
    }
    return out.str();
}


// as the code generator loops through the elements vector, this will run for
// each element
Emitted emitElement(const ShaderElement &element, int elementIndex) {
  Emitted emmitedOutput;
  if (element.structure == "waveGrid") {
    std::string functionName =
        element.structure + "_" + std::to_string(elementIndex);
    std::string functionResult =
        element.structure + "Val_" + std::to_string(elementIndex);

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
        "float " + functionResult + " = " + functionName +
        "(uv);\n"
        "// Below mixes color to form the wave grid\n"
        "col = mix(color1, color2, " +
        functionResult +
        ");\n"
        "// Above is the end of wavegrid color mixing\n";
  }

  // placeholder examples
  else if (element.structure == "noiseGrid") {
    // emit noiseGrid
  } else if (element.structure == "circleField") {
    // emit circleField
  }
  return emmitedOutput;
}

/// main() now accepts the stitched body
std::string getMainFunction(const ShaderTemplate &tmpl,
                            const std::string &injectedBody) {
  std::ostringstream mainFunction;
  mainFunction <<
      R"GLSL(
void main() {
    vec2 uv = vPos.xy;
    float t = u_time;
    vec3 col = vec3(0.0);

)GLSL";
  mainFunction << injectedBody;

  if (tmpl.hasBackground) {
    mainFunction << "    col = mix(col, " << tmpl.backgroundColor
                 << ", 0.1);\n";
  }
  mainFunction << "    fragColor = vec4(col, 1.0);\n}\n";
  return mainFunction.str();
}

} // namespace shaderLib