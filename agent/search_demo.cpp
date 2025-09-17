// search_demo.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "third_party/nlohmann/json.hpp"  // agent/third_party/json/json.hpp if you renamed

using json = nlohmann::json;

static float dot(const std::vector<float>& a, const std::vector<float>& b) {
    float s = 0.f;
    for (size_t i = 0; i < a.size(); ++i) s += a[i]*b[i];
    return s;
}
static float norm(const std::vector<float>& a) {
    return std::sqrt(dot(a,a));
}
static float cosine(const std::vector<float>& a, const std::vector<float>& b) {
    float na = norm(a), nb = norm(b);
    if (na == 0.f || nb == 0.f) return 0.f;
    return dot(a,b) / (na*nb);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " <docs.jsonl> <query.jsonl> [k=5]\n";
        return 1;
    }
    const std::string docs_path = argv[1];
    const std::string query_path = argv[2];
    int k = (argc > 3) ? std::stoi(argv[3]) : 5;

    // load query embedding (assume single line)
    std::ifstream qf(query_path);
    if (!qf) { std::cerr << "failed to open " << query_path << "\n"; return 1; }
    std::string qline; std::getline(qf, qline);
    auto qj = json::parse(qline);
    std::vector<float> qvec = qj.at("embedding").get<std::vector<float>>();

    // load docs
    std::ifstream df(docs_path);
    if (!df) { std::cerr << "failed to open " << docs_path << "\n"; return 1; }
    struct Item { std::string text; float score; };
    std::vector<Item> items;
    std::string line;
    while (std::getline(df, line)) {
        if (line.empty()) continue;
        auto j = json::parse(line);
        const auto& dvec = j.at("embedding").get<std::vector<float>>();
        float s = cosine(qvec, dvec);
        items.push_back({ j.value("text", ""), s });
    }

    std::partial_sort(items.begin(),
                      items.begin() + std::min<int>(k, items.size()),
                      items.end(),
                      [](const Item& a, const Item& b){ return a.score > b.score; });

    for (int i = 0; i < std::min<int>(k, items.size()); ++i) {
        std::cout << "[" << i << "] score=" << items[i].score
                  << "  text=\"" << items[i].text << "\"\n";
    }
    return 0;
}
