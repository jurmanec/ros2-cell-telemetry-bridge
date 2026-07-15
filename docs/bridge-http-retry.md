# Bridge HTTP failure / retry (M2)

**Status:** Active  
**Related:** [ADR 0002](decisions/0002-bridge-transport.md), [telemetry-batch-http.md](contracts/telemetry-batch-http.md)

Minimal reliability for the northbound HTTPS/HTTP POST bridge. Not a full delivery platform.

## Policy

1. **Timeout** — each attempt fails if the HTTP call does not complete within ~2 seconds.
2. **Retryable** — TCP/DNS/connect failures; HTTP **5xx**; empty/unexpected transport errors.
3. **Not retryable** — HTTP **4xx** (bad request / contract violation). Log and drop the batch.
4. **Attempts** — at most **3** tries per flush (1 initial + 2 retries).
5. **Backoff** — roughly 200 ms after first failure, 500 ms after second (wall-clock sleep in the bridge worker).
6. **Ordering** — best-effort; a dropped batch is not persisted for later replay in M2.
7. **Logging** — structured-enough log lines: attempt number, HTTP status or error string, sample count, cell_id.

## Out of scope (M2)

- Disk spool / store-and-forward  
- Exactly-once / idempotent ingest keys (may add in M3)  
- Circuit breakers, jittered exponential backoff catalogs  
- TLS certificate pinning  
