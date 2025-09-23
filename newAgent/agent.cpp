#include <iostream>
#include <string>
#include "third_party/nlohmann_json/single_include/nlohmann/json.hpp"
#include "third_party/llama.cpp/llama.h"

using json = nlohmann::json;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <prompt>\n";
        return 1;
    }

    std::string prompt = argv[1];
    std::string model_path = "../models/llama-2-7b-chat.Q4_K_M.gguf";  // Updated path

    // Test model loading
    json output;
    output["prompt"] = prompt;
    output["status"] = "loading_model";

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
    ctx_params.n_ctx = 512;  // Smaller context for testing
    ctx_params.seed = -1;

    llama_context* ctx = llama_new_context_with_model(model, ctx_params);
    if (!ctx) {
        output["response"] = "Failed to create context";
        output["status"] = "error";
        llama_free_model(model);
        std::cout << output.dump(2) << std::endl;
        return 1;
    }

    output["response"] = "Model loaded successfully!";
    output["status"] = "model_loaded";

    // Clean up
    llama_free(ctx);
    llama_free_model(model);

    std::cout << output.dump(2) << std::endl;
    return 0;
}