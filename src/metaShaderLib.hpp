
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
  
  std::string structure; ///< Valid values: "waveGrid", "noiseGrid", "circleField", "blob", "superformula", "lissajous", "lorenzAttractor", "star"
  std::string texture; ///< Valid: "abs", "perlin", "fbm"
  std::string symmetry; ///< Valid: "none", "vertical", "horizontal", "both"
  std::string layering; ///< Valid: "add", "blend", "screen", "multiply", "overlay"
  std::string colorUsage;///< Valid: "primary", "secondary", "accent", "alt"
};
////
// STRUCTURE FOR THE WHOLE SHADER. use this to specify which code gets fetched
// from the shader string library
////
struct ShaderTemplate {
  bool hasBackground; ///< boolean
  std::string backgroundColor; ///<  (value1, value2, value3)
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



// THE FOLLOWING FUNCTIONS RETRIEVE STRINGS FOR COMPONENETS OF AN ELEMENT ////
Emitted emitElementStructure(const ShaderElement &element, int elementIndex) {
  Emitted emmitedOutput;

  std::string functionName =
        element.structure + "_" + std::to_string(elementIndex);
   std::string val = "val_" + std::to_string(elementIndex);  // <-- standard name
  if (element.structure == "waveGrid") {
    

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
        "float " + val + " = " + functionName + "(uv);\n";  // <- standard scalar name

  }

 else if (element.structure == "noiseGrid") {
  emmitedOutput.helpers +=
      "// Noise-based grid (pseudo-random)\n"
      "float " + functionName + "(vec2 p) {\n"
      "  return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "circleField") {
  emmitedOutput.helpers +=
      "// Circle field function\n"
      "float " + functionName + "(vec2 p) {\n"
      "  return length(p) - 0.5;\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "blob") {
  emmitedOutput.helpers +=
      "// Organic blob shape with time-based wobble\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float r = 0.5 + 0.1*sin(u_time + p.x*10.0) * cos(p.y*10.0);\n"
      "  return length(p) - r;\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
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

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "lissajous") {
  emmitedOutput.helpers +=
      "// Lissajous curve pattern\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float a = 3.0, b = 2.0;\n"
      "  float delta = PI/2.0;\n"
      "  return sin(a*p.x + delta) - sin(b*p.y);\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
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

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "star") {
  emmitedOutput.helpers +=
      "// Star polygon pattern\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float a = atan(p.y,p.x);\n"
      "  float r = cos(5.0*a) * 0.5 + 0.5;\n"
      "  return length(p) - r;\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}
  return emmitedOutput;
}

Emitted emitElementSymmetry(const ShaderElement &element, int elementIndex) {
  Emitted emmitedOutput;

  if (element.symmetry == "horizontal") {
    emmitedOutput.calls +=
        "// Apply horizontal symmetry to UVs\n"
        "uv.x = abs(uv.x);\n";
  }
  else if (element.symmetry == "vertical") {
    emmitedOutput.calls +=
        "// Apply vertical symmetry to UVs\n"
        "uv.y = abs(uv.y);\n";
  }
  else if (element.symmetry == "both") {
    emmitedOutput.calls +=
        "// Apply 4-way symmetry to UVs\n"
        "uv = abs(uv);\n";
  }

  return emmitedOutput;
}


Emitted emitElementTexture(const ShaderElement &element, int elementIndex) {
  Emitted out;
  std::string val = "val_" + std::to_string(elementIndex);

 if (element.texture == "abs") {
  out.calls += "// texture: abs\n";
  out.calls += val + " = abs(" + val + ");\n";
}
else if (element.texture == "pow2") {
  out.calls += "// texture: power curve (x^2)\n";
  out.calls += val + " = " + val + " * " + val + ";\n";
}
else if (element.texture == "smooth") {
  out.calls += "// texture: smoothstep shaping\n";
  out.calls += val + " = smoothstep(0.0, 1.0, " + val + ");\n";
}
else if (element.texture == "fbm") {
  std::string fn = "fbmTone_" + std::to_string(elementIndex);
  out.helpers +=
    "// texture: fbm tone-map (placeholder)\n"
    "float " + fn + "(float x){ return 0.5 + 0.5*sin(6.28318*x + 2.0*x); }\n";
  out.calls += val + " = " + fn + "(" + val + ");\n";
}

  return out;
}

Emitted emitElementColor(const ShaderElement &element, int elementIndex) {
  Emitted out;
  std::string val = "val_" + std::to_string(elementIndex);

  if (element.colorUsage == "primary") {
    out.calls +=
      "// color usage: primary\n"
      "col = mix(color0, color1, " + val + ");\n";
  } else if (element.colorUsage == "secondary") {
    out.calls +=
      "// color usage: secondary\n"
      "col = mix(color1, color2, " + val + ");\n";
  } else {
    out.calls +=
      "// color usage: default (grayscale add)\n"
      "col += vec3(" + val + ");\n";
  }
  return out;
}





Emitted getFullElement(const ShaderElement &element, int elementIndex){
    // figure out this part
    Emitted output;
    auto addToOutput = [&](const Emitted& em){ output.helpers += em.helpers; output.calls += em.calls; }; // lambda emitter outputs . [&] is a capture of an instance of the emmiter struct. 1line function for taking emmited snippet and adding to output

    addToOutput(emitElementSymmetry(element, elementIndex));
    addToOutput(emitElementStructure(element, elementIndex));
    addToOutput(emitElementTexture(element, elementIndex));
    addToOutput(emitElementColor(element, elementIndex));
    

    return output;
    
}











/// THIS SECTION DEALS WITH THE MAIN FUNCTION //////
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
    mainFunction << "    col = mix(col, vec3" << tmpl.backgroundColor
                 << ", 0.1);\n";
  }
  mainFunction << "    fragColor = vec4(col, 1.0);\n}\n";
  return mainFunction.str();
}



// MASTER FUNCTION FOR GENERATING CODE /// 
std::string generateShaderCode(const shaderLib::ShaderTemplate &tmpl) {
  std::ostringstream glslString;

  //
  glslString << shaderLib::getHeader();
  glslString << shaderLib::getUniforms(tmpl);
  glslString << shaderLib::getColorPalette(tmpl);
  
  // Collect helpers from selected element functions (top-level) and main body (inside main)
  std::string helpers;
  std::string body;

  for (int i = 0; i < tmpl.elements.size(); ++i) {

    const auto &element = tmpl.elements[i]; // fetches the current element - just a concise way of writing

    shaderLib::Emitted elementOutput = shaderLib::getFullElement(tmpl.elements[i], i);
    helpers += elementOutput.helpers;
    body    += elementOutput.calls;
  }

  glslString << helpers;
  glslString << shaderLib::getMainFunction(tmpl, body);
  return glslString.str();
}

// takes code from generateShaderCode and writes to a frag //
void writeShaderFile(const std::string &path, const std::string &code) {
  std::ofstream out(path);
  if (!out.is_open()) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return;
  }
  out << code;
  out.close();
  std::cout << "Shader written to: " << path << std::endl;
}



} // namespace shaderLib