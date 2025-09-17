#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "../third_party/nlohmann/json.hpp"
#include "AgentEmbedding.hpp"

using json = nlohmann::json;


struct VocabItem {
  std::string kind;
  std::string name;
  std::vector<std::string> pos;
  std::vector<std::string> neg;
};

static float l2norm(const std::vector<float>& v) {
  double s = 0.0;
  for (float x : v) s += double(x) * double(x);
  return float(std::sqrt(s));
}
static void normalize(std::vector<float>& v) {
  float n = l2norm(v);
  if (n > 0) for (auto& x : v) x /= n;
}
static std::vector<float> avg_vecs(const std::vector<std::vector<float>>& vecs) {
  if (vecs.empty()) return {};
  std::vector<float> sum(vecs[0].size(), 0.0f);
  for (const auto& v : vecs) {
    if (v.size() != sum.size()) throw std::runtime_error("embedding dim mismatch");
    for (size_t i = 0; i < v.size(); ++i) sum[i] += v[i];
  }
  const float inv = 1.0f / float(vecs.size());
  for (auto& x : sum) x *= inv;
  return sum;
}

static std::string strip_json_comments(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  bool in_str = false;
  bool esc = false;
  for (size_t i = 0; i < s.size(); ++i) {
    char c = s[i];

    if (!in_str && i + 1 < s.size()) {
      if (s[i] == '/' && s[i + 1] == '/') {
        // line comment
        i += 2;
        while (i < s.size() && s[i] != '\n' && s[i] != '\r') ++i;
        out.push_back('\n');
        continue;
      }
      if (s[i] == '/' && s[i + 1] == '*') {
        // block comment
        i += 2;
        while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) ++i;
        if (i + 1 < s.size()) i += 1; // skip '/'
        continue;
      }
    }

    out.push_back(c);
    if (!esc && c == '"') in_str = !in_str;
    esc = (!esc && c == '\\');
    if (c != '\\') esc = false;
  }
  return out;
}

static void push_item(std::vector<VocabItem>& out,
                      const std::string& kind,
                      const std::string& name,
                      const json& val) {
  VocabItem it;
  it.kind = kind;
  it.name = name;

  if (val.is_object()) {
    if (val.contains("pos")) it.pos = val.at("pos").get<std::vector<std::string>>();
    if (val.contains("neg")) it.neg = val.at("neg").get<std::vector<std::string>>();
    if (it.pos.empty() && val.contains("phrases")) {
      it.pos = val.at("phrases").get<std::vector<std::string>>();
    }
  } else if (val.is_array()) {
    // interpret as list of positive phrases
    it.pos = val.get<std::vector<std::string>>();
  } else if (val.is_string()) {
    it.pos = { val.get<std::string>() };
  }

  if (!it.kind.empty() && !it.name.empty() && !it.pos.empty()) {
    out.push_back(std::move(it));
  }
}

