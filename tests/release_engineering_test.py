import hashlib
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from build_evidence import file_record
from supplier_admission import validate,validate_api_lifecycle,upgrade_is_compatible

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
        record.update(revision="a"*40,status="CANDIDATE",dependencies=[],api_lifecycle={
            "supported_versions":["1.0","2.0","3.0"],
            "deprecated_versions":["1.0"],
            "compatible_upgrades":[["1.0","2.0"],["2.0","3.0"]]})
        self.assertEqual(validate(record),[])
        record["status"]="SUPPORTED"
        self.assertTrue(validate(record))
        record["status"]="CANDIDATE"
        record["dependencies"]=[{"name":"test","version":"1"}]
        self.assertTrue(validate(record))

    def test_api_lifecycle_requires_closed_exact_version_declarations(self):
        valid={"supported_versions":["v1","v2","v3"],
               "deprecated_versions":["v1"],
               "compatible_upgrades":[["v1","v2"],["v2","v3"]]}
        self.assertTrue(validate_api_lifecycle(valid))
        self.assertTrue(upgrade_is_compatible(valid,"v1","v2"))
        self.assertFalse(upgrade_is_compatible(valid,"v2","v1"))
        self.assertFalse(upgrade_is_compatible(valid,"v3","v4"))
        self.assertFalse(upgrade_is_compatible(valid," ","v2"))
        invalid=[
            {"supported_versions":["v1","v1"],"deprecated_versions":[],"compatible_upgrades":[]},
            {"supported_versions":["v1"],"deprecated_versions":["v2"],"compatible_upgrades":[]},
            {"supported_versions":["v1"],"deprecated_versions":[],"compatible_upgrades":[["v1","v2"]]},
            {"supported_versions":["v1","v2"],"deprecated_versions":["v2"],"compatible_upgrades":[["v1","v2"]]},
            {"supported_versions":["v1","v2"],"deprecated_versions":[],"compatible_upgrades":[["v1","v2"],["v1","v2"]]},
            {"supported_versions":["v1","v2"],"deprecated_versions":[],"compatible_upgrades":[["v1","v1"]]},
            {"supported_versions":[True],"deprecated_versions":[],"compatible_upgrades":[]},
            {"supported_versions":["v1"],"deprecated_versions":[],"compatible_upgrades":[],"latest":"v1"},
        ]
        for policy in invalid:
            with self.subTest(policy=policy),self.assertRaises(ValueError):
                validate_api_lifecycle(policy)
        record={k:"fixture" for k in ("supplier","component","version","maintainer","support_policy",
            "change_policy","license","api_contract","resource_budget","limitations","evidence_reference")}
        record.update(revision="a"*40,status="CANDIDATE",dependencies=[])
        self.assertTrue(any("API lifecycle policy" in issue for issue in validate(record)))

if __name__=="__main__":
    unittest.main()
