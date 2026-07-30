#!/usr/bin/env python3
"""Verify license files for FetchContent dependencies in build/_deps."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


def find_dependency_dir(deps_root: Path, fetch_prefix: str) -> Path | None:
    if not deps_root.is_dir():
        return None

    for child in sorted(deps_root.iterdir()):
        if child.is_dir() and child.name.startswith(fetch_prefix) and child.name.endswith("-src"):
            return child

    return None


def license_file_ok(dep_dir: Path, relative_paths: list[str], snippet: str | None) -> tuple[bool, str]:
    for relative_path in relative_paths:
        license_path = dep_dir / relative_path

        if not license_path.is_file():
            continue

        if snippet is None:
            return True, str(license_path)

        try:
            content = license_path.read_text(encoding="utf-8", errors="ignore")
        except OSError as error:
            return False, f"unable to read {license_path}: {error}"

        if snippet.lower() in content.lower():
            return True, str(license_path)

        return False, f"snippet {snippet!r} not found in {license_path}"

    return False, f"no license file among {relative_paths!r}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--manifest",
        type=Path,
        default=Path("third_party/manifest.json"),
        help="Dependency manifest path",
    )
    parser.add_argument(
        "--deps-dir",
        type=Path,
        default=Path("build/_deps"),
        help="CMake FetchContent output directory",
    )
    parser.add_argument(
        "--require-optional",
        action="store_true",
        help="Fail when optional dependencies are missing",
    )
    args = parser.parse_args()

    if not args.manifest.is_file():
        raise SystemExit(f"manifest not found: {args.manifest}")

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    failures: list[str] = []

    for dependency in manifest.get("dependencies", []):
        name = dependency["name"]
        fetch_prefix = dependency["fetch_prefix"]
        optional = bool(dependency.get("optional", False))
        dep_dir = find_dependency_dir(args.deps_dir, fetch_prefix)

        if dep_dir is None:
            if optional and not args.require_optional:
                continue

            failures.append(f"{name}: dependency directory not found under {args.deps_dir}")

            continue

        ok, detail = license_file_ok(
            dep_dir,
            list(dependency.get("license_files", [])),
            dependency.get("license_snippet"),
        )

        if not ok:
            failures.append(f"{name}: {detail} (in {dep_dir})")

    if failures:
        print("Third-party license scan failures:", file=sys.stderr)

        for failure in failures:
            print(f"  - {failure}", file=sys.stderr)

        return 1

    print("All third-party license checks passed.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
