# Models

This directory is where you put local `.gguf` model files.  
They are **ignored by Git** to keep the repo lean.

## Example (Nomic Embedding)

Download the model into this folder:

```bash
# from repo root
cd agent/models
# put your own source URL here
curl -L -o nomic-embed-text-v1.5.Q5_K_S.gguf "<MODEL_DOWNLOAD_URL>"
```
