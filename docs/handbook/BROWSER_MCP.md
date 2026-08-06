# Browser MCP tab handling (LogScope web demos)

Guide for agents using `cursor-ide-browser` to verify the LogScope SPA (`logscope-web` on `http://127.0.0.1:8080`).

## Root cause (known failure mode)

`browser_tabs` with `action: "new"` **without** `position: "active"` creates `about:blank` tabs that often have **no Browser view**. Navigating or snapshotting those `viewId`s returns:

- `No browser tab available`
- `Browser view not found`

**Do not** create a tab with `browser_tabs new` and then call `browser_navigate` / `browser_snapshot` on that blank `viewId`.

## Correct workflow

### 1. List tabs first

```
browser_tabs { action: "list" }
```

Prefer a tab already on `http://127.0.0.1:8080`.

### 2. Attach to a real tab

**Option A — reuse existing tab (preferred)**

```
browser_lock { action: "lock", viewId: "<id-from-list>" }
browser_navigate { url: "http://127.0.0.1:8080/", viewId: "<id>" }
browser_snapshot { viewId: "<id>" }
```

**Option B — no suitable tab**

```
browser_navigate { url: "http://127.0.0.1:8080/" }
```

Omit `newTab`. Reuses or creates a tab with a proper view. Capture `viewId` from response metadata.

**Option C — user must see the browser**

```
browser_navigate { url: "http://127.0.0.1:8080/", position: "active" }
```

### 3. Always pass `viewId`

After the first successful navigation/snapshot, pass the same `viewId` on every browser tool call.

### 4. Unlock when done

```
browser_lock { action: "unlock", viewId: "<id>" }
```

## Anti-patterns

| Bad | Why |
|-----|-----|
| `browser_tabs { action: "new" }` then navigate blank tab | Orphan tab, no Browser view |
| `browser_navigate { newTab: true }` repeatedly | Tab sprawl, stale viewIds |
| `browser_snapshot` without `viewId` after multiple tabs | Targets wrong / blank tab |
| Retry navigate >1× on same error without `browser_tabs list` | Gather new evidence first |

## Web demo server prep

```powershell
cd build
cmake --build . --config Release --target logscope-web
$env:LOGSCOPE_WEB_UI_DIR = "..\apps\web\ui\dist"
Start-Process -FilePath ".\apps\web\logscope-web.exe" `
  -ArgumentList "--config","..\samples\demo-story-gate.properties" `
  -WorkingDirectory ".." -WindowStyle Hidden
```

Use `samples/demo-story-gate.properties` (`allow_server_paths=true`, `samples/` roots) for Story Gate API + UI demos.

## Story Gate browser checks

1. `browser_tabs list` → pick `127.0.0.1:8080` tab or `browser_navigate` once
2. Create investigation → **Add log** (file picker, e.g. `samples/sample.log`) → **Add pstack** (`samples/pstack.txt`)
3. `browser_lock` → open investigation
4. Verify **Understand Why It Crashed** panel (UX.1)
5. Click fault thread (`.crash-thread--fault`; use `browser_cdp` `Runtime.evaluate` if snapshot ref is ambiguous)
6. Confirm `.crash-pstack-thread--highlight` and status `Jumped to pstack thread`

## Cleanup

Close orphan `about:blank` tabs via `browser_tabs { action: "close", index: N }`.

## Fallback

If browser MCP fails after one correct workflow attempt, use API/integration tests and report the blocker — do not spawn more blank tabs.
