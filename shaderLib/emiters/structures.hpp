#pragma once


#include "../../shaderLib/metaShaderUtility.hpp"

  

namespace structures {

  using ShaderElement = shaderUtility::ShaderElement;
  using Emitted       = shaderUtility::Emitted;




// THE FOLLOWING FUNCTIONS RETRIEVE STRINGS FOR COMPONENETS OF AN ELEMENT ////
inline Emitted emitElementStructure(const ShaderElement &element, int elementIndex) {
  Emitted emmitedOutput;

  std::string functionName =
        element.structure + "_" + std::to_string(elementIndex);
   std::string val = "val_" + std::to_string(elementIndex);  // <-- standard name
  
  if (element.structure == "" || 
  element.structure == " " ||
  element.structure ==  "  "){
   std::cerr << "ERROR: Element " << elementIndex << " structure is empty " << std::endl;

  }
  else if (element.structure == "waveGrid") {
    

    emmitedOutput.helpers +=
        "// below is a wave grid function\n"
        "float " +
        functionName +
        "(vec2 p) {\n"
        "  return sin(p.x + sin(p.y * 2.0) + sin(p.y * 0.43));\n"
        "}\n";

    // color mix could branch on element.colorUsage *here in C++*,
    // but the emitted GLSL is always straight-line.

    emmitedOutput.calls +=
        // "float " + functionResult + " = " + functionName +
        // "(uv);\n"
        // "// Below mixes color to form the wave grid\n"
        // "col = mix(color1, color2, " +
        // functionResult +
        // ");\n"
        // "// Above is the end of wavegrid color mixing\n";
        "float " + val + " = " + functionName + "(uv);\n";  // <- standard scalar name

  }

 else if (element.structure == "noiseGrid") {
  emmitedOutput.helpers +=
      "// Noise-based grid (pseudo-random)\n"
      "float " + functionName + "(vec2 p) {\n"
      "  return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "circleField") {
  emmitedOutput.helpers +=
      "// Circle field function\n"
      "float " + functionName + "(vec2 p) {\n"
      "  return length(p) - 0.5;\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "blob") {
  emmitedOutput.helpers +=
      "// Organic blob shape with time-based wobble\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float r = 0.5 + 0.1*sin(u_time + p.x*10.0) * cos(p.y*10.0);\n"
      "  return length(p) - r;\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "superformula") {
  emmitedOutput.helpers +=
      "// Superformula-based shape\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float m = 6.0;\n"
      "  float n1 = 0.3;\n"
      "  float n2 = 1.7;\n"
      "  float n3 = 1.7;\n"
      "  float a = 1.0, b = 1.0;\n"
      "  float phi = atan(p.y, p.x);\n"
      "  float r = pow(pow(abs(cos(m*phi/4.0)/a), n2) +\n"
      "                pow(abs(sin(m*phi/4.0)/b), n3), -1.0/n1);\n"
      "  return length(p) - r;\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "lissajous") {
  emmitedOutput.helpers +=
      "// Lissajous curve pattern\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float a = 3.0, b = 2.0;\n"
      "  float delta = PI/2.0;\n"
      "  return sin(a*p.x + delta) - sin(b*p.y);\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "lorenzAttractor") {
  emmitedOutput.helpers +=
      "// Lorenz-like attractor projection\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float sigma = 10.0;\n"
      "  float rho = 28.0;\n"
      "  float beta = 8.0/3.0;\n"
      "  vec3 v = vec3(p, 0.1);\n"
      "  for (int i = 0; i < 10; i++) {\n"
      "    vec3 dv;\n"
      "    dv.x = sigma * (v.y - v.x);\n"
      "    dv.y = v.x * (rho - v.z) - v.y;\n"
      "    dv.z = v.x * v.y - beta * v.z;\n"
      "    v += 0.01 * dv;\n"
      "  }\n"
      "  return length(v.xy);\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else if (element.structure == "star") {
  emmitedOutput.helpers +=
      "// Star polygon pattern\n"
      "float " + functionName + "(vec2 p) {\n"
      "  float a = atan(p.y,p.x);\n"
      "  float r = cos(5.0*a) * 0.5 + 0.5;\n"
      "  return length(p) - r;\n"
      "}\n";

  emmitedOutput.calls += "float " + val + " = " + functionName + "(uv);\n";
}

else {
    std::cerr << "ERROR: Inputted structure " << elementIndex << " name does not match library" << std::endl;
  }
  return emmitedOutput;
}









// placing here to test before putting in seperate modules 





}