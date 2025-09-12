#pragma once


#include "../../shaderLib/shaderLibUtility.hpp"

  

namespace color {

  using ShaderElement = shaderUtility::ShaderElement;
  using Emitted       = shaderUtility::Emitted;

  inline Emitted emitElementColor(const ShaderElement &element, int elementIndex) {
  Emitted out;

  const std::string idx  = std::to_string(elementIndex);
  const std::string val  = "val_" + idx;            // scalar from structure
  const std::string dest = "layerCol_" + idx;       // per-element color for layering

  if (shaderUtility::isBlank(element.colorUsage)) {
    std::cerr << "ERROR: Element " << elementIndex << " colorUsage is empty " << std::endl;
  }

  const bool known =
      element.colorUsage == "primary"   ||
      element.colorUsage == "secondary" ||
      element.colorUsage == "default";

  if (!known) {
    std::cerr << "ERROR: Inputted colorUsage " << elementIndex
              << " name does not match library" << std::endl;
  }

  // Emit color based on usage (expects uniforms color0, color1, color2)
  if (element.colorUsage == "primary") {
    out.calls +=
      "// color usage: primary -> layerCol_i\n"
      "vec3 " + dest + " = mix(color0, color1, " + val + ");\n";
  }
  else if (element.colorUsage == "secondary") {
    out.calls +=
      "// color usage: secondary -> layerCol_i\n"
      "vec3 " + dest + " = mix(color1, color2, " + val + ");\n";
  }
  else { // default
    out.calls +=
      "// color usage: default (grayscale) -> layerCol_i\n"
      "vec3 " + dest + " = vec3(" + val + ");\n";
  }

  return out;
}



}