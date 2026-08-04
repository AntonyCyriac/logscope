# Securing logscope-web

| Field | Value |
|-------|-------|
| Document | Securing logscope-web |
| Category | Handbook |
| Version | 1.1.0 |
| Status | Approved |
| Created | 31-07-2026 |
| Applies to | `v2.2.1+` (M15.4 thin auth); hashed keys at rest `v2.2.2+` |

---

# 1. Purpose

Operators run `logscope-web` on shared hosts, LAN servers, or behind reverse proxies. This guide covers **thin auth** shipped in **v2.2.1**: API key policy, session lifecycle, bind exposure, and TLS termination patterns. Full RBAC is **long-term enterprise scope** (out of scope for current investigation releases).

See [ADR-009 Web Platform REST](../architecture/decisions/ADR-009-Web-Platform-REST.md) and [ADR-009 M15.4 Thin Auth](../architecture/decisions/ADR-009-M15.4-Thin-Auth.md).

---

# 2. Defaults (safe for local dev)

| Setting | Default | Implication |
|---------|---------|-------------|
| `web.bind_host` | `127.0.0.1` | Not reachable from other machines |
| `web.api_key` | empty | No API key required |
| `web.health_requires_api_key` | `false` | Health probe open even when API key set |
| `web.session_ttl_seconds` | `0` | Sessions never evicted by idle TTL |
| `web.max_sessions` | `0` | No in-memory session cap |

**Rule:** Do not bind to `0.0.0.0` or a public interface without `web.api_key` (or TLS + reverse-proxy auth).

---

# 3. API key

When an API key is configured (`web.api_key_hash`, legacy `web.api_key`, or `LOGSCOPE_WEB_API_KEY` / `LOGSCOPE_WEB_API_KEY_HASH`), clients must send:

```http
X-LogScope-Api-Key: <your-key>
```

on all mutating `/api/v1/*` routes (sessions, sources, analyze, workspaces, tail, jobs, etc.).

**Health checks:** By default, `GET /api/v1/health` does **not** require the key (Kubernetes/liveness friendly). Set `web.health_requires_api_key=true` to protect health when the key is configured.

**Storage (v2.2.2+):** Prefer **`web.api_key_hash`** so the secret is not stored in plain text in properties files. Generate a hash:

```bash
logscope-web --hash-api-key 'use-a-long-random-secret'
```

Copy the printed `sha256:...` value into your config:

```properties
web.api_key_hash=sha256:<salt_hex>:<digest_hex>
```

**Legacy:** `web.api_key=<plaintext>` still works but logs a startup warning. **`LOGSCOPE_WEB_API_KEY`** (environment) remains plain text for containers and secret managers — that is expected.

Clients always send the **plain** key in `X-LogScope-Api-Key`; only on-disk config uses the hash.

---

# 4. Bind address and startup warning

If `web.bind_host` is **not** loopback (`127.0.0.1`, `::1`, `localhost`) and `web.api_key` is empty, `logscope-web` logs:

```text
logscope-web: WARNING: bind_host is not loopback and web.api_key is empty — API is exposed without authentication. Set web.api_key or use a reverse proxy with TLS.
```

The process still starts (operators may rely on upstream TLS termination).

---

# 5. Session lifecycle

| Key | Default | Behavior |
|-----|---------|----------|
| `web.session_ttl_seconds` | `0` | When `> 0`, idle sessions are evicted after no activity. Evicted session IDs return **401** `SESSION_EXPIRED` on mutating routes. |
| `web.max_sessions` | `0` | When `> 0`, caps in-memory sessions; evicts oldest **idle** sessions before creating new ones. Returns **503** only when no idle session can be freed (e.g. all have running analyze jobs). |

Activity is updated on each request with a valid `X-LogScope-Session` header. Async analyze jobs prevent eviction of their session until the job finishes or fails.

**Upload temps:** Staged multipart uploads are deleted when superseded or when the session is evicted or shut down — configure `web.upload_temp_dir` if you need a dedicated disk for large uploads.

---

# 6. TLS and reverse proxy

## Embedded TLS (single process)

Requires OpenSSL-enabled build:

```properties
web.tls_cert=/path/to/cert.pem
web.tls_key=/path/to/key.pem
```

Or `--tls-cert` / `--tls-key` / `LOGSCOPE_WEB_TLS_CERT` / `LOGSCOPE_WEB_TLS_KEY`.

## Reverse proxy (recommended for production)

Typical pattern:

1. Bind `logscope-web` to loopback or an internal interface only.
2. Terminate TLS at nginx, Caddy, or cloud load balancer.
3. Set `web.api_key` and pass `X-LogScope-Api-Key` from the proxy (or use proxy auth).
4. Keep health open at the app (`web.health_requires_api_key=false`) or probe through the proxy with the key.

Ensure `web.cors_origins` includes your public origin if the browser SPA is served from a different host.

---

# 7. Health checks

`GET /api/v1/health` returns version, uptime, and session count. Use for liveness when health is open. When `web.health_requires_api_key=true`, include `X-LogScope-Api-Key` in probes.

Idle session eviction runs on health when `web.session_ttl_seconds > 0` (amortized cleanup).

---

# 8. Related documents

| Document | Purpose |
|----------|---------|
| [Configuration Guide §11](CONFIGURATION_GUIDE.md#11-web-platform-logscope-web) | All `web.*` keys |
| [M15.4 Thin Auth Scenarios](../planning/M15.4-THIN-AUTH-SCENARIOS.md) | Acceptance tests |
| [ADR-009](../architecture/decisions/ADR-009-Web-Platform-REST.md) | REST security model |

---

# 9. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 31-07-2026 | Initial M15.4 / v2.2.1 thin auth operator guide |
| 1.1.0 | 04-08-2026 | API key hashing planned for v2.2.2; RBAC long-term enterprise scope. |
| 1.2.0 | 04-08-2026 | v2.2.2: `web.api_key_hash`, `--hash-api-key`, migration from plaintext `web.api_key`. |
