#!/usr/bin/env bash
# Merge per-package compile_commands.json into ws/build/ and symlink ws/compile_commands.json.
set -euo pipefail
WS="$(cd "$(dirname "$0")/../ws" && pwd)"
python3 - "$WS" <<'PY'
import json, pathlib, sys
ws = pathlib.Path(sys.argv[1])
build = ws / "build"
entries = []
for path in sorted(build.glob("*/compile_commands.json")):
    entries.extend(json.loads(path.read_text()))
merged = build / "compile_commands.json"
merged.write_text(json.dumps(entries, indent=2) + "\n")
link = ws / "compile_commands.json"
if link.exists() or link.is_symlink():
    link.unlink()
link.symlink_to(merged)
print(f"merged {len(entries)} entries -> {link} -> {merged}")
PY
