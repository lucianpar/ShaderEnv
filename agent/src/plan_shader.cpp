#include "AgentEmbedding.hpp"
#include "../third_party/nlohmann/json.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

using json = nlohmann::json;

static float l2(const std::vector<float>& v){ double s=0; for(float x:v)s+=double(x)*x; return float(std::sqrt(s)); }
static void normalize(std::vector<float>& v){ float n=l2(v); if(n>0) for(auto&x:v)x/=n; }
static float dot(const std::vector<float>& a, const std::vector<float>& b){
  float s=0; size_t n = std::min(a.size(), b.size());
  for(size_t i=0;i<n;++i) s += a[i]*b[i]; return s;
}

struct Option {
  std::string tag;       // e.g. "structure:quasicrystal"
  std::string category;  // "structure"
  std::string name;      // "quasicrystal"
  std::vector<float> pos;// mean positive vector (unit)
  std::vector<float> neg;// mean negative vector (unit; may be empty)
};

static bool parse_vec(const json& j, const char* key, std::vector<float>& out){
  if(!j.contains(key)) return false;
  const auto& a = j[key];
  if(!a.is_array()) return false;
  out.resize(a.size());
  for(size_t i=0;i<a.size();++i) out[i] = a[i].get<float>();
  return true;
}

// Accept several field names: "mean_pos"/"mean_neg" or "pos"/"neg"
static void parse_pos_neg(const json& j, std::vector<float>& pos, std::vector<float>& neg){
  if(!(parse_vec(j,"mean_pos",pos) || parse_vec(j,"pos",pos))) pos.clear();
  if(!(parse_vec(j,"mean_neg",neg) || parse_vec(j,"neg",neg))) neg.clear();
  if(!pos.empty()) normalize(pos);
  if(!neg.empty()) normalize(neg);
}

static std::vector<Option> load_vocab_jsonl(const std::string& path){
  std::ifstream in(path);
  if(!in) throw std::runtime_error("failed to open vocab jsonl: " + path);
  std::vector<Option> out;
  std::string line;
  while(std::getline(in, line)){
    if(line.empty()) continue;
    json j = json::parse(line);
    if(!j.contains("tag")) continue;
    Option o;
    o.tag = j["tag"].get<std::string>();
    auto p = o.tag.find(':');
    if(p != std::string::npos){
      o.category = o.tag.substr(0, p);
      o.name     = o.tag.substr(p+1);
    } else {
      o.category = "misc";
      o.name     = o.tag;
    }
    parse_pos_neg(j, o.pos, o.neg);
    if(!o.pos.empty() || !o.neg.empty()) out.push_back(std::move(o));
  }
  return out;
}

int main(int argc, char** argv){
  if(argc < 4){
    std::cerr << "usage: " << argv[0] << " <model.gguf> <vocab.jsonl> \"<prompt>\" [topk]\n";
    return 1;
  }
  const std::string model_path = argv[1];
  const std::string vocab_path = argv[2];
  const std::string prompt     = argv[3];
  int topk = (argc >= 5) ? std::max(1, std::atoi(argv[4])) : 3;

  try{
    // 1) load options
    auto options = load_vocab_jsonl(vocab_path);
    if(options.empty()) throw std::runtime_error("no options loaded from vocab");

    // 2) embed prompt (unit-normalized by AgentEmbedding)
    AgentEmbedding emb(model_path);
    auto q = emb.encode(prompt).v;

    // 3) score
    struct Scored { std::string name; float score; };
    std::unordered_map<std::string, std::vector<Scored>> buckets;
    for(const auto& o : options){
      float sp = o.pos.empty() ? 0.0f : dot(q, o.pos);
      float sn = o.neg.empty() ? 0.0f : dot(q, o.neg);
      float sc = sp - sn; // simple margin
      buckets[o.category].push_back({o.name, sc});
    }

    // 4) sort & take top-k per category
    json out;
    out["prompt"] = prompt;
    json choices = json::object();
    for(auto& kv : buckets){
      auto& vec = kv.second;
      std::sort(vec.begin(), vec.end(), [](const Scored& a, const Scored& b){ return a.score > b.score; });
      json arr = json::array();
      for(int i=0;i<std::min<int>(topk, (int)vec.size()); ++i){
        arr.push_back({{"name", vec[i].name}, {"score", vec[i].score}});
      }
      choices[kv.first] = arr;
    }
    out["choices"] = choices;

    // 5) best guess (top-1 per category)
    json best = json::object();
    for(auto& kv : buckets){
      if(!kv.second.empty()){
        best[kv.first] = kv.second.front().name;
      }
    }
    out["best"] = best;

    std::cout << out.dump(2) << std::endl;
  }catch(const std::exception& e){
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}