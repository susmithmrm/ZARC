import re
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
README = ROOT / "README.md"
LOCALIZATION = ROOT / "docs" / "localization"


def fenced_blocks(text):
    return [(language.strip(), body.strip()) for language, body in
            re.findall(r"(?ms)^```([^\n]*)\n(.*?)^```\s*$", text)]


class LocalizationTests(unittest.TestCase):
    def test_glossary_covers_required_languages_and_terms(self):
        text = (LOCALIZATION / "GLOSSARY.md").read_text(encoding="utf-8")
        self.assertIn("| English | 简体中文 | 한국어 |", text)
        required = (
            "Zephyr RTOS", "portable core", "target manifest",
            "board support package (BSP)", "native adapter", "requested state",
            "observed state", "candidate configuration", "fallback behavior",
            "software-in-the-loop (SIL)", "hardware-in-the-loop (HIL)",
            "review required", "unknown outcome", "support window",
        )
        rows = [line for line in text.splitlines() if line.startswith("|")][2:]
        english = {row.split("|", 2)[1].strip(): row for row in rows}
        for term in required:
            with self.subTest(term=term):
                self.assertIn(term, english)
                columns = [value.strip() for value in english[term].strip("|").split("|")]
                self.assertEqual(len(columns), 3)
                self.assertTrue(all(columns))
        self.assertIn("draft", text.lower())
        self.assertIn("native-language technical reviewers", " ".join(text.split()))

    def test_quickstarts_link_sources_and_preserve_readme_code_blocks(self):
        readme = README.read_text(encoding="utf-8")
        source = re.search(r"(?ms)^## Quick start\s*\n(.*?)(?=^## |\Z)", readme)
        self.assertIsNotNone(source)
        expected = fenced_blocks(source.group(1))
        self.assertEqual(expected, [
            ("powershell", "cmake --preset desktop-windows\n"
             "cmake --build --preset desktop-windows\n"
             "ctest --preset desktop-windows"),
            ("bash", "cmake --preset desktop-linux\n"
             "cmake --build --preset desktop-linux\n"
             "ctest --preset desktop-linux"),
        ])
        for name in ("quickstart.zh-CN.md", "quickstart.ko.md"):
            path = LOCALIZATION / name
            text = path.read_text(encoding="utf-8")
            with self.subTest(language=name):
                self.assertEqual(fenced_blocks(text), expected)
                self.assertIn("../../README.md#quick-start", text)
                self.assertIn("../IMPLEMENTATION-STATUS.md", text)
                self.assertIn("GLOSSARY.md", text)
                self.assertIn("WORKFLOW.md", text)
                self.assertRegex(text.lower(), r"draft|초안|草稿")
                for relative in ("../../README.md", "../IMPLEMENTATION-STATUS.md",
                                 "GLOSSARY.md", "WORKFLOW.md"):
                    self.assertTrue((path.parent / relative).resolve().exists(), relative)
                self.assertNotIn("single simulated principal/session", text)
                self.assertNotIn("단일 모의 주체와 세션", text)
                self.assertNotIn("单一模拟主体和会话", text)
                self.assertRegex(text.lower(), r"physical|실제 하드웨어|真实硬件")

    def test_workflow_preserves_source_and_review_status(self):
        text = (LOCALIZATION / "WORKFLOW.md").read_text(encoding="utf-8")
        self.assertIn("copy command and code blocks exactly", text)
        self.assertIn("native-language technical reviewer", text)
        self.assertIn("partner acceptance", text.lower())
        normalized = " ".join(text.split())
        for name in ("quickstart.zh-CN.md", "quickstart.ko.md"):
            self.assertIn(name, normalized)


if __name__ == "__main__":
    unittest.main()
