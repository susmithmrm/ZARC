import hashlib
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from build_evidence import file_record
from supplier_admission import validate

class Tests(unittest.TestCase):
    def test_hash_and_scope(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d)
            p=root/"artifact"; p.write_bytes(b"abc")
            self.assertEqual(file_record(root,p)["sha256"],hashlib.sha256(b"abc").hexdigest())
            self.assertEqual(file_record(root,p)["path"],"artifact")
            with self.assertRaises(ValueError):
                file_record(root/"subdirectory",p)
    def test_supplier(self):
        record={k:"fixture" for k in ("supplier","component","version","maintainer","support_policy",
            "change_policy","license","api_contract","resource_budget","limitations","evidence_reference")}
        record.update(revision="a"*40,status="CANDIDATE",dependencies=[])
        self.assertEqual(validate(record),[])
        record["status"]="SUPPORTED"
        self.assertTrue(validate(record))
        record["status"]="CANDIDATE"
        record["dependencies"]=[{"name":"test","version":"1"}]
        self.assertTrue(validate(record))

if __name__=="__main__":
    unittest.main()
