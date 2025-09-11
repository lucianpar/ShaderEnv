#pragma once


#include "../../shaderLib/shaderLibUtility.hpp"

 namespace textures {
using ShaderElement = shaderUtility::ShaderElement;
using Emitted       = shaderUtility::Emitted;

inline Emitted emitElementTexture (const ShaderElement &element, int elementIndex) {
  Emitted out;
  std::string val = "val_" + std::to_string(elementIndex);
if (element.texture == "" || 
  element.texture == " " ||
  element.texture ==  "  "){
   std::cerr << "ERROR: Element " << elementIndex << " texture is empty " << std::endl;

  }
 else if (element.texture == "abs") {
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

else if (element.texture == "none") {
  out.calls += "// texture: none (no-op)\n";
}

// add back in perlin

  else {
    std::cerr << "ERROR: Inputted texture " << elementIndex << "name does not match library" << std::endl;
  }
  return out;
}

 }