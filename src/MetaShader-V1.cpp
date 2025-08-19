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
std::string generateShaderCode(const shaderLib::ShaderTemplate &tmpl) {
  std::ostringstream glslString;

  //
  glslString << shaderLib::getHeader();
  glslString << shaderLib::getUniforms(tmpl);
  // 1 indexing this instead of 0 seems to function properly
  // Collect helpers (top-level) and main body (inside main)
  std::string helpers;
  std::string body;

  for (size_t i = 0; i < tmpl.elements.size(); ++i) {
    shaderLib::Emitted e =
        shaderLib::emitElement(tmpl.elements[i], static_cast<int>(i));
    helpers += e.helpers;
    body += e.calls;
  }

  glslString << helpers;
  glslString << shaderLib::getMainFunction(tmpl, body);
  return glslString.str();
}

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
  newTemplate.colorPalette = "BlueGreenPastel";
  newTemplate.globalUniforms = {"u_time"};

  shaderLib::ShaderElement element1;
  element1.structure = "waveGrid";
  element1.texture = "fbm";
  element1.symmetry = "rotate";
  element1.layering = "blend";
  element1.colorUsage = "primary";

  newTemplate.elements.push_back(element1);

  // ^ CONCLUDES NEW TEMPLATE CREATION. //

  std::string shaderCode = generateShaderCode(newTemplate);
  writeShaderFile("../shader-env/shaders/newShader4.frag", shaderCode);

  return 0;
}
