#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../third_party/nlohmann/json.hpp"
#include "AgentEmbedding.hpp"

using json = nlohmann::json;
static std::vector<float> mean_vec(const std::vector<std::vector<float>>& xs) {
    if (xs.empty()) return {};
    const int d = (int)xs[0].size();
    std::vector<float> m(d, 0.0f);
    for (auto& v : xs) for (int i = 0; i < d; ++i) m[i] += v[i];
    for (int i = 0; i < d; ++i) m[i] /= float(xs.size());
    // L2-normalize the mean (good for cosine similarity later)
    double s = 0.0; for (float v : m) s += (double)v * (double)v;
    if (s > 0) { float inv = 1.0f / float(std::sqrt(s)); for (auto& v : m) v *= inv; }
    return m;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "usage: " << argv[0]
                  << " <model.gguf> <shader_vocab.json> <out.jsonl>\n";
        return 1;
    }
    const std::string model_path = argv[1];
    const std::string vocab_json = argv[2];
    const std::string out_jsonl  = argv[3];

    // Load JSON spec
    json spec;
    {
        std::ifstream in(vocab_json);
        if (!in) { std::cerr << "failed to open " << vocab_json << "\n"; return 1; }
        in >> spec;
    }

    // Init embedder
    AgentEmbedding emb(model_path);

    std::ofstream out(out_jsonl);
    if (!out) { std::cerr << "failed to open " << out_jsonl << " for write\n"; return 1; }

    int written = 0, skipped = 0;

    // Expect: { "structure": { "quasicrystal": { "pos":[...], "neg":[...] }, ... }, "texture": { ... }, ... }
    for (auto itCat = spec.begin(); itCat != spec.end(); ++itCat) {
        if (!itCat.value().is_object()) continue;
        const std::string category = itCat.key();
        for (auto itOpt = itCat.value().begin(); itOpt != itCat.value().end(); ++itOpt) {
            const std::string option = itOpt.key();
            const json& cfg = itOpt.value();

            // collect pos/neg phrases
            std::vector<std::string> pos, neg;
            if (cfg.contains("pos") && cfg["pos"].is_array())
                for (auto& s : cfg["pos"]) pos.push_back(s.get<std::string>());
            if (cfg.contains("neg") && cfg["neg"].is_array())
                for (auto& s : cfg["neg"]) neg.push_back(s.get<std::string>());

            if (pos.empty() && neg.empty()) {
                std::cerr << "[skip] " << category << ":" << option << " — no pos/neg phrases\n";
                ++skipped; continue;
            }

            // embed phrases
            std::vector<std::vector<float>> pos_vecs, neg_vecs;
            for (auto& s : pos) pos_vecs.push_back(emb.encode(s).v);  // query-style
            for (auto& s : neg) neg_vecs.push_back(emb.encode(s).v);

            // mean vectors
            auto mean_pos = mean_vec(pos_vecs);
            auto mean_neg = mean_vec(neg_vecs);

            json line;
            line["tag"] = category + ":" + option;
            if (!mean_pos.empty()) line["mean_pos"] = mean_pos;
            if (!mean_neg.empty()) line["mean_neg"] = mean_neg;

            out << line.dump() << "\n";
            ++written;
        }
    }

    out.close();
    std::cerr << "wrote " << written << " tags, skipped " << skipped << "\n";
    return 0;
}