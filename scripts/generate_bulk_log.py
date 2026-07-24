#!/usr/bin/env python3
"""Generate synthetic bulk log files for CLI matrix and release smoke tests."""

from __future__ import annotations

import argparse
import json
import random
from datetime import datetime, timedelta, timezone
from pathlib import Path

LEVELS = ("INFO", "WARNING", "ERROR")
MESSAGES = (
    "Application started",
    "Connecting to database",
    "Connection refused",
    "Request timeout",
    "Retrying request",
    "Error code=500",
    "Error code=503",
    "Cache miss for key=user-42",
    "Health check passed",
)


def write_plain_log(path: Path, line_count: int, seed: int) -> None:
    random.seed(seed)
    start = datetime(2026, 7, 11, 10, 0, 0, tzinfo=timezone.utc)

    with path.open("w", encoding="utf-8", newline="\n") as stream:
        for index in range(line_count):
            timestamp = start + timedelta(seconds=index)
            level = LEVELS[index % 7 % 3]
            message = MESSAGES[index % len(MESSAGES)]
            correlation = f"trace-{index % 128:03d}"
            stream.write(
                f"{timestamp:%Y-%m-%d %H:%M:%S} {level} [{correlation}] {message}\n"
            )


def write_jsonl_log(path: Path, line_count: int, seed: int) -> None:
    random.seed(seed)
    start = datetime(2026, 7, 21, 10, 0, 0, tzinfo=timezone.utc)

    with path.open("w", encoding="utf-8", newline="\n") as stream:
        for index in range(line_count):
            timestamp = (start + timedelta(seconds=index)).isoformat().replace("+00:00", "Z")
            level = ("info", "warning", "error")[index % 7 % 3]
            message = MESSAGES[index % len(MESSAGES)]
            payload = {
                "timestamp": timestamp,
                "level": level,
                "message": message,
                "correlationId": f"trace-{index % 128:03d}",
            }
            stream.write(json.dumps(payload, separators=(",", ":")) + "\n")

            if index % 997 == 0 and index > 0:
                stream.write("not valid json\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lines", type=int, default=10_000, help="Number of log lines to generate")
    parser.add_argument("--format", choices=("plain", "jsonl"), required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--seed", type=int, default=42)
    args = parser.parse_args()

    if args.lines < 1:
        raise SystemExit("--lines must be at least 1")

    args.output.parent.mkdir(parents=True, exist_ok=True)

    if args.format == "plain":
        write_plain_log(args.output, args.lines, args.seed)
    else:
        write_jsonl_log(args.output, args.lines, args.seed)

    print(f"Wrote {args.lines} {args.format} lines to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
