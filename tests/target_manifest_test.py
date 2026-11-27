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
    def test_runtime_allocation(self):
        header=generate(self.m)
        self.assertIn("#define ZARC_RUNTIME_PERIOD_MS 10u",header)
        self.assertIn("#define ZARC_RUNTIME_MAIN_STACK 8192u",header)
        for key,value in (("deadline_ms",11),("period_ms",12),("health_limit_ms",10)):
            m=copy.deepcopy(self.m);m["benign_runtime"][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):validate(m)
        m=copy.deepcopy(self.m);m["tasks"][0]["deadline_us"]=9000
        with self.assertRaises(ValueError):validate(m)
    def test_profile_variants(self):
        for name,count in (("companion",1),("zonal",3),("integrated",2)):
            m=json.loads((ROOT/f"config/targets/native-sim-{name}.json").read_text())
            self.assertEqual(validate(m),list(range(1,len(m["services"])+1)))
            self.assertIn(f"#define ZARC_PROFILE_ENDPOINT_COUNT {count}u",generate(m))
            bad=copy.deepcopy(m);bad["profile_runtime"]["endpoint_ids"]=[1,1]
            with self.assertRaises(ValueError):validate(bad)
            bad=copy.deepcopy(m);bad["services"].pop()
            with self.assertRaises(ValueError):validate(bad)
    def test_timing(self):
        assumptions=dict(uniprocessor=True,fully_preemptive=True,no_self_suspension=True)
        tasks=copy.deepcopy(self.m["tasks"])
        # Synthetic analytical allocation, not the actual self-suspending native fixture.
        tasks[0]["blocking_us"]=0
        tasks[1]["deadline_us"]=10000
        result=analyze(tasks,**assumptions)
        self.assertTrue(all(x["admitted"] for x in result.values()))
        tasks[0]["execution_us"]=9800
        self.assertFalse(analyze(tasks,**assumptions)["virtual_device"]["admitted"])
        with self.assertRaises(ValueError):analyze(tasks,**{**assumptions,"uniprocessor":False})

if __name__=="__main__":unittest.main()
