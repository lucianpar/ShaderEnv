#include <stdlib.h>
#include <string>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/_types/_size_t.h>
#include <vector>

#include "../src/metaShaderLib.hpp"

// takes template and generates code //
// std::string generateShaderCode(const shaderLib::ShaderTemplate &tmpl) {
//   std::ostringstream glslString;

//   //
//   glslString << shaderLib::getHeader();
//   glslString << shaderLib::getUniforms(tmpl);
//   glslString << shaderLib::getColorPalette(tmpl);
  
//   // Collect helpers from selected element functions (top-level) and main body (inside main)
//   std::string helpers;
//   std::string body;

//   for (int i = 0; i < tmpl.elements.size(); ++i) {

//     const auto &element = tmpl.elements[i]; // fetches the current element - just a concise way of writing

//     shaderLib::Emitted elementOutput = shaderLib::getFullElement(tmpl.elements[i], i);
//     helpers += elementOutput.helpers;
//     body    += elementOutput.calls;
//   }

//   glslString << helpers;
//   glslString << shaderLib::getMainFunction(tmpl, body);
//   return glslString.str();
// }



// takes code from generateShaderCode and writes to a frag //
void writeShaderFile(const std::string &path, const std::string &code) {
  std::ofstream out(path);
  if (!out.is_open()) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return;
  }
  out << code;
  out.close();
  std::cout << "Shader written to: " << path << std::endl;
}

// === Main === //
int main() {

  // BEGINS NEW TEMPLATE CREATION. // this section is where you specify which
  // code bits get retrieved from the shader string library. this would
  // typically happen in a usage file or command line

  shaderLib::ShaderTemplate newTemplate;
  newTemplate.hasBackground = true;
  newTemplate.backgroundColor = "vec3(0.9, 0.03, 0.07)";
  newTemplate.colorPalette = 
    {{0.8f, 0.9f, 1.0f},
    {0.2f, 0.6f, 0.9f},
    {0.9f, 0.03f, 0.07f}}; // can have any number of color vectors inside
  newTemplate.globalUniforms = {"u_time"}; //can create any uniform 

  shaderLib::ShaderElement element1;
  element1.structure = "waveGrid";
  element1.texture = "smooth";
  element1.symmetry = "both";
  element1.layering = "blend";
  element1.colorUsage = "secondary";

  newTemplate.elements.push_back(element1);

  // ^ CONCLUDES NEW TEMPLATE CREATION. //

  std::string shaderCode = shaderLib::generateShaderCode(newTemplate);
  writeShaderFile("../shader-env/shaders/newShader4.frag", shaderCode);

  return 0;
}
