"""Check a C build's enforced diagnostics and review a SARIF analysis report.

This is a release-audit helper, not a coding-standard approval authority. It
checks only the compiler warning policy configured by this repository and the
contents of supplied artifacts; owners must still approve the coding rules and
any deviations for a release.
"""

from __future__ import annotations

import argparse
from datetime import date
import json
from pathlib import Path
import re
import shlex
import sys


GNU_REQUIRED = {"-Wall", "-Wextra", "-Wpedantic", "-Werror", "-std=c17"}
MSVC_REQUIRED = {"/W4", "/WX", "/std:c17"}


def _tokens(entry: dict) -> list[str]:
    arguments = entry.get("arguments")
    if isinstance(arguments, list) and all(isinstance(item, str) for item in arguments):
        return arguments
    command = entry.get("command")
    if not isinstance(command, str):
        return []
    return shlex.split(command, posix=True)


def _is_msvc(tokens: list[str]) -> bool:
    compiler = Path(tokens[0]).name.lower() if tokens else ""
    return compiler in {"cl", "cl.exe", "clang-cl", "clang-cl.exe"}


def compile_command_errors(database: object) -> tuple[list[str], dict]:
    errors: list[str] = []
    if not isinstance(database, list) or not database:
        return ["compile database must be a non-empty command array"], {}

    c_entries = 0
    compilers: dict[str, int] = {}
    for index, entry in enumerate(database):
        if not isinstance(entry, dict):
            errors.append(f"compile command {index}: entry must be an object")
            continue
        tokens = _tokens(entry)
        if not tokens:
            errors.append(f"compile command {index}: missing command or arguments")
            continue
        source = entry.get("file")
        if not isinstance(source, str) or Path(source).suffix.lower() != ".c":
            continue
        c_entries += 1
        msvc = _is_msvc(tokens)
        compiler_name = "MSVC" if msvc else "GCC/Clang"
        compilers[compiler_name] = compilers.get(compiler_name, 0) + 1
        required = MSVC_REQUIRED if msvc else GNU_REQUIRED
        present = set(tokens)
        missing = sorted(required - present)
        if missing:
            errors.append(f"{source}: missing required {compiler_name} flags: {', '.join(missing)}")

    if c_entries == 0:
        errors.append("compile database contains no C source commands")
    if len(compilers) > 1:
        errors.append("compile database mixes compiler warning policies")
    return errors, {"c_commands": c_entries, "compilers": compilers}


def _finding_key(rule: object, uri: object, line: object) -> tuple[str, str, int] | None:
    if not isinstance(rule, str) or not rule.strip() or not isinstance(uri, str) or not uri:
        return None
    if not isinstance(line, int) or line < 1:
        return None
    normalized_uri = uri.replace("\\", "/")
    return rule, normalized_uri, line


def _deviation_errors(deviations: object, findings: list[tuple[str, str, int]], today: date) -> list[str]:
    errors: list[str] = []
    if not isinstance(deviations, list):
        return ["deviations must be an array"]
    approved: dict[tuple[str, str, int], dict] = {}
    for index, row in enumerate(deviations):
        if not isinstance(row, dict):
            errors.append(f"deviation {index}: entry must be an object")
            continue
        key = _finding_key(row.get("ruleId"), row.get("uri"), row.get("line"))
        if key is None:
            errors.append(f"deviation {index}: requires ruleId, uri and positive line")
            continue
        required_text = ("rationale", "reviewer", "approval", "expires")
        missing = [name for name in required_text if not isinstance(row.get(name), str) or not row[name].strip()]
        if missing:
            errors.append(f"deviation {index}: missing {', '.join(missing)}")
            continue
        try:
            expires = date.fromisoformat(row["expires"])
        except ValueError:
            errors.append(f"deviation {index}: expires must be an ISO date")
            continue
        if expires < today:
            errors.append(f"deviation {index}: approval expired on {expires.isoformat()}")
        if key in approved:
            errors.append(f"deviation {index}: duplicate finding deviation")
        approved[key] = row

    for key in findings:
        if key not in approved:
            errors.append(f"unreviewed analyzer finding: {key[0]} at {key[1]}:{key[2]}")
    return errors


