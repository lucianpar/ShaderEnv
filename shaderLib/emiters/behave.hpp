#pragma once


#include "../../shaderLib/shaderLibUtility.hpp"

  
namespace behave {

  using ShaderElement = shaderUtility::ShaderElement;
  using Emitted       = shaderUtility::Emitted;

  inline Emitted emitElementBehavior(const ShaderElement& element, int elementIndex){

    Emitted out;

      const std::string u = element.behaviorUniform;
      const std::string val = "val_" + std::to_string(elementIndex);

      // Behaviors (add more as needed)
  if (element.elementBehavior == "scaleWith") {
    // Scale the structure response by a uniform
    out.calls += "// behavior: scaleWith(" + u + ")\n";
    out.calls += val + " *= " + u + ";\n";
  }
  else if (element.elementBehavior == "sineMod") {
    // Sinusoidal remap using the uniform as a phase driver
    out.calls += "// behavior: sineMod(" + u + ")\n";
    out.calls += val + " = 0.5 + 0.5 * sin(" + u + " + 6.28318 * " + val + ");\n";
  }
  else if (element.elementBehavior == "rotateUV") {
    // Rotate UV by angle proportional to the uniform
    out.calls += "// behavior: rotateUV(" + u + ")\n";
    out.calls += "float a = " + u + " * 0.5;\n";
    out.calls += "float c = cos(a), s = sin(a);\n";
    out.calls += "uv = mat2(c, -s, s, c) * uv;\n";
  }
  else if (element.elementBehavior == "scrollUV") {
    // Scroll UV at a rate driven by uniform
    out.calls += "// behavior: scrollUV(" + u + ")\n";
    out.calls += "uv += vec2(0.1, 0.0) * " + u + ";\n";
  }
  else if (element.elementBehavior == "threshWith") {
    // Threshold structure using uniform as a cutoff
    out.calls += "// behavior: threshWith(" + u + ")\n";
    out.calls += val + " *= step(" + u + ", " + val + ");\n";
  }
  else {
    std::cerr << "ERROR: Element " << elementIndex
              << " elementBehavior '" << element.elementBehavior
              << "' not recognized\n";
  }

  return out;
}
                            






}