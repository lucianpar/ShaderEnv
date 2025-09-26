#include <iostream>
#include <string>
#include <vector>
#include "third_party/nlohmann_json/single_include/nlohmann/json.hpp"
#include "third_party/llama.cpp/llama.h"
#include "structure_reference.hpp"
#include "agentOutput.hpp"  // New include

using json = nlohmann::json;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <prompt>\n";
        return 1;
    }

    std::string prompt = argv[1];
    std::string model_path = "../models/llama-2-7b-chat.Q4_K_M.gguf";
    std::string json_path = "../reference/structures_reference.json";

    json output;
    output["prompt"] = prompt;
    output["status"] = "loading_model";

    // Load structure reference
    StructureReference ref(json_path);

    // Step 1: Load the model
    llama_model_params model_params = llama_model_default_params();
    llama_model* model = llama_load_model_from_file(model_path.c_str(), model_params);
    if (!model) {
        output["response"] = "Failed to load model: " + model_path;
        output["status"] = "error";
        std::cout << output.dump(2) << std::endl;
        return 1;
    }

    // Step 2: Create context
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 512;
    ctx_params.seed = -1;

    llama_context* ctx = llama_new_context_with_model(model, ctx_params);
    if (!ctx) {
        output["response"] = "Failed to create context";
        output["status"] = "error";
        llama_free_model(model);
        std::cout << output.dump(2) << std::endl;
        return 1;
    }

    // Step 3: Enhance prompt with structure context
    std::string enhanced_prompt = ref.getAllStructuresContext() + "\nUser Prompt: " + prompt + "\nBased on the available structures, suggest or generate GLSL code and fill out a ShaderTemplate with appropriate values.";

    // Step 4: Tokenize the enhanced prompt
    std::vector<llama_token> tokens(enhanced_prompt.size() + 1);
    int n_tokens = llama_tokenize(
        model,
        enhanced_prompt.c_str(),
        (int32_t)enhanced_prompt.length(),
        tokens.data(),
        (int32_t)tokens.size(),
        true,
        false
    );
    
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(
            model,
            enhanced_prompt.c_str(),
            (int32_t)enhanced_prompt.length(),
            tokens.data(),
            (int32_t)tokens.size(),
            true,
            false
        );
    }
    tokens.resize(n_tokens);

    // Placeholder for model response (replace with actual inference later)
    std::string model_response = "structure: waveGrid\nsize: 0.5\ntexture: perlin\nsymmetry: vertical\nlayering: blend\ncolorUsage: primary\nelementBehavior: sineMod\nbehaviorUniform: u_time\nspeed: 2.0";  // Example response

    // Step 5: Create ShaderTemplate from model response
    shaderUtility::ShaderTemplate shader_template = agentOutput::createShaderTemplate(model_response);

    // Add template to output JSON
    output["shader_template"] = {
        {"hasBackground", shader_template.hasBackground},
        {"backgroundColor", shader_template.backgroundColor},
        {"colorPalette", shader_template.colorPalette},
        {"globalUniforms", shader_template.globalUniforms},
        {"elements", json::array()}
    };
    for (const auto& elem : shader_template.elements) {
        output["shader_template"]["elements"].push_back({
            {"structure", elem.structure},
            {"size", elem.size},
            {"placementCoords", elem.placementCoords},
            {"texture", elem.texture},
            {"symmetry", elem.symmetry},
            {"layering", elem.layering},
            {"colorUsage", elem.colorUsage},
            {"elementBehavior", elem.elementBehavior},
            {"behaviorUniform", elem.behaviorUniform},
            {"speed", elem.speed}
        });
    }

    output["response"] = "Model loaded, structures referenced, prompt tokenized, and ShaderTemplate generated!";
    output["status"] = "template_generated";
    output["enhanced_prompt"] = enhanced_prompt;
    output["token_count"] = n_tokens;
    output["model_response_example"] = model_response;  // For debugging

    // Clean up
    llama_free(ctx);
    llama_free_model(model);

    std::cout << output.dump(2) << std::endl;
    return 0;
}