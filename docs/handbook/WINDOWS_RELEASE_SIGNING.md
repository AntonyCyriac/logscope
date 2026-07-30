# Windows Release Signing

| Field | Value |
|-------|-------|
| Document | Windows Release Signing |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

Windows shows **Unknown publisher** (and SmartScreen warnings) for unsigned executables downloaded from GitHub Releases. This document describes how maintainers enable **Authenticode signing** for release builds.

Unsigned releases remain valid; signing is optional until certificate secrets are configured.

---

# 2. What gets signed

| Artifact | Files |
|----------|-------|
| `logscope-windows-amd64.zip` | `logscope.exe` |
| `logscope-desktop-windows-amd64.zip` | `logscope-desktop.exe` and Qt DLLs from `windeployqt` |

Signing runs in [release.yml](../../.github/workflows/release.yml) via [scripts/sign_windows_binaries.ps1](../../scripts/sign_windows_binaries.ps1) **before** archives are created.

If secrets are not set, the script logs a skip message and the workflow continues (unsigned build).

---

# 3. Certificate options

| Type | SmartScreen | Notes |
|------|-------------|-------|
| **OV** (Organization Validation) | Reputation builds over downloads | Lower cost |
| **EV** (Extended Validation) | Immediate SmartScreen trust (typical for public apps) | Requires org validation; hardware token or cloud HSM |

Purchase from a public CA (DigiCert, Sectigo, SSL.com, etc.). Personal name-only certs are not suitable for a product publisher identity.

---

# 4. GitHub Actions secrets

Repository → **Settings** → **Secrets and variables** → **Actions**:

| Secret | Value |
|--------|-------|
| `WINDOWS_CERTIFICATE` | Base64-encoded `.pfx` file |
| `WINDOWS_CERTIFICATE_PASSWORD` | PFX export password |

Encode the PFX (PowerShell):

```powershell
[Convert]::ToBase64String([IO.File]::ReadAllBytes("logscope.pfx")) | Set-Content cert-base64.txt
```

Paste the file contents into `WINDOWS_CERTIFICATE`. Never commit the `.pfx` or password to the repository.

---

# 5. Release workflow behavior

When both secrets are present on a `v*` tag push:

1. Windows CLI job packages `dist/logscope.exe` → signs `dist/`
2. Windows desktop job runs `windeployqt` → copies to `dist/` → signs `dist/`
3. Archives are created from signed binaries

Timestamp server: `http://timestamp.digicert.com` (SHA-256, RFC 3161).

---

# 6. Local signing (maintainer test)

Requires Windows SDK (`signtool.exe`) and a `.pfx` on disk:

```powershell
cmake --build build --config Release --target logscope
New-Item -ItemType Directory -Force dist | Out-Null
Copy-Item build\apps\cli\Release\logscope.exe dist\
$env:CERT_PASSWORD = "your-pfx-password"
.\scripts\sign_windows_binaries.ps1 -Path dist `
  -CertificatePath C:\secrets\logscope.pfx `
  -CertificatePassword $env:CERT_PASSWORD
```

Verify signature:

```powershell
Get-AuthenticodeSignature dist\logscope.exe
```

---

# 7. User experience after signing

| Before | After (EV cert) |
|--------|-----------------|
| Properties → Unknown publisher | Properties shows organization name |
| SmartScreen “Windows protected your PC” | Typically no SmartScreen block for EV |

OV certificates may still show SmartScreen until the certificate builds reputation.

---

# 8. Related documents

| Document | Topic |
|----------|-------|
| [MACOS_RELEASE_NOTARIZATION.md](MACOS_RELEASE_NOTARIZATION.md) | macOS Gatekeeper / notarization |
| [RELEASE.md](../release/RELEASE.md) | Full release checklist |
| [ADR-006](../architecture/decisions/ADR-006-Plugin-Loading.md) | Plugin signing is future work (separate from release binary signing) |

---

# 9. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 30-07-2026 | Initial Windows Authenticode signing guide and CI hook. |
