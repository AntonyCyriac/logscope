#!/usr/bin/env bash
# Submit a signed binary or .app to Apple notarization and staple the ticket.
# Skips when notarization credentials or signing certificate are unset.

set -euo pipefail

TARGET="${1:-}"
if [[ -z "$TARGET" ]]; then
  echo "Usage: $0 PATH_TO_APP_OR_BINARY" >&2
  exit 1
fi

if [[ ! -e "$TARGET" ]]; then
  echo "Notarization target not found: $TARGET" >&2
  exit 1
fi

has_api_key=false
if [[ -n "${APPLE_API_KEY_BASE64:-}" && -n "${APPLE_API_KEY_ID:-}" && -n "${APPLE_API_ISSUER_ID:-}" ]]; then
  has_api_key=true
fi

has_apple_id=false
if [[ -n "${APPLE_ID:-}" && -n "${APPLE_APP_SPECIFIC_PASSWORD:-}" && -n "${APPLE_TEAM_ID:-}" ]]; then
  has_apple_id=true
fi

if [[ "$has_api_key" == false && "$has_apple_id" == false ]]; then
  echo "Notarization credentials not configured; skipping notarization."
  exit 0
fi

if [[ -z "${MACOS_CERTIFICATE:-}" ]]; then
  echo "MACOS_CERTIFICATE not set; skipping notarization (unsigned binaries cannot be notarized)."
  exit 0
fi

ZIP="${RUNNER_TEMP:-/tmp}/logscope-notarize-upload.zip"
rm -f "$ZIP"

if [[ "$TARGET" == *.app ]]; then
  ditto -c -k --keepParent "$TARGET" "$ZIP"
else
  ditto -c -k "$TARGET" "$ZIP"
fi

notary_args=(submit "$ZIP" --wait)

if [[ "$has_api_key" == true ]]; then
  KEY_PATH="${RUNNER_TEMP:-/tmp}/AuthKey.p8"
  echo -n "$APPLE_API_KEY_BASE64" | base64 --decode > "$KEY_PATH"
  notary_args+=(--key "$KEY_PATH" --key-id "$APPLE_API_KEY_ID" --issuer "$APPLE_API_ISSUER_ID")
else
  notary_args+=(--apple-id "$APPLE_ID" --password "$APPLE_APP_SPECIFIC_PASSWORD" --team-id "$APPLE_TEAM_ID")
fi

xcrun notarytool "${notary_args[@]}"
xcrun stapler staple "$TARGET"
xcrun stapler validate "$TARGET"

echo "Notarized and stapled $TARGET"
