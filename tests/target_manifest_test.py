import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"tools"))
from target_manifest import validate,generate,reconcile_kconfig
from timing_analysis import analyze

class Tests(unittest.TestCase):
    def setUp(self):
        self.m=json.loads((ROOT/"config/targets/native-sim-edge.json").read_text())
    def test_candidate(self):
        self.assertEqual(validate(self.m),[1,2])
        self.assertEqual(generate(self.m),generate(copy.deepcopy(self.m)))
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/".config";p.write_text("# CONFIG_CAN is not set\n")
            reconcile_kconfig(self.m,p)
            p.write_text("CONFIG_CAN=y\n")
            with self.assertRaises(ValueError):reconcile_kconfig(self.m,p)
    def test_invalid_profiles(self):
        for change in ("cycle","duplicate","unsupported","owner","profile","flash","uart"):
            m=copy.deepcopy(self.m)
            if change=="cycle":m["services"][0]["dependencies"]=[2]
            elif change=="duplicate":m["resources"].append(m["resources"][0])
            elif change=="unsupported":m["services"][0]["supported"]=False
            elif change=="owner":del m["resources"][0]["owners"]["reset"]
            elif change=="profile":m["profile"]="companion"
            elif change=="flash":m["features"]["mcuboot"]=True
            elif change=="uart":m["features"]["uart"]=m["uart_console_shared"]=True
            with self.subTest(change=change),self.assertRaises(ValueError):validate(m)
    def test_timing(self):
        assumptions=dict(uniprocessor=True,fully_preemptive=True,no_self_suspension=True)
        tasks=self.m["tasks"]
        result=analyze(tasks,**assumptions)
        self.assertTrue(all(x["admitted"] for x in result.values()))
        tasks[0]["execution_us"]=9500
        self.assertFalse(analyze(tasks,**assumptions)["virtual_device"]["admitted"])
        with self.assertRaises(ValueError):analyze(tasks,**{**assumptions,"uniprocessor":False})

if __name__=="__main__":unittest.main()
