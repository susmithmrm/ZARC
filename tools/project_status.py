"""Publish candidate requirement, test, and configuration status.

This report is deliberately descriptive. It cannot turn open evidence into an
accepted result or promote a candidate target to supported status.
"""
import argparse
import json
from pathlib import Path
import sys
import xml.etree.ElementTree as ET

from target_manifest import validate as validate_target
from verify_evidence import read_csv, trace_errors


def _junit_status(path):
    if path is None:
        return {"state": "NOT_RUN", "total": 0, "passed": 0, "failed": 0,
                "skipped": 0, "failures": [], "source": None}
    root = ET.parse(path).getroot()
    cases = root.findall(".//testcase")
    passed = failed = skipped = 0
    failures = []
    for case in cases:
        name = case.get("name", "unnamed")
        suite = case.get("classname", "")
        label = f"{suite}.{name}" if suite else name
        if case.find("failure") is not None or case.find("error") is not None:
            failed += 1
            node = case.find("failure")
            if node is None:
                node = case.find("error")
            failures.append({"test": label,
                             "message": node.get("message", "") if node is not None else ""})
        elif case.find("skipped") is not None:
            skipped += 1
        else:
            passed += 1
    return {"state": "RECORDED", "total": len(cases), "passed": passed,
            "failed": failed, "skipped": skipped, "failures": failures,
            "source": str(path)}


def _configuration_status(root):
    records = []
    errors = []
    for path in sorted((root / "config/targets").glob("*.json")):
        try:
            manifest = json.loads(path.read_text(encoding="utf-8"))
            validate_target(manifest)
            valid = True
            error = None
        except (OSError, ValueError, TypeError, KeyError, json.JSONDecodeError) as exc:
            manifest = {}
            valid = False
            error = str(exc)
            errors.append(f"{path.relative_to(root)}: {error}")
        maturity = manifest.get("maturity", "UNKNOWN")
        records.append({
            "manifest": path.relative_to(root).as_posix(),
            "target": manifest.get("target", path.stem),
            "profile": manifest.get("profile", "UNKNOWN"),
            "board": manifest.get("board", "UNKNOWN"),
            "soc": manifest.get("soc", "UNKNOWN"),
            "revision": manifest.get("revision", "UNKNOWN"),
            "zephyr": manifest.get("zephyr", "UNKNOWN"),
            "toolchain": manifest.get("toolchain", "UNKNOWN"),
            "maintainer": manifest.get("maintainer", "UNKNOWN"),
            "maturity": maturity,
            "manifest_valid": valid,
            "status": "CANDIDATE" if valid and maturity == "CANDIDATE" else "NOT_SUPPORTED",
            "features": manifest.get("features", {}),
            "limitations": manifest.get("limitations", []),
            "validation_error": error,
        })
    if not records:
        errors.append("no target manifests found")
    return records, errors


def build_support_matrix(root):
    """Build the public matrix only from validated, explicitly candidate records."""
    root = Path(root).resolve()
    records, errors = _configuration_status(root)
    if errors:
        raise ValueError("invalid target maturity records: " + "; ".join(errors))
    candidates = [record for record in records if record["status"] == "CANDIDATE"]
    if len(candidates) != len(records):
        raise ValueError("support matrix contains a non-candidate record")
    return {
        "schema": 1,
        "source": "config/targets/*.json",
        "scope": "candidate simulator records only; no production target is supported",
        "production_supported": [],
        "records": candidates,
    }


def build_report(root, junit=None):
    root = Path(root).resolve()
    baseline, trace_validation_errors = trace_errors(root)
    trace = read_csv(root / "docs/REQUIREMENT-TRACE.csv")
    requirements = [{"id": row["Requirement"], "status": row["Status"],
                     "missing_evidence": row["Open_evidence"]}
                    for row in trace]
    configs, config_errors = _configuration_status(root)
    errors = trace_validation_errors + config_errors
    tests = _junit_status(junit)
    if tests["state"] == "RECORDED" and tests["total"] == 0:
        errors.append("test results contain no test cases")
    counts = {}
    missing = []
    for row in requirements:
        counts[row["status"]] = counts.get(row["status"], 0) + 1
        if row["missing_evidence"]:
            missing.append({"id": row["id"], "gap": row["missing_evidence"]})
    return {
        "schema": 1,
        "report_scope": "candidate status only; this report is not product approval or qualification evidence",
        "requirements": {"total": len(baseline), "status_counts": counts,
                         "records": requirements, "trace_errors": trace_validation_errors},
        "tests": tests,
        "missing_evidence": missing,
        "configurations": {"records": configs, "production_supported": [],
                           "statement": "Candidate manifests are not supported-product claims."},
        "validation_errors": errors,
        "release_status": "OPEN" if errors or missing or tests["state"] != "RECORDED" or tests["failed"] else "CANDIDATE_ONLY",
    }


