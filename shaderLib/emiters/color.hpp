#pragma once


#include "../../shaderLib/shaderLibUtility.hpp"

  

namespace color {

  using ShaderElement = shaderUtility::ShaderElement;
  using Emitted       = shaderUtility::Emitted;

  inline Emitted emitElementColor   (const ShaderElement &element, int elementIndex){
  Emitted out;
  std::string val = "val_" + std::to_string(elementIndex);

  if (shaderUtility::isBlank(element.colorUsage)) {
     std::cerr << "ERROR: Element " << elementIndex << " colorUsage is empty " << std::endl;

    }
      const bool known =
          element.colorUsage == "primary"   ||
          element.colorUsage == "secondary" ||
          element.colorUsage == "default";
     if (!known) {
        std::cerr << "ERROR: Inputted colorUsage " << elementIndex << " name does not match library" << std::endl;
    }

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



}