#pragma once


#include "../../shaderLib/shaderLibUtility.hpp"
#include <iomanip>

  
namespace behave {

  using ShaderElement = shaderUtility::ShaderElement;
  using Emitted       = shaderUtility::Emitted;

  inline std::string glslFloat(double x) {
  std::ostringstream ss;
  ss.setf(std::ios::fixed);
  ss << std::setprecision(6) << x; // e.g., 2.000000
  return ss.str();
}

  inline Emitted emitElementBehavior(const ShaderElement& element, int elementIndex){

    Emitted out;

      const std::string u = element.behaviorUniform;
      const std::string val = "val_" + std::to_string(elementIndex);
     // SPEED multiplier as a GLSL literal
  const std::string sp = glslFloat(element.speed);

      // Behaviors (add more as needed)
  if (element.elementBehavior == "scaleWith") {
    // scales the structure response by a (uniform * speed)
    out.calls += "// behavior: scaleWith(" + u + ") * speed=" + sp + "\n";
    out.calls += val + " *= (" + u + " * " + sp + ");\n";
  }
  else if (element.elementBehavior == "sineMod") {
    // phase driven by (uniform * speed); spatial term unchanged
    out.calls += "// behavior: sineMod(" + u + ") * speed=" + sp + "\n";
    out.calls += val + " = 0.5 + 0.5 * sin((" + u + " * " + sp + ") + 6.28318 * " + val + ");\n";
  }
  else if (element.elementBehavior == "rotateUV") {
    // rotation angle scales with (uniform * speed)
    out.calls += "// behavior: rotateUV(" + u + ") * speed=" + sp + "\n";
    out.calls += "float a = (" + u + " * " + sp + ") * 0.5;\n";
    out.calls += "float c = cos(a), s = sin(a);\n";
    out.calls += "uv = mat2(c, -s, s, c) * uv;\n";
  }
  else if (element.elementBehavior == "scrollUV") {
    // scroll rate scales with (uniform * speed)
    out.calls += "// behavior: scrollUV(" + u + ") * speed=" + sp + "\n";
    out.calls += "uv += vec2(0.1, 0.0) * (" + u + " * " + sp + ");\n";
  }
  else if (element.elementBehavior == "threshWith") {
    // threshold moves with (uniform * speed)
    out.calls += "// behavior: threshWith(" + u + ") * speed=" + sp + "\n";
    out.calls += val + " *= step((" + u + " * " + sp + "), " + val + ");\n";
  }
  else {
    std::cerr << "ERROR: Element " << elementIndex
              << " elementBehavior '" << element.elementBehavior
              << "' not recognized\n";
  }

  return out;
}
                            






}