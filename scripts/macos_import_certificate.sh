#!/usr/bin/env bash
# Import Developer ID .p12 into a temporary keychain for release signing.
# Skips when MACOS_CERTIFICATE is unset (unsigned release build).

set -euo pipefail

if [[ -z "${MACOS_CERTIFICATE:-}" ]]; then
  echo "MACOS_CERTIFICATE not set; skipping Apple signing keychain setup."
  exit 0
fi

if [[ -z "${MACOS_CERTIFICATE_PASSWORD:-}" ]]; then
  echo "MACOS_CERTIFICATE_PASSWORD is required when MACOS_CERTIFICATE is set." >&2
  exit 1
fi

KEYCHAIN="${RUNNER_TEMP:-/tmp}/logscope-build.keychain-db"
CERT_PATH="${RUNNER_TEMP:-/tmp}/logscope_certificate.p12"
KEYCHAIN_PASSWORD="${MACOS_KEYCHAIN_PASSWORD:-actions}"

echo -n "$MACOS_CERTIFICATE" | base64 --decode > "$CERT_PATH"

security create-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN"
security default-keychain -s "$KEYCHAIN"
security unlock-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN"
security import "$CERT_PATH" -k "$KEYCHAIN" -P "$MACOS_CERTIFICATE_PASSWORD" \
  -T /usr/bin/codesign -T /usr/bin/security
security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$KEYCHAIN_PASSWORD" "$KEYCHAIN"
security list-keychain -d user -s "$KEYCHAIN"

echo "Apple signing certificate imported to $KEYCHAIN"
