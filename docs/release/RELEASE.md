# Release Process

| Field | Value |
|-------|-------|
| Document | Release Process |
| Category | Release |
| Version | 1.4.3 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 24-07-2026 |

---

# Purpose

This document describes how maintainers cut LogScope releases, from version bump through GitHub Release publication.

---

# Release Types

| Tag | When |
|-----|------|
| `v0.x.0` | Milestone completion (M3, M4, M5 interim) |
| `v1.0.0` | First stable production release after M5 validation |
| `v1.0.0-rc.N` | Release candidate before v1.0.0 |

---

# Maintainer Checklist

## 1. Prepare release branch

```bash
git checkout master
git pull origin master
git checkout -b chore/vX.Y.Z-release
```

## 2. Version bump

- Update `VERSION` in root [`CMakeLists.txt`](../../CMakeLists.txt)
- Move `[Unreleased]` entries to `[X.Y.Z]` in [`CHANGELOG.md`](../../CHANGELOG.md)
- Update status references in [`README.md`](../../README.md), [`docs/ROADMAP.md`](../ROADMAP.md), [`docs/PRODUCT.md`](../PRODUCT.md)

## 3. Verify locally

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Optional:

```bash
cmake -S . -B build -DLOGSCOPE_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target logscope_benchmarks
```

## 4. Open and merge PR

- Title: `chore(release): prepare vX.Y.Z`
- Wait for CI (build matrix, coverage, benchmarks, fuzz)

## 5. Tag and push

```bash
git checkout master
git pull origin master
git tag -a vX.Y.Z -m "vX.Y.Z — <summary>"
git push origin vX.Y.Z
```

Release tags (`vX.Y.Z`) are the public sync points for related private strategy materials. See [§8 Post-release housekeeping](#8-post-release-housekeeping) and [Git Conventions](../handbook/GIT_CONVENTIONS.md#4-release-tags).

## 6. GitHub Release

For tags matching `v*`, the [release workflow](../../.github/workflows/release.yml) builds per-OS artifacts, runs the bulk-log CLI matrix, and attaches binaries to the GitHub Release.

**Bulk-log matrix size:** CI (`cli-matrix` on Ubuntu) uses **10k-line** fixtures (`BULK_LOG_LINES: 10000`) for fast PR feedback. Release workflows (all OSes) use **100k-line** fixtures (`BULK_LOG_LINES: 100000`), including `--persist-index` scenarios, after **`v1.4.2`** batched SQLite writes.

The workflow creates the GitHub Release with binaries attached. Release notes are loaded automatically from `docs/release/vX.Y.Z-RELEASE-NOTES.md` matching the tag (for example `v2.0.1` → `docs/release/v2.0.1-RELEASE-NOTES.md`).

**Windows Authenticode signing (optional):** When repository secrets `WINDOWS_CERTIFICATE` and `WINDOWS_CERTIFICATE_PASSWORD` are configured, the workflow signs `.exe` and `.dll` in Windows release folders before archiving. Without secrets, Windows binaries ship unsigned (SmartScreen may show **Unknown publisher**). See [Windows Release Signing](../handbook/WINDOWS_RELEASE_SIGNING.md).

**macOS signing and notarization (optional):** When `MACOS_CERTIFICATE`, `MACOS_SIGNING_IDENTITY`, and notarization credentials (API key or app-specific password) are configured, macOS CLI and desktop artifacts are signed with hardened runtime, notarized, and stapled before archiving. Without secrets, macOS binaries ship unsigned (Gatekeeper may block). See [macOS Release Notarization](../handbook/MACOS_RELEASE_NOTARIZATION.md).

Maintainer steps after the workflow completes:

1. Open **Releases** for the new tag and verify the notes body and attached artifacts.

## 7. Source packages (optional)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --build build --target package
```

Produces `logscope-<version>.tar.gz` and `.zip` via CPack.

## 8. Post-release housekeeping

Complete after the public tag and GitHub Release are live:

| Step | Action |
|------|--------|
| Release notes | Ensure the GitHub Release body summarizes the `[X.Y.Z]` section from [`CHANGELOG.md`](../../CHANGELOG.md) |
| Private strategy sync | On the private strategy repository: update long-horizon docs for the shipped milestone, then tag `sync/vX.Y.Z` on that commit (annotated message: `sync/vX.Y.Z — public vX.Y.Z <milestone summary>`) and push the tag |
| Bulk matrix (when ready) | CI at `10000` lines; release runners at `100000` lines |

Example private strategy sync (run in the strategy repository checkout, not in this repo):

```bash
git checkout master
git pull
# commit strategy doc updates for the shipped milestone
git tag -a sync/vX.Y.Z -m "sync/vX.Y.Z — public vX.Y.Z <summary>"
git push origin sync/vX.Y.Z
```

---

# Install from Source

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix /usr/local
logscope analyze samples/sample.log
```

---

# Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-07-2026 | Initial release process documentation. |
| 1.1.0 | 24-07-2026 | Note `sync/vX.Y.Z` private strategy alignment after public release tags. |
| 1.2.0 | 24-07-2026 | Release workflow runs bulk-log CLI matrix before publishing binaries. |
| 1.3.0 | 24-07-2026 | Document 10k bulk matrix (restore 100k in v1.4.2); add post-release housekeeping checklist. |
| 1.4.0 | 24-07-2026 | Header bump; 100k release matrix shipped in v1.4.2. |
| 1.4.2 | 30-07-2026 | Optional Windows Authenticode signing via GitHub secrets. |
| 1.4.3 | 30-07-2026 | Optional macOS signing and notarization via GitHub secrets. |
