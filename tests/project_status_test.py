import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from project_status import _configuration_status, _junit_status, build_report, markdown


class ProjectStatusTests(unittest.TestCase):
    def test_report_includes_gaps_tests_and_candidate_only_targets(self):
        report = build_report(ROOT)
        self.assertEqual(report["requirements"]["total"], 178)
        self.assertEqual(len(report["requirements"]["records"]), 178)
        self.assertGreater(len(report["missing_evidence"]), 0)
        self.assertEqual(report["tests"]["state"], "NOT_RUN")
        self.assertEqual(report["configurations"]["production_supported"], [])
        self.assertTrue(all(x["status"] == "CANDIDATE"
                            for x in report["configurations"]["records"]))
        self.assertIn("not product approval", report["report_scope"])
        self.assertIn("Missing evidence", markdown(report))

    def test_junit_pass_fail_and_skip_boundary(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "results.xml"
            path.write_text("""<testsuites tests='3' failures='1'>
              <testsuite><testcase classname='suite' name='ok'/>
              <testcase classname='suite' name='bad'><failure message='broken'/></testcase>
              <testcase classname='suite' name='skip'><skipped/></testcase></testsuite>
              </testsuites>""", encoding="utf-8")
            result = _junit_status(path)
        self.assertEqual((result["total"], result["passed"], result["failed"], result["skipped"]),
                         (3, 1, 1, 1))
        self.assertEqual(result["failures"][0]["test"], "suite.bad")
        self.assertEqual(result["failures"][0]["message"], "broken")

    def test_empty_or_missing_results_never_look_passed(self):
        self.assertEqual(_junit_status(None)["state"], "NOT_RUN")
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "empty.xml"
            path.write_text("<testsuites><testsuite/></testsuites>", encoding="utf-8")
            self.assertEqual(_junit_status(path)["total"], 0)

    def test_invalid_manifest_is_not_reported_as_supported(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            directory = root / "config/targets"
            directory.mkdir(parents=True)
            (directory / "bad.json").write_text(json.dumps({"maturity": "SUPPORTED"}), encoding="utf-8")
            records, errors = _configuration_status(root)
        self.assertEqual(records[0]["status"], "NOT_SUPPORTED")
        self.assertFalse(records[0]["manifest_valid"])
        self.assertTrue(errors)


if __name__ == "__main__":
    unittest.main()
