#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include "../shaderLib/ShaderLibUtility.hpp"  // Adjust path if needed

namespace agentOutput {

using namespace shaderUtility;

// Helper function to extract a value from a string (simple keyword-based parsing)
std::string extractValue(const std::string& response, const std::string& keyword) {
    size_t pos = response.find(keyword);
    if (pos != std::string::npos) {
        size_t start = response.find(":", pos);
        if (start != std::string::npos) {
            size_t end = response.find("\n", start);
            if (end == std::string::npos) end = response.length();
            std::string value = response.substr(start + 1, end - start - 1);
            // Trim whitespace
            value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), value.end());
            return value;
        }
    }
    return "";  // Default empty
}

// Function to create a ShaderElement from response text
ShaderElement createShaderElement(const std::string& response) {
    ShaderElement element;
    
    // Parse structure (default to "waveGrid" if not found)
    std::string structStr = extractValue(response, "structure");
    if (!structStr.empty() && (structStr == "waveGrid" || structStr == "noiseGrid" || structStr == "circleField" || 
                               structStr == "blob" || structStr == "superformula" || structStr == "lissajous" || 
                               structStr == "lorenzAttractor" || structStr == "star" || structStr == "mandalaRadial" || 
                               structStr == "quasicrystal" || structStr == "voronoi" || structStr == "roseCurve" || 
                               structStr == "superellipse" || structStr == "phyllotaxis" || structStr == "julia" || 
                               structStr == "reactionDiffusion" || structStr == "branchNoise")) {
        element.structure = structStr;
    } else {
        element.structure = "waveGrid";  // Default
    }
    
    // Parse size (default 1.0)
    std::string sizeStr = extractValue(response, "size");
    if (!sizeStr.empty()) {
        try { element.size = std::stof(sizeStr); } catch (...) { element.size = 1.0f; }
    }
    
    // Parse placementCoords (default {0.0, 0.0})
    std::string coordsStr = extractValue(response, "placementCoords");
    if (!coordsStr.empty()) {
        std::stringstream ss(coordsStr);
        double x, y;
        if (ss >> x >> y) {
            element.placementCoords = {x, y};
        }
    }
    
    // Parse texture (default "abs")
    std::string textureStr = extractValue(response, "texture");
    if (!textureStr.empty() && (textureStr == "abs" || textureStr == "perlin" || textureStr == "fbm")) {
        element.texture = textureStr;
    } else {
        element.texture = "abs";
    }
    
    // Parse symmetry (default "none")
    std::string symmetryStr = extractValue(response, "symmetry");
    if (!symmetryStr.empty() && (symmetryStr == "none" || symmetryStr == "vertical" || symmetryStr == "horizontal" || symmetryStr == "both")) {
        element.symmetry = symmetryStr;
    } else {
        element.symmetry = "none";
    }
    
    // Parse layering (default "add")
    std::string layeringStr = extractValue(response, "layering");
    if (!layeringStr.empty() && (layeringStr == "add" || layeringStr == "blend" || layeringStr == "screen" || 
                                 layeringStr == "multiply" || layeringStr == "overlay")) {
        element.layering = layeringStr;
    } else {
        element.layering = "add";
    }
    
    // Parse colorUsage (default "primary")
    std::string colorStr = extractValue(response, "colorUsage");
    if (!colorStr.empty() && (colorStr == "primary" || colorStr == "secondary" || colorStr == "accent" || colorStr == "alt")) {
        element.colorUsage = colorStr;
    } else {
        element.colorUsage = "primary";
    }
    
    // Parse elementBehavior (default "scaleWith")
    std::string behaviorStr = extractValue(response, "elementBehavior");
    if (!behaviorStr.empty() && (behaviorStr == "scaleWith" || behaviorStr == "sineMod" || behaviorStr == "rotateUV" || 
                                 behaviorStr == "scrollUV" || behaviorStr == "threshWith")) {
        element.elementBehavior = behaviorStr;
    } else {
        element.elementBehavior = "scaleWith";
    }
    
    // Parse behaviorUniform (default "")
    element.behaviorUniform = extractValue(response, "behaviorUniform");
    
    // Parse speed (default 1.0)
    std::string speedStr = extractValue(response, "speed");
    if (!speedStr.empty()) {
        try { element.speed = std::stod(speedStr); } catch (...) { element.speed = 1.0; }
    }
    
    return element;
}

// Function to create a ShaderTemplate from response text
ShaderTemplate createShaderTemplate(const std::string& response) {
    ShaderTemplate template_;
    
    // Parse hasBackground (default false)
    std::string bgStr = extractValue(response, "hasBackground");
    template_.hasBackground = (bgStr == "true" || bgStr == "1");
    
    // Parse backgroundColor (default "")
    template_.backgroundColor = extractValue(response, "backgroundColor");
    
    // Parse colorPalette (default empty)
    // For simplicity, assume a basic palette; expand if needed
    template_.colorPalette = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};  // Example: red and green
    
    // Parse globalUniforms (default empty vector)
    // This could be expanded to parse a list
    template_.globalUniforms = {"u_time"};  // Example default
    
    // Parse elements (create one element for now; expand to parse multiple)
    ShaderElement element = createShaderElement(response);
    template_.elements.push_back(element);
    
    return template_;
}

}  // namespace agentOutput