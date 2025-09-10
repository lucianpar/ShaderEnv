#pragma once
#include <stdlib.h>
#include <string>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../shaderLib/emiters/structures.hpp"
#include "../shaderLib/emiters/textures.hpp"
#include "../shaderLib/emiters/symmetry.hpp"
#include "../shaderLib/emiters/color.hpp"
#include "../shaderLib/emiters/behave.hpp"
#include "../shaderLib/shaderLibUtility.hpp"

//

// === Utility Function String Snippets === //

namespace shaderLib {


using shaderUtility::Color;
using shaderUtility::ColorPalette;
using shaderUtility::ShaderElement;  
using shaderUtility::ShaderTemplate;
using shaderUtility::Emitted;        

// INDIVIDUAL ELEMENT STRUCTURE CONTAINS THE FOLLOWING COMPONENTS. the elements
// are appended to a vector of elements. this vector is looped through, adding
// all the code to the final shader string //
/////
using structures::emitElementStructure;
using structures::emitElementPlacement;
using structures::emitElementSize;
using color::emitElementColor;
using symmetry::emitElementSymmetry;
using textures::emitElementTexture;
using behave::emitElementBehavior;


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


//assembles code from all components of element
inline Emitted getFullElement(const ShaderElement &element, int elementIndex){
    // figure out this part
    Emitted output;
    auto addToOutput = [&](const Emitted& em){ output.helpers += em.helpers; output.calls += em.calls; }; // lambda emitter outputs . [&] is a capture of an instance of the emmiter struct. 1line function for taking emmited snippet and adding to output

    //ORDER IS CRUCIAL HERE, does not matter when creating template instances

    addToOutput(emitElementSymmetry(element, elementIndex));
    addToOutput(emitElementPlacement( element, elementIndex));
    addToOutput(emitElementSize( element, elementIndex));
    addToOutput(emitElementBehavior(element, elementIndex, behave::BehaviorPhase::UV));
    addToOutput(emitElementStructure(element, elementIndex));
    addToOutput(emitElementBehavior(element, elementIndex, behave::BehaviorPhase::VAL));
    addToOutput(emitElementTexture(element, elementIndex));
    addToOutput(emitElementColor(element, elementIndex));

    return output;
    
}



/// THIS SECTION DEALS WITH THE "MAIN" FUNCTION in the glsl file //////
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