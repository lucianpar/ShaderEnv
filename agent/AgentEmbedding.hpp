#pragma once
#include <memory>
#include <string>
#include <vector>

struct EmbedVec { std::vector<float> v; };

class AgentEmbedding {
public:
  // gguf_model_path: path to a llama.cpp-compatible embedding model (GGUF)
  explicit AgentEmbedding(const std::string& gguf_model_path);
  ~AgentEmbedding();

  // For user requests (query side)
  EmbedVec encode(const std::string& text) const;

  // For tags / option texts (document side)
  EmbedVec encodeDocument(const std::string& text) const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
