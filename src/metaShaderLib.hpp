
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

  // placeholder examples
  else if (element.structure == "noiseGrid") {
    // emit noiseGrid
  } else if (element.structure == "circleField") {
    // emit circleField
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
    mainFunction << "    col = mix(col, " << tmpl.backgroundColor
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



} // namespace shaderLib