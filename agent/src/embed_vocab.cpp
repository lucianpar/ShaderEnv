#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../third_party/nlohmann/json.hpp"
#include "AgentEmbedding.hpp"

using json = nlohmann::json;

static void add_inplace(std::vector<float>& a, const std::vector<float>& b) {
    if (a.empty()) a.resize(b.size(), 0.0f);
    if (a.size() != b.size()) throw std::runtime_error("dim mismatch in add_inplace");
    for (size_t i = 0; i < a.size(); ++i) a[i] += b[i];
}
static void sub_inplace(std::vector<float>& a, const std::vector<float>& b) {
    if (a.empty()) a.resize(b.size(), 0.0f);
    if (a.size() != b.size()) throw std::runtime_error("dim mismatch in sub_inplace");
    for (size_t i = 0; i < a.size(); ++i) a[i] -= b[i];
}
static void scale_inplace(std::vector<float>& a, float s) {
    for (auto& x : a) x *= s;
}
static void normalize_inplace(std::vector<float>& a) {
    double s = 0.0;
    for (float x : a) s += double(x) * double(x);
    float n = float(std::sqrt(s));
    if (n > 0.0f) for (auto& x : a) x /= n;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "usage: " << argv[0]
                  << " <model.gguf> <vocab.jsonl> <out.jsonl>\n";
        return 1;
    }
    const std::string model_path = argv[1];
    const std::string in_path    = argv[2];
    const std::string out_path   = argv[3];

    try {
        AgentEmbedding emb(model_path);

        std::ifstream fin(in_path);
        if (!fin) {
            std::cerr << "failed to open " << in_path << "\n";
            return 1;
        }
        std::ofstream fout(out_path);
        if (!fout) {
            std::cerr << "failed to open " << out_path << " for write\n";
            return 1;
        }

        std::string line;
        size_t line_no = 0, wrote = 0;
        while (std::getline(fin, line)) {
            ++line_no;
            // skip blanks and comments
            bool blank = true;
            for (char c : line) { if (!std::isspace((unsigned char)c)) { blank = false; break; } }
            if (blank || (!line.empty() && line[0] == '#')) continue;

            json j;
            try {
                j = json::parse(line);
            } catch (const std::exception& e) {
                std::cerr << "JSON parse error at line " << line_no << ": " << e.what() << "\n";
                continue;
            }

            const std::string kind = j.value("kind", "");
            const std::string name = j.value("name", "");
            if (name.empty() || kind.empty()) {
                std::cerr << "missing 'kind' or 'name' at line " << line_no << "\n";
                continue;
            }

            std::vector<std::string> pos, neg;
            if (j.contains("pos"))
                for (auto& s : j["pos"]) pos.push_back(s.get<std::string>());
            if (j.contains("neg"))
                for (auto& s : j["neg"]) neg.push_back(s.get<std::string>());

            if (pos.empty() && neg.empty()) {
                std::cerr << "no pos/neg terms for '" << name << "' (line " << line_no << ")\n";
                continue;
            }

            // accumulate mean(pos) - mean(neg)
            std::vector<float> acc;
            if (!pos.empty()) {
                for (auto& s : pos) add_inplace(acc, emb.encodeDocument(s).v);
                scale_inplace(acc, 1.0f / float(pos.size()));
            }
            if (!neg.empty()) {
                std::vector<float> negacc;
                for (auto& s : neg) add_inplace(negacc, emb.encodeDocument(s).v);
                scale_inplace(negacc, 1.0f / float(neg.size()));
                sub_inplace(acc, negacc);
            }

            normalize_inplace(acc);

            json out;
            out["kind"] = kind;
            out["name"] = name;
            out["vec"]  = acc;

            fout << out.dump() << "\n";
            ++wrote;
        }

        std::cerr << "wrote " << wrote << " rows to " << out_path << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