static std::vector<VocabItem> load_vocab_items(const std::string& path) {
  // read whole file
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("failed to open " + path);
  std::string all;
  in.seekg(0, std::ios::end);
  all.resize(size_t(in.tellg()));
  in.seekg(0, std::ios::beg);
  in.read(&all[0], std::streamsize(all.size()));
  in.close();

  // remove comments then try to parse
  const std::string clean = strip_json_comments(all);
  std::vector<VocabItem> out;

  try {
    json root = json::parse(clean);

    // CASE A: { "kind": [ ... ] }  (array may contain objects/arrays/strings)
    if (root.is_object()) {
      for (auto it = root.begin(); it != root.end(); ++it) {
        const std::string kind = it.key();
        const json& bucket = it.value();

        // A1: array under a kind
        if (bucket.is_array()) {
          for (const auto& entry : bucket) {
            if (entry.is_object()) {
              // object may have name or be { name: {...} } already handled below
              if (entry.contains("name")) {
                push_item(out, kind, entry.at("name").get<std::string>(), entry);
              } else {
                // try to synthesize a name from the first phrase
                if (entry.contains("pos") && entry.at("pos").is_array() && !entry.at("pos").empty()) {
                  std::string synth = entry.at("pos")[0].get<std::string>();
                  if (synth.size() > 40) synth.resize(40);
                  push_item(out, kind, synth, entry);
                }
              }
            } else if (entry.is_array()) {
              // list of phrases without name -> synthesize a short name
              std::string synth = entry.empty() ? std::string() : entry[0].get<std::string>();
              if (synth.empty()) continue;
              if (synth.size() > 40) synth.resize(40);
              push_item(out, kind, synth, entry);
            } else if (entry.is_string()) {
              std::string s = entry.get<std::string>();
              std::string name = s.size() > 40 ? s.substr(0, 40) : s;
              push_item(out, kind, name, entry);
            }
          }
        }

        // A2: object under a kind -> { "name": {...} } or { "name": ["p1","p2"] }
        else if (bucket.is_object()) {
          for (auto jt = bucket.begin(); jt != bucket.end(); ++jt) {
            const std::string name = jt.key();
            push_item(out, kind, name, jt.value());
          }
        }
      }
      return out;
    }

    // CASE B: flat array of items
    if (root.is_array()) {
      for (const auto& entry : root) {
        if (entry.is_object()) {
          std::string kind, name;
          if (entry.contains("kind")) kind = entry.at("kind").get<std::string>();
          if (entry.contains("name")) name = entry.at("name").get<std::string>();
          if (!kind.empty() && !name.empty()) {
            push_item(out, kind, name, entry);
          }
        }
      }
      return out;
    }
  } catch (...) {
    // fall through to JSONL attempt
  }

  // JSONL fallback: one JSON object per line (expects kind/name/pos[/neg])
  size_t i = 0, lineno = 1;
  while (i < clean.size()) {
    size_t j = clean.find_first_of("\r\n", i);
    const std::string line = (j == std::string::npos) ? clean.substr(i) : clean.substr(i, j - i);
    if (!line.empty() && line.find_first_not_of(" \t") != std::string::npos) {
      try {
        json jline = json::parse(line);
        if (jline.contains("kind") && jline.contains("name")) {
          push_item(out,
                    jline["kind"].get<std::string>(),
                    jline["name"].get<std::string>(),
                    jline);
        } else {
          std::cerr << "missing 'kind' or 'name' at line " << lineno << "\n";
        }
      } catch (const std::exception& e) {
        std::cerr << "warning: failed to parse line " << lineno << ": " << e.what() << "\n";
      }
    }
    if (j == std::string::npos) break;
    if (j + 1 < clean.size() && clean[j] == '\r' && clean[j + 1] == '\n') i = j + 2;
    else i = j + 1;
    ++lineno;
  }
  return out;
}

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "usage: " << argv[0]
              << " <model.gguf> <vocab.(json|jsonl)> <out.jsonl>\n";
    return 1;
  }
  const std::string model_path = argv[1];
  const std::string vocab_path = argv[2];
  const std::string out_path   = argv[3];

  try {
    AgentEmbedding emb(model_path);

    auto items = load_vocab_items(vocab_path);
    if (items.empty()) {
      std::cerr << "error: no options loaded from vocab\n";
      return 1;
    }

    std::ofstream out(out_path);
    if (!out) throw std::runtime_error("failed to open " + out_path + " for write");

    size_t wrote = 0;
    for (const auto& it : items) {
      std::vector<std::vector<float>> pos_vecs, neg_vecs;
      for (const auto& s : it.pos) pos_vecs.push_back(emb.encodeDocument(s).v);
      for (const auto& s : it.neg) neg_vecs.push_back(emb.encodeDocument(s).v);
      if (pos_vecs.empty()) {
        std::cerr << "warning: skipping '" << it.kind << "/" << it.name << "' (no pos phrases)\n";
        continue;
      }
      auto pos_mean = avg_vecs(pos_vecs);
      std::vector<float> combined = pos_mean;
      if (!neg_vecs.empty()) {
        auto neg_mean = avg_vecs(neg_vecs);
        for (size_t i = 0; i < combined.size(); ++i) combined[i] -= neg_mean[i];
      }
      normalize(combined);

      json line = { {"kind", it.kind}, {"name", it.name}, {"embedding", combined} };
      out << line.dump() << "\n";
      ++wrote;
    }
    std::cerr << "wrote " << wrote << " rows to " << out_path << "\n";
    return wrote ? 0 : 2;

  } catch (const std::exception& e) {
    std::cerr << "fatal: " << e.what() << "\n";
    return 1;
  }
}