def markdown(report):
    lines = ["# ZARC Candidate Status", "",
             f"> {report['report_scope']}", "",
             f"Release status: **{report['release_status']}**", "",
             "## Requirements", "",
             f"Total: {report['requirements']['total']}  ",
             "Status counts: " + ", ".join(
                 f"{key}={value}" for key, value in sorted(report['requirements']['status_counts'].items())), "",
             "## Test results", "",
             f"State: {report['tests']['state']}; total {report['tests']['total']}, "
             f"passed {report['tests']['passed']}, failed {report['tests']['failed']}, "
             f"skipped {report['tests']['skipped']}", ""]
    if report["tests"]["failures"]:
        lines.extend(["Failed tests:", ""])
        lines.extend(f"- `{item['test']}`: {item['message']}" for item in report["tests"]["failures"])
        lines.append("")
    lines.extend(["## Configurations", "", report["configurations"]["statement"], "",
                  "| Target | Profile | Status | Manifest |", "|---|---|---|---|"])
    lines.extend(f"| {x['target']} | {x['profile']} | {x['status']} | `{x['manifest']}` |"
                 for x in report["configurations"]["records"])
    lines.extend(["", "## Missing evidence", ""])
    if report["missing_evidence"]:
        lines.extend(f"- **{x['id']}**: {x['gap']}" for x in report["missing_evidence"])
    else:
        lines.append("No open evidence gaps recorded.")
    lines.append("")
    if report["validation_errors"]:
        lines.extend(["## Report validation errors", ""])
        lines.extend(f"- {error}" for error in report["validation_errors"])
        lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--junit", type=Path,
                        help="CTest/JUnit XML from this candidate run; omitted means tests are NOT RUN")
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--markdown-out", type=Path)
    parser.add_argument("--support-matrix-out", type=Path,
                        help="write the deterministic public target matrix")
    parser.add_argument("--check-support-matrix", action="store_true",
                        help="fail if the generated matrix differs from the committed file")
    args = parser.parse_args()
    try:
        report = build_report(args.root, args.junit)
        matrix = build_support_matrix(args.root) if args.support_matrix_out or args.check_support_matrix else None
    except (OSError, ET.ParseError, ValueError, TypeError, KeyError) as exc:
        print(f"cannot build status report: {exc}", file=sys.stderr)
        return 2
    rendered = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.json_out:
        args.json_out.write_text(rendered, encoding="utf-8")
    if args.markdown_out:
        args.markdown_out.write_text(markdown(report), encoding="utf-8")
    if args.support_matrix_out and not args.check_support_matrix:
        args.support_matrix_out.parent.mkdir(parents=True, exist_ok=True)
        args.support_matrix_out.write_text(json.dumps(matrix, indent=2, sort_keys=True) + "\n",
                                           encoding="utf-8")
    if args.check_support_matrix:
        if not args.support_matrix_out:
            parser.error("--check-support-matrix requires --support-matrix-out")
        expected = json.dumps(matrix, indent=2, sort_keys=True) + "\n"
        try:
            actual = args.support_matrix_out.read_text(encoding="utf-8")
        except OSError as exc:
            print(f"cannot read support matrix: {exc}", file=sys.stderr)
            return 1
        if actual != expected:
            print("generated support matrix differs from committed output", file=sys.stderr)
            return 1
    if not args.json_out and not args.markdown_out and not args.support_matrix_out:
        print(markdown(report))
    elif args.support_matrix_out and not args.json_out and not args.markdown_out:
        print(f"Validated {len(matrix['records'])} candidate target records; no production support claimed")
    return 1 if report["validation_errors"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
