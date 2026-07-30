# macOS Release Notarization

| Field | Value |
|-------|-------|
| Document | macOS Release Notarization |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

macOS **Gatekeeper** blocks or warns on unsigned or un-notarized software downloaded outside the App Store (“cannot be opened because the developer cannot be verified”).

This document describes how maintainers enable **Developer ID signing** and **notarization** for LogScope release builds.

Unsigned releases remain valid; signing and notarization are optional until Apple credentials are configured in GitHub Actions.

---

# 2. What gets signed and notarized

| Artifact | Target |
|----------|--------|
| `logscope-macos-amd64.tar.gz` | `dist/logscope` CLI binary |
| `logscope-desktop-macos-amd64.tar.gz` | `dist/logscope-desktop.app` (after `macdeployqt`) |

Steps run in [release.yml](../../.github/workflows/release.yml) via:

| Script | Role |
|--------|------|
| [macos_import_certificate.sh](../../scripts/macos_import_certificate.sh) | Import `.p12` into a CI keychain |
| [macos_sign_release.sh](../../scripts/macos_sign_release.sh) | Hardened-runtime `codesign` |
| [macos_notarize_release.sh](../../scripts/macos_notarize_release.sh) | `notarytool submit` + `stapler staple` |

If secrets are missing, each script logs a skip message and the workflow continues.

---

# 3. Prerequisites

| Requirement | Notes |
|-------------|-------|
| **Apple Developer Program** | ~$99/year — https://developer.apple.com/programs/ |
| **Developer ID Application certificate** | Create in Xcode or Certificates portal; export as `.p12` |
| **Notarization credentials** | App Store Connect API key (**recommended**) or app-specific password |

Desktop app entitlements: [apps/desktop/macos/entitlements.plist](../../apps/desktop/macos/entitlements.plist) (`disable-library-validation` for Qt and optional dynamic plugins).

---

# 4. GitHub Actions secrets

Repository → **Settings** → **Secrets and variables** → **Actions**:

### Signing (required for notarization)

| Secret | Value |
|--------|-------|
| `MACOS_CERTIFICATE` | Base64-encoded `.p12` (Developer ID Application) |
| `MACOS_CERTIFICATE_PASSWORD` | PFX export password |
| `MACOS_SIGNING_IDENTITY` | Full identity string, e.g. `Developer ID Application: Your Name (TEAMID)` |

Encode the `.p12` (macOS/Linux):

```bash
base64 -i DeveloperID.p12 | pbcopy   # macOS
# or
base64 -w0 DeveloperID.p12 > cert-base64.txt
```

Find the signing identity:

```bash
security find-identity -v -p codesigning
```

### Notarization — option A (recommended): API key

Create in [App Store Connect → Users and Access → Keys](https://appstoreconnect.apple.com/access/api):

| Secret | Value |
|--------|-------|
| `APPLE_API_KEY_BASE64` | Base64 of `AuthKey_XXXXXXXXXX.p8` |
| `APPLE_API_KEY_ID` | Key ID (10 characters) |
| `APPLE_API_ISSUER_ID` | Issuer ID from App Store Connect |

```bash
base64 -i AuthKey_XXXXXXXXXX.p8 | pbcopy
```

### Notarization — option B: app-specific password

| Secret | Value |
|--------|-------|
| `APPLE_ID` | Apple ID email |
| `APPLE_APP_SPECIFIC_PASSWORD` | From https://appleid.apple.com (App-Specific Passwords) |
| `APPLE_TEAM_ID` | 10-character Team ID |

Use **either** option A or option B (API key preferred for CI).

---

# 5. Release workflow behavior

On `v*` tag push, macOS jobs:

1. Import certificate into a temporary keychain
2. Package binaries into `dist/`
3. Desktop: `macdeployqt` with `-sign-for-notarization` when identity is set
4. `codesign` with `--options runtime` (hardened runtime)
5. Zip upload → `notarytool submit --wait`
6. `stapler staple` on the binary or `.app`
7. Create `tar.gz` for GitHub Release

---

# 6. Local signing and notarization (maintainer test)

```bash
export MACOS_CERTIFICATE="$(base64 -i DeveloperID.p12)"
export MACOS_CERTIFICATE_PASSWORD='pfx-password'
export MACOS_SIGNING_IDENTITY='Developer ID Application: Your Name (TEAMID)'
export APPLE_API_KEY_BASE64="$(base64 -i AuthKey_XXX.p8)"
export APPLE_API_KEY_ID='XXXXXXXXXX'
export APPLE_API_ISSUER_ID='xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx'

./scripts/macos_import_certificate.sh

# CLI
cp build/apps/cli/logscope dist/logscope
./scripts/macos_sign_release.sh --binary dist/logscope
./scripts/macos_notarize_release.sh dist/logscope

# Desktop
macdeployqt dist/logscope-desktop.app -always-overwrite -sign-for-notarization
./scripts/macos_sign_release.sh --app dist/logscope-desktop.app \
  --entitlements apps/desktop/macos/entitlements.plist
./scripts/macos_notarize_release.sh dist/logscope-desktop.app
```

Verify:

```bash
codesign --verify --verbose=2 dist/logscope-desktop.app
spctl --assess --verbose dist/logscope-desktop.app
```

---

# 7. User experience after notarization

| Before | After |
|--------|-------|
| Gatekeeper block / quarantine warning | Normal open from Downloads (double-click or `open`) |
| `spctl` rejects | `spctl --assess` accepted |

Users may still need to remove quarantine on old downloads: `xattr -dr com.apple.quarantine logscope-desktop.app`.

---

# 8. Related documents

| Document | Topic |
|----------|-------|
| [WINDOWS_RELEASE_SIGNING.md](WINDOWS_RELEASE_SIGNING.md) | Windows Authenticode |
| [RELEASE.md](../release/RELEASE.md) | Full release checklist |

---

# 9. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 30-07-2026 | Initial macOS signing and notarization guide and CI hooks. |
