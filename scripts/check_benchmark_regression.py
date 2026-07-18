#!/usr/bin/env python3
"""Compare Google Benchmark JSON output against committed baselines."""

from __future__ import annotations

import json
import sys
from pathlib import Path


def load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def benchmark_map(results: dict) -> dict[str, float]:
    mapped: dict[str, float] = {}

    for benchmark in results.get("benchmarks", []):
        name = benchmark.get("name", "")
        items_per_second = benchmark.get("items_per_second")

        if items_per_second is not None:
            mapped[name] = float(items_per_second)

    return mapped


def main() -> int:
    if len(sys.argv) != 3:
        print("Usage: check_benchmark_regression.py <baseline.json> <results.json>")
        return 1

    baseline_path = Path(sys.argv[1])
    results_path = Path(sys.argv[2])

    baseline = load_json(baseline_path)
    results = load_json(results_path)
    measured = benchmark_map(results)

    warnings = 0

    for entry in baseline.get("benchmarks", []):
        name = entry["name"]
        minimum = float(entry["minimum"])
        tolerance = float(entry.get("regression_tolerance_percent", 20))
        threshold = minimum * (1.0 - tolerance / 100.0)

        if name not in measured:
            print(f"WARNING: benchmark '{name}' not found in results")
            warnings += 1
            continue

        value = measured[name]

        if value < threshold:
            print(
                f"WARNING: {name} throughput {value:.0f} lines/sec "
                f"below threshold {threshold:.0f} (baseline minimum {minimum:.0f})"
            )
            warnings += 1
        else:
            print(f"OK: {name} = {value:.0f} lines/sec (threshold {threshold:.0f})")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
