#!/usr/bin/env python3
"""M2 stub ingest: accepts POST /v1/telemetry/batch and logs accepted samples."""

from __future__ import annotations

import json
import logging
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
LOG = logging.getLogger("stub_ingest")

HOST = "0.0.0.0"
PORT = 8080
PATH = "/v1/telemetry/batch"


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt: str, *args) -> None:
        LOG.info("%s - " + fmt, self.address_string(), *args)

    def do_POST(self) -> None:
        if self.path.split("?", 1)[0] != PATH:
            self.send_error(404, "not found")
            return

        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length)
        try:
            body = json.loads(raw.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError) as exc:
            LOG.warning("bad json: %s", exc)
            self._json_response(400, {"status": "error", "message": "invalid json"})
            return

        samples = body.get("samples")
        if not isinstance(samples, list):
            self._json_response(400, {"status": "error", "message": "samples must be an array"})
            return

        cell_id = body.get("cell_id", "?")
        LOG.info(
            "accepted cell_id=%s schema_version=%s samples=%d body=%s",
            cell_id,
            body.get("schema_version"),
            len(samples),
            json.dumps(body, separators=(",", ":")),
        )
        self._json_response(202, {"status": "ok", "accepted": len(samples)})

    def _json_response(self, code: int, payload: dict) -> None:
        data = json.dumps(payload).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)


def main() -> None:
    server = ThreadingHTTPServer((HOST, PORT), Handler)
    LOG.info("stub ingest listening on http://%s:%d%s", HOST, PORT, PATH)
    server.serve_forever()


if __name__ == "__main__":
    main()
