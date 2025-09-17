#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "../third_party/nlohmann/json.hpp"

using json = nlohmann::json;

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* s = static_cast<std::string*>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

static std::string post_embeddings(
    const std::string& host, const std::vector<std::string>& inputs) {

    json payload{{"model", "nomic-embed-text-v1.5"}, {"input", inputs}};
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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error(std::string("curl error: ") + curl_easy_strerror(rc));
    }
    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (code < 200 || code >= 300) {
        throw std::runtime_error("HTTP " + std::to_string(code) + " body: " + resp);
    }
    return resp;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "usage: " << argv[0]
                  << " <input.txt> <output.jsonl> <doc|query> <host=http://127.0.0.1:8080>\n";
        return 1;
    }
    const std::string in_path  = argv[1];
    const std::string out_path = argv[2];
    const std::string mode     = argv[3];
    const std::string host     = (argc >= 5 ? argv[4] : "http://127.0.0.1:8080");

    const std::string prefix = (mode == "doc") ? "search_document: " : "search_query: ";

    std::ifstream in(in_path);
    if (!in) { std::cerr << "failed to open " << in_path << "\n"; return 1; }
    std::vector<std::pair<size_t, std::string>> items;
    for (std::string line; std::getline(in, line); ) {
        std::string trimmed = line;
        trimmed.erase(trimmed.begin(),
                      std::find_if(trimmed.begin(), trimmed.end(), [](unsigned char c){return !std::isspace(c);} ));
        trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(), [](unsigned char c){return !std::isspace(c);}).base(),
                      trimmed.end());
        if (!trimmed.empty()) items.emplace_back(items.size(), trimmed);
    }

    std::ofstream out(out_path);
    if (!out) { std::cerr << "failed to open " << out_path << "\n"; return 1; }

    const size_t BATCH = 64;
    for (size_t i = 0; i < items.size(); i += BATCH) {
        const size_t j = std::min(i + BATCH, items.size());
        std::vector<std::string> batch;
        batch.reserve(j - i);
        for (size_t k = i; k < j; ++k) batch.push_back(prefix + items[k].second);

        try {
            std::string resp = post_embeddings(host, batch);
            auto jresp = json::parse(resp);
            const auto& data = jresp["data"];
            if (!data.is_array() || data.size() != batch.size())
                throw std::runtime_error("unexpected response shape");

            for (size_t k = 0; k < data.size(); ++k) {
                json row;
                row["id"] = items[i + k].first;
                row["text"] = items[i + k].second;
                row["embedding"] = data[k]["embedding"];
                out << row.dump() << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "batch " << i << " error: " << e.what() << "\n";
            return 2;
        }
    }
    std::cerr << "wrote " << out_path << "\n";
    return 0;
}
