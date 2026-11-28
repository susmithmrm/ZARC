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
from target_manifest import (validate,generate,reconcile_kconfig,
                             hardware_lifecycle_delta,
                             validate_profile_budget_declarations)
from timing_analysis import analyze

class Tests(unittest.TestCase):
    def setUp(self):
        self.m=json.loads((ROOT/"config/targets/native-sim-edge.json").read_text())

    @staticmethod
    def lifecycle():
        return {"schema":1,"board_revision":"board-r1","silicon_stepping":"step-a0",
                "errata":[{"id":"errata-1","reference":"synthetic:errata-1"}],
                "vendor_safety_docs":[{"id":"safety-manual","revision":"rev-1",
                                        "reference":"synthetic:safety-manual"}],
                "components":[{"id":"power-switch","revision":"rev-1",
                                "substitutions":["switch-alt-a"]}],
                "review_status":"REVIEWED","review_reference":"synthetic:review-1"}

    @staticmethod
    def profile_budgets():
        return {"schema":1,"profile":"edge","categories":{
            "resource":{"status":"DECLARED","limits":{"ram":{"maximum":64,"unit":"KiB"}},
                        "verification_method":"compare measured map to declared limit"},
            "boot":{"status":"DECLARED","limits":{"startup":{"maximum":500,"unit":"ms"}},
                    "verification_method":"repeat cold-start measurement"},
            "timing":{"status":"DECLARED","limits":{"control_deadline":{"maximum":10,"unit":"ms"}},
                       "verification_method":"compare trace against allocated deadline"},
            "power":{"status":"DECLARED","limits":{"average":{"maximum":1.5,"unit":"W"}},
                     "verification_method":"measure current at declared operating mode"}}}

    def test_profile_budget_declarations_require_all_categories_and_do_not_approve(self):
        record=self.profile_budgets()
        record["categories"]["resource"]["limits"]["accelerator"]={
            "maximum":0,"unit":"allocations"}
        result=validate_profile_budget_declarations(record,"edge")
        self.assertEqual(result["categories"],["boot","power","resource","timing"])
        self.assertEqual(result["deviations"],[])
        self.assertEqual(result["budget_approval"],"NOT_ASSERTED")
        self.assertEqual(result["measurements"],"NOT_ASSERTED")
        self.assertFalse(result["supported_admission"])
        for category in ("resource","boot","timing","power"):
            incomplete=copy.deepcopy(record)
            del incomplete["categories"][category]
            with self.subTest(category=category),self.assertRaisesRegex(ValueError,"categories required"):
                validate_profile_budget_declarations(incomplete)

    def test_profile_budget_declarations_reject_malformed_values(self):
        cases=[]
        def mutate(label,change):
            record=self.profile_budgets();change(record);cases.append((label,record))
        mutate("schema bool",lambda r:r.update(schema=True))
        mutate("unknown profile",lambda r:r.update(profile="invalid"))
        mutate("missing metric",lambda r:r["categories"]["boot"].update(limits={}))
        mutate("boolean limit",lambda r:r["categories"]["resource"]["limits"]["ram"].update(maximum=True))
        mutate("negative limit",lambda r:r["categories"]["resource"]["limits"]["ram"].update(maximum=-1))
        mutate("nonfinite limit",lambda r:r["categories"]["power"]["limits"]["average"].update(maximum=float("nan")))
        mutate("infinite limit",lambda r:r["categories"]["power"]["limits"]["average"].update(maximum=float("inf")))
        mutate("unit missing",lambda r:r["categories"]["timing"]["limits"]["control_deadline"].update(unit=" "))
        mutate("verification missing",lambda r:r["categories"]["boot"].update(verification_method=""))
        mutate("unknown category",lambda r:r["categories"].update(memory=r["categories"].pop("resource")))
        for label,record in cases:
            with self.subTest(label=label),self.assertRaises(ValueError):
                validate_profile_budget_declarations(record)
        with self.assertRaisesRegex(ValueError,"profile mismatch"):
            validate_profile_budget_declarations(self.profile_budgets(),"integrated")
        enormous=self.profile_budgets()
        enormous["categories"]["resource"]["limits"]["ram"]["maximum"]=10**10000
        self.assertFalse(validate_profile_budget_declarations(enormous)["supported_admission"])

    def test_profile_budget_deviation_is_explicit_and_stays_unapproved(self):
        record=self.profile_budgets()
        record["categories"]["power"].update(status="DEVIATION",deviation={
            "rationale":"caller supplied deviation fixture","reference":"fixture:deviation-1"})
        result=validate_profile_budget_declarations(record)
        self.assertEqual(result["deviations"],["power"])
        self.assertEqual(result["budget_approval"],"NOT_ASSERTED")
        self.assertFalse(result["supported_admission"])
        missing=copy.deepcopy(record)
        missing["categories"]["power"]["deviation"].pop("reference")
        with self.assertRaisesRegex(ValueError,"explicit power budget deviation"):
            validate_profile_budget_declarations(missing)
        wrong_status=copy.deepcopy(record)
        wrong_status["categories"]["power"]["status"]="DECLARED"
        with self.assertRaisesRegex(ValueError,"deviation details require"):
            validate_profile_budget_declarations(wrong_status)
        candidate=copy.deepcopy(self.m)
        candidate["profile_budgets"]=record
        self.assertEqual(validate(candidate),[1,2])
        candidate["profile_budgets"]["categories"].pop("power")
        with self.assertRaises(ValueError):validate(candidate)

    def test_hardware_lifecycle_unchanged_is_not_hardware_acceptance(self):
        record=self.lifecycle()
        report=hardware_lifecycle_delta(record,copy.deepcopy(record))
        self.assertEqual(report["verdict"],"NO_CHANGE_RECORDED")
        self.assertEqual(report["changes"],[])
        self.assertEqual(report["hardware_acceptance"],"NOT_ASSERTED")

    def test_empty_hardware_inventory_is_a_valid_boundary(self):
        record=self.lifecycle()
        record["errata"]=[]
        record["vendor_safety_docs"]=[]
        record["components"]=[]
        self.assertEqual(hardware_lifecycle_delta(record,copy.deepcopy(record))["verdict"],
                         "NO_CHANGE_RECORDED")

    def test_hardware_lifecycle_changes_require_review(self):
        cases=[]
        def changed(path,mutate):
            old=self.lifecycle();new=copy.deepcopy(old);mutate(new)
            cases.append((path,old,new))
        changed("board_revision",lambda m:m.update(board_revision="board-r2"))
        changed("silicon_stepping",lambda m:m.update(silicon_stepping="step-a1"))
        changed("errata",lambda m:m["errata"].append({"id":"errata-2","reference":"synthetic:errata-2"}))
        changed("errata",lambda m:m["errata"][0].update(reference="synthetic:errata-1-r2"))
        changed("errata",lambda m:m.update(errata=[]))
        changed("vendor_safety_docs",lambda m:m["vendor_safety_docs"][0].update(revision="rev-2"))
        changed("vendor_safety_docs",lambda m:m["vendor_safety_docs"][0].update(reference="synthetic:safety-r2"))
        changed("components",lambda m:m["components"][0].update(revision="rev-2"))
        changed("components",lambda m:m["components"][0].update(substitutions=["switch-alt-b"]))
        changed("components",lambda m:m["components"].append({"id":"connector","revision":"rev-1","substitutions":[]}))
        changed("components",lambda m:m.update(components=[]))
        for field,old,new in cases:
            with self.subTest(field=field,new=new):
                report=hardware_lifecycle_delta(old,new)
                self.assertEqual(report["verdict"],"REVIEW_REQUIRED")
                self.assertTrue(any(change["field"]==field for change in report["changes"]))
                self.assertEqual(report["hardware_acceptance"],"NOT_ASSERTED")

    def test_missing_or_unreviewed_hardware_lifecycle_requires_review(self):
        record=self.lifecycle()
        self.assertEqual(hardware_lifecycle_delta(None,record)["verdict"],"REVIEW_REQUIRED")
        self.assertEqual(hardware_lifecycle_delta(record,None)["verdict"],"REVIEW_REQUIRED")
        unreviewed=copy.deepcopy(record);unreviewed["review_status"]="UNREVIEWED";unreviewed["review_reference"]=""
        report=hardware_lifecycle_delta(record,unreviewed)
        self.assertEqual(report["verdict"],"REVIEW_REQUIRED")
        self.assertTrue(any(change["change"]=="UNREVIEWED" for change in report["changes"]))

    def test_malformed_or_duplicate_hardware_records_are_rejected(self):
        record=self.lifecycle()
        cases=[]
        duplicate=copy.deepcopy(record);duplicate["errata"].append(copy.deepcopy(duplicate["errata"][0]));cases.append(duplicate)
        duplicate=copy.deepcopy(record);duplicate["vendor_safety_docs"].append(copy.deepcopy(duplicate["vendor_safety_docs"][0]));cases.append(duplicate)
        duplicate=copy.deepcopy(record);duplicate["components"].append(copy.deepcopy(duplicate["components"][0]));cases.append(duplicate)
        missing=copy.deepcopy(record);del missing["silicon_stepping"];cases.append(missing)
        malformed=copy.deepcopy(record);malformed["components"][0]["substitutions"]="switch-alt-a";cases.append(malformed)
        unreferenced=copy.deepcopy(record);unreferenced["review_reference"]="";cases.append(unreferenced)
        for invalid in cases:
            with self.subTest(invalid=invalid),self.assertRaises(ValueError):
                hardware_lifecycle_delta(record,invalid)

    def test_hardware_lifecycle_cli_requires_review_and_never_claims_acceptance(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);before=root/"before.json";after=root/"after.json"
            old={"hardware_lifecycle":self.lifecycle()}
            new=copy.deepcopy(old);new["hardware_lifecycle"]["board_revision"]="board-r2"
            before.write_text(json.dumps(old),encoding="utf-8")
            after.write_text(json.dumps(new),encoding="utf-8")
            command=[sys.executable,str(ROOT/"tools/target_manifest.py"),
                     "--compare-lifecycle",str(before),str(after)]
            result=subprocess.run(command,capture_output=True,text=True,check=False)
            self.assertEqual(result.returncode,2,result.stderr)
            report=json.loads(result.stdout)
            after.write_text(json.dumps(old),encoding="utf-8")
            unchanged=subprocess.run(command,capture_output=True,text=True,check=False)
            self.assertEqual(unchanged.returncode,0,unchanged.stderr)
            stable=json.loads(unchanged.stdout)
        self.assertEqual(stable["verdict"],"NO_CHANGE_RECORDED")
        self.assertEqual(stable["hardware_acceptance"],"NOT_ASSERTED")
        self.assertEqual(report["verdict"],"REVIEW_REQUIRED")
        self.assertEqual(report["hardware_acceptance"],"NOT_ASSERTED")

    def test_hardware_lifecycle_cli_unchanged_record_is_not_approval(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);before=root/"before.json";after=root/"after.json"
            record={"hardware_lifecycle":self.lifecycle()}
            before.write_text(json.dumps(record),encoding="utf-8")
            after.write_text(json.dumps(copy.deepcopy(record)),encoding="utf-8")
            result=subprocess.run([sys.executable,str(ROOT/"tools/target_manifest.py"),
                                   "--compare-lifecycle",str(before),str(after)],
                                  capture_output=True,text=True,check=False)
        self.assertEqual(result.returncode,0,result.stderr)
        report=json.loads(result.stdout)
        self.assertEqual(report["verdict"],"NO_CHANGE_RECORDED")
        self.assertEqual(report["hardware_acceptance"],"NOT_ASSERTED")
    def test_candidate(self):
        self.assertEqual(validate(self.m),[1,2])
        self.assertEqual(generate(self.m),generate(copy.deepcopy(self.m)))
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/".config";p.write_text("# CONFIG_CAN is not set\nCONFIG_ZBUS=y\nCONFIG_PM_DEVICE_RUNTIME=y\nCONFIG_NVS=y\n")
            reconcile_kconfig(self.m,p)
            p.write_text("CONFIG_CAN=y\nCONFIG_ZBUS=y\nCONFIG_PM_DEVICE_RUNTIME=y\nCONFIG_NVS=y\n")
            with self.assertRaises(ValueError):reconcile_kconfig(self.m,p)

    def test_optional_hardware_lifecycle_is_validated_at_manifest_admission(self):
        m=copy.deepcopy(self.m);m["hardware_lifecycle"]=self.lifecycle()
        self.assertEqual(validate(m),[1,2])
        malformed=copy.deepcopy(m)
        malformed["hardware_lifecycle"]["components"].append(
            copy.deepcopy(malformed["hardware_lifecycle"]["components"][0]))
        with self.assertRaises(ValueError):validate(malformed)

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
        unknown=copy.deepcopy(m)
        unknown["memory_regions"][1]["cachable"]=False
        with self.assertRaisesRegex(ValueError,"unknown memory region field"):
            validate(unknown)
        shared=copy.deepcopy(m)
        shared["memory_regions"].append(region("shared","shared",8192))
        shared["memory_regions"][2]["coherency"]="synthetic coherency fixture"
        self.assertEqual(validate(shared),[1,2])
        shared["memory_regions"][2]["initialization"]="unsupported extra field"
        with self.assertRaisesRegex(ValueError,"unknown memory region field"):
            validate(shared)
        external=copy.deepcopy(m)
        external["memory_regions"].append(region("external","external_ram",8192,write=True))
        external["memory_regions"][2]["initialization"]="synthetic init fixture"
        self.assertEqual(validate(external),[1,2])
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
