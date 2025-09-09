#pragma once
#include <stdlib.h>
#include <string>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace shaderUtility {


using Color = std::vector<float>;         // alias for "std::vector<float>". "using" seems to helpful for readability
using ColorPalette = std::vector<Color>;  // alias for "std::vector<std::vector<float>>" 

struct ShaderElement {
  
  std::string structure; ///< Valid values: "waveGrid", "noiseGrid", "circleField", "blob", "superformula", "lissajous", "lorenzAttractor", "star"
  std::string texture; ///< Valid: "abs", "perlin", "fbm"
  std::string symmetry; ///< Valid: "none", "vertical", "horizontal", "both"
  std::string layering; ///< Valid: "add", "blend", "screen", "multiply", "overlay"
  std::string colorUsage;///< Valid: "primary", "secondary", "accent", "alt"
  std::string elementBehavior; // < Valid Behaviours: 
  std::string behaviorUniform; // < Enter one of from your uniform vector. Such as u_time, etc

};
////
// STRUCTURE FOR THE WHOLE SHADER. use this to specify which code gets fetched
// from the shader string library
////
struct ShaderTemplate {
  bool hasBackground; ///< boolean
  std::string backgroundColor; ///<  (value1, value2, value3)
  ColorPalette colorPalette;
  std::vector<std::string> globalUniforms;
  std::vector<ShaderElement> elements; // vector of elements
};

// a pair of strings: top-level helpers + main() statements
struct Emitted {
  std::string helpers; // GLSL functions/defs (top-level)
  std::string calls;   // lines to paste inside main()
};

}