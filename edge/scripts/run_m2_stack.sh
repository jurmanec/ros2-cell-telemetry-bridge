#!/usr/bin/env bash
# Build edge image + stub, then run publisher + bridge + ingest stub.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

"$ROOT/scripts/build.sh"
echo "==> Building stub ingest image"
docker compose build ingest

echo "==> Starting M2 stack (Ctrl+C to stop)"
docker compose up
