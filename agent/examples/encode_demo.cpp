#include "../AgentEmbedding.hpp"
#include <cstdio>
#include <string>
#include <numeric>
#include <cmath>
#include <exception>

int main(int argc, char** argv) {
  try {
    std::string model = (argc > 1)
      ? std::string(argv[1])
      : std::string("models/nomic-embed-text-v1.5.Q5_K_S.gguf");

    AgentEmbedding emb(model);

    auto q = emb.encode("cracked honeycomb mosaic, fractured glaze");
    auto d = emb.encodeDocument("honeycomb; tessellation; mosaic; shards; cellular noise");

    double dot = 0.0;
    for (size_t i = 0; i < q.v.size() && i < d.v.size(); ++i) dot += q.v[i] * d.v[i];

    std::printf("OK: q_dim=%zu d_dim=%zu cosine≈%.4f\n", q.v.size(), d.v.size(), dot);
    return 0;
  } catch (const std::exception& e) {
    std::fprintf(stderr, "ERROR: %s\n", e.what());
    return 1;
  }
}
