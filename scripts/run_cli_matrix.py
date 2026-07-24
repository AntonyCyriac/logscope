#!/usr/bin/env python3
"""Run a matrix of LogScope CLI command combinations against bulk log fixtures."""

from __future__ import annotations

import argparse
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Callable


@dataclass(frozen=True)
class Scenario:
    name: str
    args: tuple[str, ...]
    expect_stdout: str | None = None
    output_file: str | None = None
    file_predicate: Callable[[Path], bool] | None = None


def run_command(logscope: Path, args: tuple[str, ...]) -> subprocess.CompletedProcess[str]:
    command = [str(logscope), *args]
    return subprocess.run(command, check=False, capture_output=True, text=True)


def default_scenarios(plain_log: Path, jsonl_log: Path, config_file: Path, work_dir: Path) -> list[Scenario]:
    html_output = work_dir / "report.html"
    pdf_output = work_dir / "report.pdf"
    session_file = work_dir / "matrix.session"

    return [
        Scenario("analyze-text", ("analyze", str(plain_log)), expect_stdout="Total log lines"),
        Scenario("analyze-json", ("analyze", "--format", "json", str(plain_log)), expect_stdout='"summary"'),
        Scenario("analyze-csv", ("analyze", "--format", "csv", str(plain_log)), expect_stdout="section,key,value"),
        Scenario(
            "analyze-markdown",
            ("analyze", "--format", "markdown", "--sections", "summary,levels", str(plain_log)),
            expect_stdout="# LogScope Report",
        ),
        Scenario(
            "analyze-html-file",
            ("analyze", "--format", "html", "--sections", "executive,summary,charts", "--output", str(html_output), str(plain_log)),
            output_file=str(html_output),
            file_predicate=lambda path: "<!DOCTYPE html>" in path.read_text(encoding="utf-8"),
        ),
        Scenario(
            "analyze-pdf-file",
            ("analyze", "--format", "pdf", "--output", str(pdf_output), str(plain_log)),
            output_file=str(pdf_output),
            file_predicate=lambda path: path.stat().st_size > 100,
        ),
        Scenario(
            "analyze-sections-analytics",
            ("analyze", "--sections", "analytics,timeline,clusters", str(plain_log)),
            expect_stdout="Analytics",
        ),
        Scenario(
            "analyze-jsonl",
            ("analyze", "--log-format", "jsonl", str(jsonl_log)),
            expect_stdout="Total log lines",
        ),
        Scenario(
            "analyze-profile-json",
            ("analyze", "--profile", "generic-json", str(jsonl_log)),
            expect_stdout="Total log lines",
        ),
        Scenario(
            "investigate-query",
            ("investigate", "--query", "error AND timeout", str(plain_log)),
            expect_stdout="INVESTIGATION RESULT",
        ),
        Scenario(
            "search-text",
            ("search", "--search", "Connection refused", str(plain_log)),
            expect_stdout="INVESTIGATION RESULT",
        ),
        Scenario(
            "search-json",
            ("search", "--format", "json", "--query", "error", str(plain_log)),
            expect_stdout='"matchingLineCount"',
        ),
        Scenario(
            "investigate-filter",
            ("investigate", "--filter", "level == ERROR", str(plain_log)),
            expect_stdout="INVESTIGATION RESULT",
        ),
        Scenario(
            "query-filter",
            ("query", "--filter", "level == ERROR", str(plain_log)),
            expect_stdout="INVESTIGATION RESULT",
        ),
        Scenario(
            "analytics-text",
            ("analytics", str(plain_log)),
            expect_stdout="Analytics summary",
        ),
        Scenario(
            "analytics-json",
            ("analytics", "--format", "json", "--bucket", "60", "--top", "5", str(plain_log)),
            expect_stdout='"trendVerdict"',
        ),
        Scenario(
            "config-validate",
            ("config", "validate", "--config", str(config_file), "--require", "log.level"),
            expect_stdout="Configuration is valid.",
        ),
        Scenario("extensions-list", ("extensions", "list"), expect_stdout="analysis.log-levels"),
        Scenario(
            "session-save",
            ("session", "save", str(session_file), "--min-errors", "1", str(plain_log)),
            expect_stdout="Session saved to",
        ),
        Scenario(
            "session-load",
            ("session", "load", str(session_file)),
            expect_stdout="Matches     : yes",
        ),
    ]


def verify_scenario(logscope: Path, scenario: Scenario) -> tuple[bool, str]:
    result = run_command(logscope, scenario.args)

    if result.returncode != 0:
        details = result.stderr.strip() or result.stdout.strip() or f"exit code {result.returncode}"
        return False, details

    if scenario.expect_stdout and scenario.expect_stdout not in result.stdout:
        return False, f"expected stdout to contain {scenario.expect_stdout!r}"

    if scenario.output_file is not None:
        output_path = Path(scenario.output_file)

        if not output_path.exists():
            return False, f"expected output file {output_path}"

        if scenario.file_predicate is not None and not scenario.file_predicate(output_path):
            return False, f"output file check failed for {output_path}"

    return True, "ok"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--logscope", type=Path, required=True)
    parser.add_argument("--plain-log", type=Path, required=True)
    parser.add_argument("--jsonl-log", type=Path, required=True)
    parser.add_argument("--config", type=Path, default=Path("samples/logscope.properties"))
    parser.add_argument("--work-dir", type=Path)
    args = parser.parse_args()

    if not args.logscope.exists():
        raise SystemExit(f"logscope binary not found: {args.logscope}")

    for fixture in (args.plain_log, args.jsonl_log, args.config):
        if not fixture.exists():
            raise SystemExit(f"fixture not found: {fixture}")

    with tempfile.TemporaryDirectory(prefix="logscope-cli-matrix-") as temp_directory:
        work_dir = args.work_dir or Path(temp_directory)
        work_dir.mkdir(parents=True, exist_ok=True)

        scenarios = default_scenarios(args.plain_log, args.jsonl_log, args.config, work_dir)
        failures: list[str] = []

        print(f"Running {len(scenarios)} CLI scenarios against {args.plain_log.name}")

        for scenario in scenarios:
            ok, message = verify_scenario(args.logscope, scenario)
            status = "PASS" if ok else "FAIL"
            print(f"[{status}] {scenario.name}")

            if not ok:
                failures.append(f"{scenario.name}: {message}")

        if failures:
            print("\nCLI matrix failures:", file=sys.stderr)

            for failure in failures:
                print(f"  - {failure}", file=sys.stderr)

            return 1

        print("All CLI matrix scenarios passed.")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
