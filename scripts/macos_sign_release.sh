#!/usr/bin/env bash
# Codesign release binaries or .app bundles with hardened runtime (notarization-ready).
# Skips when MACOS_CERTIFICATE or MACOS_SIGNING_IDENTITY is unset.

set -euo pipefail

usage() {
  echo "Usage: $0 --binary PATH | --app PATH [--entitlements PLIST]" >&2
  exit 1
}

if [[ -z "${MACOS_CERTIFICATE:-}" ]]; then
  echo "MACOS_CERTIFICATE not set; skipping codesign."
  exit 0
fi

IDENTITY="${MACOS_SIGNING_IDENTITY:-}"
if [[ -z "$IDENTITY" ]]; then
  echo "MACOS_SIGNING_IDENTITY not set; skipping codesign."
  exit 0
fi

MODE=""
TARGET=""
ENTITLEMENTS=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --binary)
      MODE="binary"
      TARGET="$2"
      shift 2
      ;;
    --app)
      MODE="app"
      TARGET="$2"
      shift 2
      ;;
    --entitlements)
      ENTITLEMENTS="$2"
      shift 2
      ;;
    *)
      usage
      ;;
  esac
done

if [[ -z "$TARGET" ]]; then
  usage
fi

if [[ ! -e "$TARGET" ]]; then
  echo "Signing target not found: $TARGET" >&2
  exit 1
fi

sign_args=(--force --sign "$IDENTITY" --options runtime --timestamp)
if [[ -n "$ENTITLEMENTS" ]]; then
  sign_args+=(--entitlements "$ENTITLEMENTS")
fi

sign_file() {
  codesign "${sign_args[@]}" "$1"
}

if [[ "$MODE" == "app" ]]; then
  if [[ -d "$TARGET/Contents/Frameworks" ]]; then
    while IFS= read -r -d '' framework; do
      sign_file "$framework"
    done < <(find "$TARGET/Contents/Frameworks" -type d -name '*.framework' -print0)
    while IFS= read -r -d '' dylib; do
      sign_file "$dylib"
    done < <(find "$TARGET/Contents/Frameworks" -type f -name '*.dylib' -print0)
  fi

  if [[ -d "$TARGET/Contents/PlugIns" ]]; then
    while IFS= read -r -d '' plugin; do
      sign_file "$plugin"
    done < <(find "$TARGET/Contents/PlugIns" -type f -print0)
  fi

  if [[ -f "$TARGET/Contents/MacOS/logscope-desktop" ]]; then
    sign_file "$TARGET/Contents/MacOS/logscope-desktop"
  fi

  codesign "${sign_args[@]}" "$TARGET"
else
  sign_file "$TARGET"
fi

codesign --verify --verbose=2 "$TARGET"
echo "Signed $TARGET"
