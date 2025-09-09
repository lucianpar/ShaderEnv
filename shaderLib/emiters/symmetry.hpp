#pragma once


#include "../../shaderLib/metaShaderUtility.hpp"

 namespace symmetry {
using ShaderElement = shaderUtility::ShaderElement;
using Emitted       = shaderUtility::Emitted;

inline Emitted emitElementSymmetry(const ShaderElement &element, int elementIndex){
  Emitted emmitedOutput;

if (element.symmetry == "" || 
  element.symmetry == " " ||
  element.symmetry ==  "  "){
   std::cerr << "ERROR: Element " << elementIndex << " symmetry is empty " << std::endl;

  }
  else if (element.symmetry == "horizontal") {
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

  else {
    std::cerr << "ERROR: Inputted symmetry " << elementIndex << " name does not match library" << std::endl;
  }
  

  return emmitedOutput;
}

 }