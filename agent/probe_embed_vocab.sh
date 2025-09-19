set -Eeuo pipefail

# Probe wrapper for agent/build/embed_vocab on macOS.
# You can override with env vars:
#   EMBED_BIN=/abs/path/to/embed_vocab
#   MODEL=/abs/path/to/model.gguf
#   EXTRA_LIB_DIRS="/custom/dir1:/custom/dir2"
#   FIX_INSTALL_NAME=1   # optionally rewrite loader path to @loader_path/libllama.dylib
#
# Example:
#   FIX_INSTALL_NAME=1 ./probe_embed_vocab.sh

ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
EMBED_BIN="${EMBED_BIN:-"$ROOT/agent/build/embed_vocab"}"
MODEL="${MODEL:-"$ROOT/models/nomic-embed-text-v1.5.Q5_K_S.gguf"}"

# Candidate locations for libllama.dylib
declare -a CANDIDATES=(
  "$ROOT/agent/build"
  "$ROOT/third_party/llama.cpp/build/bin"
  "$ROOT/agent/third_party/llama.cpp/build/bin"
  "$ROOT/../llama.cpp/build/bin"
  "$ROOT/llama.cpp/build/bin"
)

# Allow caller to add more
if [[ -n "${EXTRA_LIB_DIRS:-}" ]]; then
  IFS=':' read -r -a EXTRA <<<"$EXTRA_LIB_DIRS"
  CANDIDATES+=("${EXTRA[@]}")
fi

# Auto-discover one if needed
FIND_LIB="$(find "$ROOT" -type f -name 'libllama.dylib' -path '*/build/bin/*' 2>/dev/null | head -n1 || true)"
if [[ -n "$FIND_LIB" ]]; then
  CANDIDATES+=("$(dirname "$FIND_LIB")")
fi

# Build a de-duplicated DYLD_LIBRARY_PATH
declare -A SEEN=()
LIBPATHS=()
for d in "${CANDIDATES[@]}"; do
  if [[ -d "$d" && -f "$d/libllama.dylib" && -z "${SEEN[$d]:-}" ]]; then
    SEEN[$d]=1
    LIBPATHS+=("$d")
  fi
done

if [[ ${#LIBPATHS[@]} -gt 0 ]]; then
  export DYLD_LIBRARY_PATH="$(IFS=:; echo "${LIBPATHS[*]}"):${DYLD_LIBRARY_PATH:-}"
else
  echo "WARN: libllama.dylib not found near repo. Proceeding without DYLD_LIBRARY_PATH hints." >&2
fi

echo "ROOT:                $ROOT"
echo "EMBED_BIN:           $EMBED_BIN"
echo "MODEL:               $MODEL"
echo "DYLD_LIBRARY_PATH:   ${DYLD_LIBRARY_PATH:-<empty>}"
echo

# Optional: patch binary to prefer local lib next to itself
if [[ "${FIX_INSTALL_NAME:-0}" != "0" && -f "$(dirname "$EMBED_BIN")/libllama.dylib" ]]; then
  echo "Applying install_name_tool fix -> @loader_path/libllama.dylib"
  install_name_tool -change @rpath/libllama.dylib @loader_path/libllama.dylib "$EMBED_BIN" || true
  echo
fi

# Sanity checks
[[ -x "$EMBED_BIN" ]] || { echo "ERROR: not executable: $EMBED_BIN"; exit 1; }
[[ -f "$MODEL" ]] || { echo "ERROR: model not found: $MODEL"; exit 1; }

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT
echo "Temp out: $TMPDIR"

# Sample inputs: a mix of formats you've tried, to see what the tool accepts
cat > "$TMPDIR/s1.jsonl" <<'EOF'
{"name":"hello","prompt_variants":["hello world"]}
EOF

cat > "$TMPDIR/s2.jsonl" <<'EOF'
{"text":"hello world"}
EOF

cat > "$TMPDIR/s3.jsonl" <<'EOF'
{"name":"hello","text":"hello world"}
EOF

cat > "$TMPDIR/s4.jsonl" <<'EOF'
{"name":"hello","prompt":"hello world"}
EOF

cat > "$TMPDIR/s5.jsonl" <<'EOF'
{"name":"hello","prompt_variants":["hello","hello world","hi there"]}
EOF

cat > "$TMPDIR/s6.jsonl" <<'EOF'
{"name":"hello","tags":["greeting"],"prompt_variants":["hello world"]}
EOF

cat > "$TMPDIR/s7.jsonl" <<'EOF'
{"name":"blank","prompt_variants":["   "]}
EOF

status=0
for f in "$TMPDIR"/s*.jsonl; do
  out="$TMPDIR/$(basename "${f%.jsonl}").out.jsonl"
  echo "== Trying $f"
  set +e
  "$EMBED_BIN" "$MODEL" "$f" "$out"
  rc=$?
  set -e
  if [[ $rc -ne 0 ]]; then
    echo "  -> embed_vocab exited with code $rc"
    status=$rc
  fi
  if [[ -s "$out" ]]; then
    n=$(wc -l < "$out")
    echo "  -> produced $n line(s). First line:"
    head -n1 "$out"
  else
    echo "  -> No output produced."
  fi
done

echo
echo "== otool -L (embed_vocab)"
otool -L "$EMBED_BIN" || true
echo
echo "== Resolved libllama candidates"
printf '  - %s\n' "${LIBPATHS[@]:-<none>}"

exit $status