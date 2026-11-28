import copy
import hashlib
import json
from pathlib import Path
import subprocess
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
            p=Path(d)/".config";p.write_text("# CONFIG_CAN is not set\nCONFIG_ZBUS=y\nCONFIG_PM_DEVICE_RUNTIME=y\nCONFIG_NVS=y\n")
            reconcile_kconfig(self.m,p)
            p.write_text("CONFIG_CAN=y\nCONFIG_ZBUS=y\nCONFIG_PM_DEVICE_RUNTIME=y\nCONFIG_NVS=y\n")
            with self.assertRaises(ValueError):reconcile_kconfig(self.m,p)

    def test_generated_header_is_byte_reproducible_from_identical_manifest(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d)
            manifest=root/"candidate.json"
            manifest.write_text(json.dumps(self.m,indent=2)+"\n",encoding="utf-8")
            outputs=[root/"one"/"zarc_manifest.h",root/"two"/"zarc_manifest.h"]
            for output in outputs:
                result=subprocess.run([sys.executable,str(ROOT/"tools/target_manifest.py"),
                                       str(manifest),"--header",str(output)],
                                      capture_output=True,text=True,check=False)
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertIn("no target support claim",result.stdout)
            first,second=(output.read_bytes() for output in outputs)
            self.assertEqual(first,second)
            self.assertEqual(hashlib.sha256(first).digest(),hashlib.sha256(second).digest())

    def test_invalid_manifest_cli_reports_actionable_error_and_writes_no_header(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d)
            invalid=copy.deepcopy(self.m)
            invalid["profile"]="unknown-profile"
            manifest=root/"invalid.json"
            manifest.write_text(json.dumps(invalid),encoding="utf-8")
            header=root/"generated"/"zarc_manifest.h"
            result=subprocess.run([sys.executable,str(ROOT/"tools/target_manifest.py"),
                                   str(manifest),"--header",str(header)],
                                  capture_output=True,text=True,check=False)
            self.assertEqual(result.returncode,1)
            self.assertIn("Rejected: unknown profile",result.stdout)
            self.assertFalse(header.exists())
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
    def test_execution_descriptor(self):
        for key,value in (("isa",""),("core_count",0),("mode","lockstep"),
                          ("byte_order","unknown"),("word_bits",True),
                          ("alignment_bytes",3),("port_owner","")):
            bad=copy.deepcopy(self.m);bad["execution"][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):validate(bad)
        bad=copy.deepcopy(self.m);del bad["execution"]
        with self.assertRaises(ValueError):validate(bad)
        bad=copy.deepcopy(self.m);bad["features"]["smp"]=True
        with self.assertRaises(ValueError):validate(bad)
    def test_physical_memory_descriptor(self):
        # Synthetic address allocation, not a supported board or real linker map.
        m=copy.deepcopy(self.m)
        del m["benign_runtime"]
        m["board"]="candidate_board";m["soc"]="candidate_soc"
        m["execution"].update(isa="arm",core="candidate",abi="candidate ABI",
                              mode="single",byte_order="little",word_bits=32)
        def region(name,kind,base,**kw):
            return dict(name=name,kind=kind,base=base,size=4096,alignment=4096,
                        read=True,write=kw.get("write",False),
                        execute=kw.get("execute",False),cacheable=False,
                        dma=False,owner="core")
        m["memory_regions"]=[region("code","code",0,execute=True),
                             region("ram","ram",4096,write=True)]
        self.assertEqual(validate(m),[1,2])
        for case in ("missing","overlap","wrap","alignment","shared","external","permissions"):
            bad=copy.deepcopy(m)
            r=bad["memory_regions"][1]
            if case=="missing":del bad["memory_regions"]
            elif case=="overlap":r["base"]=0
            elif case=="wrap":r["base"]=(1<<32)-2048
            elif case=="alignment":r["alignment"]=3
            elif case=="shared":r["kind"]="shared"
            elif case=="external":r["kind"]="external_ram"
            else:del r["dma"]
            with self.subTest(case=case),self.assertRaises(ValueError):validate(bad)
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
