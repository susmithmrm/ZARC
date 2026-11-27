import hashlib
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from verify_evidence import trace_errors,evidence_errors,ROOT

class Tests(unittest.TestCase):
    def test_trace(self):
        base, errors=trace_errors(ROOT)
        self.assertEqual(len(base),111)
        self.assertEqual(errors,[])
    def test_fail_closed(self):
        base={"X":{"Verification_ID":"T-X","Method":"HIL","Profiles":"ALL"}}
        commit="a"*40
        with tempfile.TemporaryDirectory() as directory:
            directory=Path(directory)
            config=directory/"manifest.json"
            config.write_text('{"fixture":true}',encoding="utf-8")
            manifest={"configuration":"bench","commit":commit,
                      "manifest_artifact":"manifest.json","manifest_sha256":hashlib.sha256(config.read_bytes()).hexdigest(),
                      "toolchain":"fixture","reviewer":"fixture","approval_record":"fixture","requirements":[]}
            self.assertTrue(evidence_errors(base,manifest,directory,"bench",commit))
            row={"id":"X","verification":"T-X","method":"HIL","applicability":"NOT_APPLICABLE",
                 "rationale":"fixture","reviewer":"fixture","approval":"fixture"}
            manifest["requirements"]=[row]
            self.assertTrue(evidence_errors(base,manifest,directory,"bench",commit))
            raw=directory/"raw.log";raw.write_bytes(b"synthetic evidence for gate unit test only")
            row.update(applicability="APPLICABLE",verdict="PASS",environment="fixture",
                       procedure="fixture",expected="fixture",actual="fixture",artifact="raw.log",
                       sha256=hashlib.sha256(raw.read_bytes()).hexdigest())
            self.assertFalse(evidence_errors(base,manifest,directory,"bench",commit))
            config.write_text("changed",encoding="utf-8")
            self.assertTrue(evidence_errors(base,manifest,directory,"bench",commit))
            config.write_text('{"fixture":true}',encoding="utf-8")
            raw.write_bytes(b"changed")
            self.assertTrue(evidence_errors(base,manifest,directory,"bench",commit))

if __name__=="__main__":
    unittest.main()
