#pragma once

#include "../../shaderLib/shaderLibUtility.hpp"

  

namespace layering {

  using ShaderElement = shaderUtility::ShaderElement;
  using Emitted       = shaderUtility::Emitted;

  inline Emitted emitElementLayering(const ShaderElement& element, int elementIndex) {
  Emitted out;

  const std::string idx   = std::to_string(elementIndex);
  const std::string mask  = "val_" + idx;        // alpha/mask from structure
  const std::string src   = "layerCol_" + idx;   // per-element color
  const std::string accum = "col";               // global accumulator

  if (shaderUtility::isBlank(element.layering)) {
    std::cerr << "ERROR: Element " << elementIndex << " layering is empty" << std::endl;
  }

  // Valid layering modes
  const bool known =
      element.layering == "add"     ||
      element.layering == "blend"   ||
      element.layering == "screen"  ||
      element.layering == "multiply"||
      element.layering == "overlay";

  if (!known) {
    std::cerr << "ERROR: Element " << elementIndex
              << " layering mode \"" << element.layering
              << "\" not recognized" << std::endl;
  }

  // Per-index overlay helper (so functions don’t clash)
  if (element.layering == "overlay") {
    out.helpers +=
      "vec3 blendOverlay_" + idx + "(vec3 b, vec3 s) {\n"
      "  vec3 lo = 2.0 * b * s;\n"
      "  vec3 hi = 1.0 - 2.0 * (1.0 - b) * (1.0 - s);\n"
      "  return mix(lo, hi, step(0.5, b));\n"
      "}\n";
  }

  // Calls: composite src into col based on layering mode
  if (element.layering == "add") {
    out.calls += accum + " = mix(" + accum + ", " + accum + " + " + src + ", " + mask + ");\n";
  }
  else if (element.layering == "blend") {
    out.calls += accum + " = mix(" + accum + ", " + src + ", " + mask + ");\n";
  }
  else if (element.layering == "screen") {
    out.calls += accum + " = mix(" + accum + ", 1.0 - (1.0 - " + accum + ")*(1.0 - " + src + "), " + mask + ");\n";
  }
  else if (element.layering == "multiply") {
    out.calls += accum + " = mix(" + accum + ", " + accum + " * " + src + ", " + mask + ");\n";
  }
  else if (element.layering == "overlay") {
    out.calls += accum + " = mix(" + accum + ", blendOverlay_" + idx + "(" + accum + ", " + src + "), " + mask + ");\n";
  }

  return out;
}




}