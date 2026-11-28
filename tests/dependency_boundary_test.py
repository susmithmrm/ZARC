import sys
import tempfile
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from dependency_boundary import dependency_errors


class DependencyBoundaryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        (self.root / "src").mkdir()
        (self.root / "include" / "zarc").mkdir(parents=True)
        (self.root / "platform" / "zephyr" / "adapters").mkdir(parents=True)
        (self.root / "src" / "core.c").write_text('#include "zarc/core.h"\n')
        (self.root / "include" / "zarc" / "core.h").write_text("#include <stdint.h>\n")

    def tearDown(self):
        self.temp.cleanup()

    def command(self, source="src/core.c", *flags):
        return {"directory": str(self.root), "file": source,
                "arguments": ["cc", *flags, "-c", source]}

    def test_generic_source_and_native_adapter_compile_separately(self):
        adapter = self.command("platform/zephyr/adapters/device.c",
                               "-Iinclude", "-Iplatform/zephyr/adapters")
        errors, summary = dependency_errors(
            [self.command("src/core.c", "-Iinclude"), adapter], self.root)
        self.assertEqual(errors, [])
        self.assertEqual(summary["selected_targets"], "NOT_ASSERTED")

    def test_private_platform_include_path_in_generic_compile_fails(self):
        errors, _ = dependency_errors(
            [self.command("src/core.c", "-Iinclude", "-Iplatform/vendor/private/include")], self.root)
        self.assertTrue(any("private include path" in error for error in errors))

    def test_forced_private_header_fails(self):
        errors, _ = dependency_errors(
            [self.command("src/core.c", "-Iinclude", "-include",
                          "vendor/chip/private_config.h")], self.root)
        self.assertTrue(any("private include path" in error for error in errors))

    def test_private_header_reference_in_generic_source_fails(self):
        (self.root / "src" / "core.c").write_text('#include "vendor/chip/registers.h"\n')
        errors, _ = dependency_errors([self.command("src/core.c", "-Iinclude")], self.root)
        self.assertTrue(any("private header include" in error for error in errors))

    def test_missing_generic_translation_unit_fails(self):
        errors, _ = dependency_errors(
            [self.command("platform/zephyr/adapters/device.c")], self.root)
        self.assertTrue(any("no generic src/*.c" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
