
#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_ControlNav.hpp"
#include "al/io/al_File.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_FileSelector.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <ostream>
#include <string>

#include "shader-env/shaderUtility/shaderToSphere.hpp"
#include "../shaderLib/shaderLibMaster.hpp"

struct Common {};
class MyApp : public al::DistributedAppWithState<Common> {
public:
  al::FileSelector selector;
  al::SearchPaths searchPaths;
  ShadedSphere shadedSphere;

  std::string vertPath;
  std::string fragPath;
  al::Parameter globalTime{"globalTime", "", 0.0, 0.0, 300.0};

  al::ParameterBool running{"running", "0", true};

  void onInit() override {
    // gam::sampleRate(audioIO().framesPerSecond());

    searchPaths.addSearchPath(
        al::File::currentPath() +
        "/../shader-env/shaders/"); // replace with relative folder

    al::FilePath vertPathSource = searchPaths.find("standard.vert");
    if (vertPathSource.valid()) {
      vertPath = vertPathSource.filepath();
      std::cout << "Found file at: " << vertPath << std::endl;
    } else {
      std::cout << "couldnt find vert scene 5 in path" << std::endl;
    }
    al::FilePath fragPathSource = searchPaths.find("updatedTestShader.frag");
    if (fragPathSource.valid()) {
      fragPath = fragPathSource.filepath();
      std::cout << "Found file at: " << fragPath << std::endl;
    } else {
      std::cout << "couldnt find frag scene 5 in path" << std::endl;
    }
  }

  void onCreate() override {
    shadedSphere.setSphere(15.0, 20);
    shadedSphere.setShaders(vertPath, fragPath);
    shadedSphere.update();
  }

  void onAnimate(double dt) override {
    if (running == true) {
      globalTime = globalTime + dt;
      //std::cout << globalTime << std::endl;
    }
  }

  void onDraw(al::Graphics &g) override {

    g.clear(0.0);

    g.shader(shadedSphere.shader());
    shadedSphere.setUniformFloat("u_time", globalTime);
    shadedSphere.draw(g);
  };

  bool onKeyDown(const al::Keyboard &k) override {

    if (isPrimary()) {

      if (k.key() == ' ' && running == false) {
        running = true;
        std::cout << "started running" << std::endl;
      } else if (k.key() == ' ' && running == true) {
        running = false;
        std::cout << "stopped running" << std::endl;
      }
    }
  }
};
int main() {
  // BEGINS NEW TEMPLATE CREATION. // this section is where you specify which
  // code bits get retrieved from the shader string library. this would
  // typically happen in a usage file or command line

  shaderLib::ShaderTemplate newTemplate;
  newTemplate.hasBackground = true;
  newTemplate.backgroundColor = "(0.9, 0.03, 0.07)";
  newTemplate.colorPalette = 
    {{0.8f, 0.9f, 1.0f},
    {0.2f, 0.6f, 0.9f},
    {0.9f, 0.03f, 0.07f}}; // can have any number of color vectors inside
  newTemplate.globalUniforms = {"u_time"}; //can create any uniform 

  shaderLib::ShaderElement element1;
  element1.structure        = "phyllotaxis";
element1.placementCoords  = {1.0, 0.0};
element1.size             = 0.5f;
element1.texture          = "abs";        // was "perlin"
element1.symmetry         = "horizontal"; 
element1.layering         = "add";
element1.colorUsage       = "primary";
element1.behaviorUniform  = "u_time";
element1.elementBehavior  = "sineMod";
element1.speed            = 2.0;

shaderLib::ShaderElement element2;
element2.structure        = "mandalaRadial";
element2.texture          = "fbm";
element2.symmetry         = "both";
element2.layering         = "overlay";
element2.colorUsage       = "primary";  // "accent" will just fall back; switch if you want the defined path
element2.behaviorUniform  = "u_time";
// element2.elementBehavior  = "rotateUV ";
// element2.speed            = 0.5;

   newTemplate.elements.push_back(element1);
  newTemplate.elements.push_back(element2);

  // ^ CONCLUDES NEW TEMPLATE CREATION. //

  std::string shaderCode = shaderLib::generateShaderCode(newTemplate);
  shaderLib::writeShaderFile("../shader-env/shaders/updatedTestShader.frag", shaderCode);
  MyApp app;
  app.start();
  return 0;
}
