#pragma once


#include "../../shaderLib/metaShaderUtility.hpp"

  

namespace color {

  using ShaderElement = shaderUtility::ShaderElement;
  using Emitted       = shaderUtility::Emitted;

  inline Emitted emitElementColor   (const ShaderElement &element, int elementIndex){
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



}