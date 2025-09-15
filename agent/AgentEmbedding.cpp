// AgentEmbedding.cpp — llama.cpp embeddings backend (nomic-bert encoder, prompt-variant fallbacks)

#include "AgentEmbedding.hpp"

#include <llama.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <cmath>
#include <array>

namespace {
inline float l2norm(const std::vector<float>& v) {
  double s = 0.0;
  for (float x : v) s += double(x) * double(x);
  return float(std::sqrt(s));
}

inline bool tokenize_two_pass(const llama_vocab* vocab,
                              const char* text, int32_t len,
                              bool add_special, bool parse_special,
                              std::vector<llama_token>& out) {
  int32_t need = llama_tokenize(vocab, text, len,
                                /*tokens*/ nullptr, /*n_max_tokens*/ 0,
                                add_special, parse_special);
  if (need <= 0) return false;
  out.resize(need);
  int32_t got = llama_tokenize(vocab, text, len,
                               out.data(), (int32_t)out.size(),
                               add_special, parse_special);
  if (got <= 0) return false;
  out.resize(got);
  return true;
}

// Try a few flag combos commonly needed for BERT/WPM
inline bool tokenize_for_bert(const llama_vocab* vocab,
                              const std::string& s,
                              std::vector<llama_token>& out) {
  const char* c = s.c_str();
  const int32_t n = (int32_t)s.size();
  // Preferred for BERT encoders: add special tokens, don't parse-as-special
  if (tokenize_two_pass(vocab, c, n, /*add_special=*/true,  /*parse_special=*/false, out)) return true;
  if (tokenize_two_pass(vocab, c, n, /*add_special=*/false, /*parse_special=*/false, out)) return true;
  if (tokenize_two_pass(vocab, c, n, /*add_special=*/true,  /*parse_special=*/true,  out)) return true;
  return false;
}
} // namespace

struct AgentEmbedding::Impl {
  llama_model*   model     = nullptr;
  llama_context* ctx       = nullptr;
  int            dim       = 0;
  int            n_threads = 0;

  explicit Impl(const std::string& model_path) {
    llama_backend_init();

    llama_model_params mp = llama_model_default_params();
    model = llama_model_load_from_file(model_path.c_str(), mp);
    if (!model) throw std::runtime_error("Failed to load GGUF model: " + model_path);

    llama_context_params cp = llama_context_default_params();
    cp.embeddings   = true;                        // enable embeddings
    cp.pooling_type = LLAMA_POOLING_TYPE_MEAN;     // pooled embedding out of llama_get_embeddings()
    cp.n_threads    = std::max(1u, std::thread::hardware_concurrency());
    cp.n_ctx        = 2048;                        // safe upper bound for nomic-bert

    ctx = llama_init_from_model(model, cp);
    if (!ctx) throw std::runtime_error("Failed to create llama context");

    dim       = llama_model_n_embd(model);
    n_threads = cp.n_threads;
  }

  ~Impl() {
    if (ctx)   llama_free(ctx);
    if (model) llama_model_free(model);
    llama_backend_free();
  }

  // Try several prompt variants until one tokenizes
  EmbedVec embed_with_prefix(const std::string& canonical_prefix,
                             const std::string& alt_prefix,
                             const std::string& text) const {
    const llama_vocab* vocab = llama_model_get_vocab(model);
    if (!vocab) throw std::runtime_error("llama_model_get_vocab returned nullptr");

    // Variants to try (in order)
    std::array<std::string, 6> variants = {
      canonical_prefix + text,         // e.g. "search_query: ...", "search_document: ..."
      alt_prefix + text,               // e.g. "query: ...", "passage: ..."
      std::string(" ") + text,         // leading space sometimes helps WPM
      text,                            // plain
      canonical_prefix + " " + text,   // with extra space after colon
      alt_prefix + " " + text
    };

    std::vector<llama_token> tokens;
    int chosen = -1;
    for (int i = 0; i < (int)variants.size(); ++i) {
      tokens.clear();
      if (tokenize_for_bert(vocab, variants[i], tokens) && !tokens.empty()) {
        chosen = i;
        break;
      }
    }

    if (chosen == -1) {
      throw std::runtime_error("Tokenization produced no tokens for any prompt variant");
    }

    // --- Encode with batch API (encoder path) ---
    llama_batch batch = llama_batch_init(/*n_tokens_alloc*/ (int32_t)tokens.size(),
                                         /*embd*/ 0, /*n_seq_max*/ 1);
    for (int i = 0; i < (int)tokens.size(); ++i) {
      batch.token[i]     = tokens[i];
      batch.pos[i]       = i;
      batch.n_seq_id[i]  = 1;
      batch.seq_id[i][0] = 0;   // single sequence
      batch.logits[i]    = 0;
    }
    // toggle last-step logits to ensure outputs are materialized
    batch.logits[(int)tokens.size() - 1] = 1;
    batch.n_tokens = (int32_t)tokens.size();

    if (llama_encode(ctx, batch) != 0) {
      llama_batch_free(batch);
      throw std::runtime_error("llama_encode failed");
    }

    const float* e = llama_get_embeddings(ctx);
    if (!e) {
      llama_batch_free(batch);
      throw std::runtime_error("No embeddings returned (ensure embeddings=true & pooling)");
    }

    EmbedVec out;
    out.v.assign(e, e + dim);

    llama_batch_free(batch);

    // L2-normalize for cosine similarity workflows
    float norm = l2norm(out.v);
    if (norm > 0.0f) for (auto &x : out.v) x /= norm;

    return out;
  }
};

AgentEmbedding::AgentEmbedding(const std::string& gguf_model_path)
: impl_(std::make_unique<Impl>(gguf_model_path)) {}

AgentEmbedding::~AgentEmbedding() = default;

EmbedVec AgentEmbedding::encode(const std::string& text) const {
  // Common prefixes for nomic-embed-text
  return impl_->embed_with_prefix("search_query: ", "query: ", text);
}

EmbedVec AgentEmbedding::encodeDocument(const std::string& text) const {
  return impl_->embed_with_prefix("search_document: ", "passage: ", text);
}
