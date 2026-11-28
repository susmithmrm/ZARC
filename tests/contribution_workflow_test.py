from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class ContributionWorkflowTests(unittest.TestCase):
    def test_contribution_process_has_owner_and_authoritative_baseline(self):
        guide = (ROOT / "CONTRIBUTING.md").read_text(encoding="utf-8")
        owners = (ROOT / ".github" / "CODEOWNERS").read_text(encoding="utf-8")
        template = (ROOT / ".github" / "PULL_REQUEST_TEMPLATE.md").read_text(encoding="utf-8")
        self.assertIn("docs/architecture/12-Requirements-Verification.csv", guide)
        self.assertIn("only requirements baseline", guide)
        self.assertIn("@susmithmrm", owners)
        for section in ("Requirement trace", "Verification", "Evidence limits"):
            self.assertIn(section, template)
        self.assertIn("Known open acceptance criteria", template)


if __name__ == "__main__":
    unittest.main()
