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

// One function, two phases
enum class BehaviorPhase { UV, VAL };

// helper 
inline bool isBlank(const std::string& s) {
  return s.empty() || s == " " || s == "  ";
}

// helper -  verify the uniform was declared
// inline bool hasUniform(const shaderUtility::ShaderTemplate& tmpl,
//                        const std::string& name) {
//   return std::find(tmpl.globalUniforms.begin(),
//                    tmpl.globalUniforms.end(), name) != tmpl.globalUniforms.end();
// }

// main behavior function 
inline Emitted emitElementBehavior(const ShaderElement& element,
                                   int elementIndex,
                                   BehaviorPhase phase) {
  Emitted out;

  const std::string u   = element.behaviorUniform;
  const std::string val = "val_" + std::to_string(elementIndex);   // <-- standard name
  const std::string uvi = "uv_"  + std::to_string(elementIndex);   // uv specific to this element - important for element size and placement
  const std::string sp  = glslFloat(element.speed);                 // SPEED multiplier as a GLSL literal

  // No behavior? No-op.
  if (isBlank(element.elementBehavior)) return out;

  // Require a uniform for all current behaviors
  if (isBlank(u)) {
    std::cerr << "ERROR: Element " << elementIndex
              << " behaviorUniform is empty while elementBehavior is '"
              << element.elementBehavior << "'" << std::endl;
    return out;
  }
  // Optional but helpful: ensure uniform exists in template
//   if (!hasUniform(tmpl, u)) {
//     std::cerr << "ERROR: Element " << elementIndex
//               << " behaviorUniform '" << u << "' is not in globalUniforms" << std::endl;
//     return out;
//   }

  // Recognized behaviors (union of both phases)
  const bool known =
      element.elementBehavior == "scaleWith" ||
      element.elementBehavior == "sineMod"   ||
      element.elementBehavior == "rotateUV"  ||
      element.elementBehavior == "scrollUV"  ||
      element.elementBehavior == "threshWith";

  if (!known) {
    std::cerr << "ERROR: Element " << elementIndex
              << " elementBehavior '" << element.elementBehavior
              << "' not recognized" << std::endl;
    return out;
  }

  // === UV behaviors (run BEFORE structure; modify uv_i) ===
  if (phase == BehaviorPhase::UV) {
    if (element.elementBehavior == "scrollUV") {
      // scroll rate scales with (uniform * speed)
      out.calls += "// behavior: scrollUV(" + u + ") * speed=" + sp + "\n";
      out.calls += uvi + " += vec2(0.1, 0.0) * (" + u + " * " + sp + ");\n";
    } else if (element.elementBehavior == "rotateUV") {
      // rotation angle scales with (uniform * speed)
      const std::string idx = std::to_string(elementIndex);
      const std::string a   = "a_" + idx;
      const std::string c   = "c_" + idx;
      const std::string s   = "s_" + idx;
      out.calls += "// behavior: rotateUV(" + u + ") * speed=" + sp + "\n";
      out.calls += "float " + a + " = (" + u + " * " + sp + ") * 0.5;\n";
      out.calls += "float " + c + " = cos(" + a + "), " + s + " = sin(" + a + ");\n";
      out.calls += uvi + " = mat2(" + c + ", -" + s + ", " + s + ", " + c + ") * " + uvi + ";\n";
    }
    // Value-only behaviors are ignored in this phase.
    return out;
  }

  // === Value behaviors (run AFTER structure; modify val_i) ===
  if (phase == BehaviorPhase::VAL) {
    if (element.elementBehavior == "scaleWith") {
      // scales the structure response by a (uniform * speed)
      out.calls += "// behavior: scaleWith(" + u + ") * speed=" + sp + "\n";
      out.calls += val + " *= (" + u + " * " + sp + ");\n";
    } else if (element.elementBehavior == "sineMod") {
      // phase driven by (uniform * speed); spatial term unchanged
      out.calls += "// behavior: sineMod(" + u + ") * speed=" + sp + "\n";
      out.calls += val + " = 0.5 + 0.5 * sin((" + u + " * " + sp + ") + 6.28318 * " + val + ");\n";
    } else if (element.elementBehavior == "threshWith") {
      // threshold moves with (uniform * speed)
      out.calls += "// behavior: threshWith(" + u + ") * speed=" + sp + "\n";
      out.calls += val + " *= step((" + u + " * " + sp + "), " + val + ");\n";
    }
    // UV-only behaviors are ignored in this phase.
    return out;
  }

  return out;
}

                         






}