import sys
from datetime import date
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from coding_audit import compile_command_errors, sarif_errors


class CodingAuditTests(unittest.TestCase):
    def test_gnu_c17_warning_policy_is_checked_per_c_translation_unit(self):
        good = {
            "file": "src/control.c",
            "arguments": ["/usr/bin/cc", "-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-c", "src/control.c"],
        }
        cxx = {"file": "host/peer.cpp", "arguments": ["c++", "host/peer.cpp"]}
        errors, summary = compile_command_errors([good, cxx])
        self.assertEqual(errors, [])
        self.assertEqual(summary["c_commands"], 1)

    def test_missing_warning_flag_fails(self):
        entry = {
            "file": "src/control.c",
            "command": "cc -std=c17 -Wall -Wextra -Wpedantic -c src/control.c",
        }
        errors, _ = compile_command_errors([entry])
        self.assertTrue(any("-Werror" in error for error in errors))

    def test_msvc_policy_is_checked(self):
        entry = {"file": "src/control.c", "arguments": ["cl.exe", "/std:c17", "/W4", "/WX", "src/control.c"]}
        self.assertEqual(compile_command_errors([entry])[0], [])

    def test_empty_or_malformed_compile_database_fails(self):
        self.assertTrue(compile_command_errors([])[0])
        self.assertTrue(compile_command_errors([{"file": "src/a.c"}])[0])

    @staticmethod
    def report(results):
        return {
            "version": "2.1.0",
            "runs": [{
                "tool": {"driver": {"name": "fixture-analyzer"}},
                "results": results,
            }],
        }

    @staticmethod
    def finding():
        return {
            "ruleId": "rule-7",
            "level": "warning",
            "locations": [{"physicalLocation": {
                "artifactLocation": {"uri": "src/control.c"},
                "region": {"startLine": 21},
            }}],
        }

    def test_empty_sarif_is_a_clean_analyzer_run(self):
        self.assertEqual(sarif_errors(self.report([]))[0], [])

    def test_finding_requires_exact_approved_deviation(self):
        report = self.report([self.finding()])
        self.assertTrue(sarif_errors(report)[0])
        deviation = [{
            "ruleId": "rule-7", "uri": "src/control.c", "line": 21,
            "rationale": "Legacy vendor header boundary", "reviewer": "reviewer",
            "approval": "CHANGE-42", "expires": "2099-12-31",
        }]
        self.assertEqual(sarif_errors(report, deviation, date(2026, 1, 1))[0], [])

    def test_expired_deviation_does_not_suppress_finding(self):
        deviation = [{
            "ruleId": "rule-7", "uri": "src/control.c", "line": 21,
            "rationale": "fixture", "reviewer": "reviewer",
            "approval": "CHANGE-42", "expires": "2025-12-31",
        }]
        errors, _ = sarif_errors(self.report([self.finding()]), deviation, date(2026, 1, 1))
        self.assertTrue(any("expired" in error for error in errors))

    def test_invalid_sarif_version_fails(self):
        errors, _ = sarif_errors({"version": "1.0", "runs": []})
        self.assertTrue(errors)


if __name__ == "__main__":
    unittest.main()
