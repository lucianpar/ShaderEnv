#pragma once


#include "../../shaderLib/shaderLibUtility.hpp"

 namespace symmetry {
using ShaderElement = shaderUtility::ShaderElement;
using Emitted       = shaderUtility::Emitted;

inline Emitted emitElementSymmetry(const ShaderElement &element, int elementIndex){
  Emitted emmitedOutput;

if (shaderUtility::isBlank(element.symmetry)) {
   std::cerr << "ERROR: Element " << elementIndex << " symmetry is empty " << std::endl;

  }
    const bool known =
        element.symmetry == "horizontal" ||
        element.symmetry == "vertical"   ||
        element.symmetry == "both"       ||
        element.symmetry == "none";
   if (!known) {
      std::cerr << "ERROR: Inputted symmetry " << elementIndex << " name does not match library" << std::endl;
  }
  
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
  else if (element.symmetry == "none") {
  // No symmetry applied
  emmitedOutput.calls += "// symmetry: none (no-op)\n";
}

  
  

  return emmitedOutput;
}

 }