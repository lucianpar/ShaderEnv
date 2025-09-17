#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include <curl/curl.h>
#include "../third_party/nlohmann/json.hpp"

using json = nlohmann::json;

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* s = static_cast<std::string*>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

static std::vector<float> embed_query_http(const std::string& host, const std::string& q) {
    json payload{{"model","nomic-embed-text-v1.5"},
                 {"input", {std::string("search_query: ") + q}}};
    std::string body = payload.dump();

    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init failed");

    std::string url = host + "/v1/embeddings";
    std::string resp;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) throw std::runtime_error(curl_easy_strerror(rc));
    long code = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (code < 200 || code >= 300) throw std::runtime_error("HTTP " + std::to_string(code));
    auto j = json::parse(resp);
    std::vector<float> v = j["data"][0]["embedding"].get<std::vector<float>>();
    return v;
}

static float cosine(const std::vector<float>& a, const std::vector<float>& b) {
    double num = 0.0, da = 0.0, db = 0.0;
    const size_t n = std::min(a.size(), b.size());
    for (size_t i = 0; i < n; ++i) {
        num += a[i] * b[i];
        da  += a[i] * a[i];
        db  += b[i] * b[i];
    }
    if (da == 0.0 || db == 0.0) return 0.0f;
    return static_cast<float>(num / (std::sqrt(da) * std::sqrt(db)));
}

struct Doc { size_t id; std::string text; std::vector<float> emb; };

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: " << argv[0]
                  << " <docs.jsonl> <query> <k=5> [host=http://127.0.0.1:8080]\n";
        return 1;
    }
    const std::string docs_path = argv[1];
    const std::string query     = argv[2];
    const int k                 = (argc >= 4 ? std::stoi(argv[3]) : 5);
    const std::string host      = (argc >= 5 ? argv[4] : "http://127.0.0.1:8080");

    std::ifstream in(docs_path);
    if (!in) { std::cerr << "failed to open " << docs_path << "\n"; return 1; }

    std::vector<Doc> docs;
    for (std::string line; std::getline(in, line); ) {
        if (line.empty()) continue;
        auto j = json::parse(line);
        Doc d;
        d.id   = j["id"].get<size_t>();
        d.text = j["text"].get<std::string>();
        d.emb  = j["embedding"].get<std::vector<float>>();
        docs.push_back(std::move(d));
    }

    std::vector<float> q = embed_query_http(host, query);

    std::vector<std::pair<float, size_t>> scored;
    scored.reserve(docs.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        scored.emplace_back(cosine(q, docs[i].emb), i);
    }
    std::partial_sort(scored.begin(), scored.begin() + std::min((size_t)k, scored.size()), scored.end(),
                      [](auto& A, auto& B){ return A.first > B.first; });

    for (int i = 0; i < k && i < (int)scored.size(); ++i) {
        const auto& d = docs[scored[i].second];
        std::cout << scored[i].first << "\t" << d.id << "\t"
                  << (d.text.size() > 120 ? d.text.substr(0,120) + "..." : d.text) << "\n";
    }
    return 0;
}
