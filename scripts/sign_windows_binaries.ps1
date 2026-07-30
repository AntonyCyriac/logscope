#Requires -Version 5.1
<#
.SYNOPSIS
  Authenticode-sign Windows release binaries (.exe, .dll).

.DESCRIPTION
  Used by .github/workflows/release.yml when GitHub secrets are configured.
  Skips silently when no certificate is available (unsigned release build).

  Environment variables (CI):
    WINDOWS_CERTIFICATE          Base64-encoded .pfx
    WINDOWS_CERTIFICATE_PASSWORD PFX password

  Local example:
    ./scripts/sign_windows_binaries.ps1 -Path dist `
      -CertificatePath C:\secrets\logscope.pfx `
      -CertificatePassword $env:CERT_PASSWORD
#>
param(
    [Parameter(Mandatory)]
    [string]$Path,

    [string]$CertificatePath = "",
    [string]$CertificatePassword = "",
    [string]$TimestampUrl = "http://timestamp.digicert.com"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-SignTool {
    $kitsRoot = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
    if (-not (Test-Path $kitsRoot)) {
        throw "Windows SDK not found at $kitsRoot"
    }

    $signtool = Get-ChildItem -Path $kitsRoot -Filter signtool.exe -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match 'x64\\signtool\.exe$' } |
        Sort-Object FullName -Descending |
        Select-Object -First 1

    if (-not $signtool) {
        throw "signtool.exe not found under $kitsRoot"
    }

    return $signtool.FullName
}

function Resolve-PfxPath {
    param(
        [string]$LocalPath,
        [string]$LocalPassword
    )

    if ($LocalPath) {
        if (-not (Test-Path $LocalPath)) {
            throw "Certificate file not found: $LocalPath"
        }
        if (-not $LocalPassword) {
            throw "CertificatePassword is required when CertificatePath is set."
        }
        return @{
            Path = $LocalPath
            Password = $LocalPassword
        }
    }

    $certB64 = $env:WINDOWS_CERTIFICATE
    if (-not $certB64) {
        return $null
    }

    $password = $env:WINDOWS_CERTIFICATE_PASSWORD
    if (-not $password) {
        throw "WINDOWS_CERTIFICATE_PASSWORD is required when WINDOWS_CERTIFICATE is set."
    }

    $tempRoot = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { $env:TEMP }
    $pfxPath = Join-Path $tempRoot "logscope-sign.pfx"
    [IO.File]::WriteAllBytes($pfxPath, [Convert]::FromBase64String($certB64))

    return @{
        Path = $pfxPath
        Password = $password
    }
}

if (-not (Test-Path $Path)) {
    throw "Signing path not found: $Path"
}

$pfx = Resolve-PfxPath -LocalPath $CertificatePath -LocalPassword $CertificatePassword
if (-not $pfx) {
    Write-Host "No signing certificate configured; skipping Authenticode signing."
    exit 0
}

$signtool = Resolve-SignTool
$files = Get-ChildItem -Path $Path -Include *.exe, *.dll -Recurse -File -ErrorAction SilentlyContinue

if (-not $files) {
    Write-Host "No .exe or .dll files found under $Path; nothing to sign."
    exit 0
}

foreach ($file in $files) {
    Write-Host "Signing $($file.FullName)"
    & $signtool sign `
        /f $pfx.Path `
        /p $pfx.Password `
        /tr $TimestampUrl `
        /td sha256 `
        /fd sha256 `
        $file.FullName

    if ($LASTEXITCODE -ne 0) {
        throw "signtool failed for $($file.FullName) (exit $LASTEXITCODE)"
    }
}

Write-Host "Signed $($files.Count) file(s) under $Path"