def sarif_errors(sarif: object, deviations: object | None = None, today: date | None = None) -> tuple[list[str], int]:
    errors: list[str] = []
    if not isinstance(sarif, dict) or sarif.get("version") != "2.1.0":
        return ["SARIF report must use version 2.1.0"], 0
    runs = sarif.get("runs")
    if not isinstance(runs, list) or not runs:
        return ["SARIF report must contain at least one run"], 0

    findings: list[tuple[str, str, int]] = []
    for run_index, run in enumerate(runs):
        if not isinstance(run, dict) or not isinstance(run.get("tool"), dict):
            errors.append(f"SARIF run {run_index}: missing tool metadata")
            continue
        results = run.get("results", [])
        if not isinstance(results, list):
            errors.append(f"SARIF run {run_index}: results must be an array")
            continue
        for result_index, result in enumerate(results):
            if not isinstance(result, dict):
                errors.append(f"SARIF run {run_index} finding {result_index}: must be an object")
                continue
            locations = result.get("locations")
            if not isinstance(locations, list) or not locations:
                errors.append(f"SARIF run {run_index} finding {result_index}: missing location")
                continue
            physical = locations[0].get("physicalLocation", {}) if isinstance(locations[0], dict) else {}
            artifact = physical.get("artifactLocation", {}) if isinstance(physical, dict) else {}
            region = physical.get("region", {}) if isinstance(physical, dict) else {}
            key = _finding_key(result.get("ruleId"), artifact.get("uri"), region.get("startLine"))
            if key is None:
                errors.append(f"SARIF run {run_index} finding {result_index}: missing rule or source line")
                continue
            findings.append(key)

    if deviations is not None:
        errors.extend(_deviation_errors(deviations, findings, today or date.today()))
    elif findings:
        errors.append(f"{len(findings)} analyzer finding(s) require reviewed deviations")
    return errors, len(findings)


def _load(path: Path) -> object:
    return json.loads(path.read_text(encoding="utf-8"))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compile-commands", type=Path, required=True)
    parser.add_argument("--sarif", type=Path)
    parser.add_argument("--deviations", type=Path)
    parser.add_argument("--require-sarif", action="store_true", help="fail if no static-analysis report is supplied")
    parser.add_argument("--report", type=Path, help="write a JSON audit summary")
    args = parser.parse_args(argv)

    errors: list[str] = []
    summary: dict = {
        "compiler_policy": "C17; warnings enabled and treated as errors",
        "analyzer": "NOT_PROVIDED",
        "analyzer_findings": None,
        "coding_standard_approval": "NOT_ASSERTED",
        "release_approval": "NOT_ASSERTED",
    }
    try:
        compile_db = _load(args.compile_commands)
        compile_errors, compile_summary = compile_command_errors(compile_db)
        summary.update(compile_summary)
        errors.extend(compile_errors)
    except (OSError, ValueError, TypeError) as exc:
        errors.append(f"cannot read compile database: {exc}")

    if args.sarif:
        try:
            sarif = _load(args.sarif)
            deviations = _load(args.deviations) if args.deviations else None
            analyzer_errors, finding_count = sarif_errors(sarif, deviations)
            summary["analyzer"] = "SARIF_REVIEWED" if not analyzer_errors else "SARIF_REQUIRES_REVIEW"
            summary["analyzer_findings"] = finding_count
            errors.extend(analyzer_errors)
        except (OSError, ValueError, TypeError) as exc:
            summary["analyzer"] = "INVALID_REPORT"
            errors.append(f"cannot read analysis artifact: {exc}")
    elif args.require_sarif:
        errors.append("static-analysis SARIF artifact is required")
    if args.deviations and not args.sarif:
        errors.append("deviations cannot be reviewed without a SARIF artifact")

    summary["result"] = "FAIL" if errors else "STRUCTURAL_CHECKS_PASS"
    summary["errors"] = errors
    rendered = json.dumps(summary, indent=2, sort_keys=True)
    if args.report:
        args.report.write_text(rendered + "\n", encoding="utf-8")
    print(rendered)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
