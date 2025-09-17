#pragma once
#include <string>
#include <vector>
#include <utility>

namespace agent {

// Forward-declared pimpl to keep llama.cpp details out of the header
class EmbeddingModel {
public:
    explicit EmbeddingModel(const std::string& model_path);
    ~EmbeddingModel();
    // type = "doc" or "query" (some embedders use different pooling/prompts)
    std::vector<float> embed(const std::string& text, const std::string& type) const;
    int dim() const;
private:
    struct Impl;
    Impl* impl_;
};

// A tiny in-memory index built from your docs.jsonl
class Index {
public:
    // Each item: {text, embedding}
    struct Item { std::string text; std::vector<float> vec; };

    // Load from docs.jsonl (the format your embed_file produces)
    bool loadJsonl(const std::string& path);
    // Optional: saveJsonl(path)

    // Search returns top-k {score, text} pairs (cosine)
    std::vector<std::pair<float,std::string>> search(
        const EmbeddingModel& model,
        const std::string& query,
        size_t k) const;

private:
    static float dot(const std::vector<float>& a, const std::vector<float>& b);
    static float norm(const std::vector<float>& a);
    static float cosine(const std::vector<float>& a, const std::vector<float>& b);
    std::vector<Item> items_;
};

} // namespace agent


