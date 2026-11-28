import csv
import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from timing_evidence import evaluate_cycles,evaluate_transport,evaluate_profile,validate_profile_plan
from change_impact import impact,dependency_changes,validate_dependency_reviews
class Tests(unittest.TestCase):
    def test_cycles(self):
        rows=[dict(release_us=i*10,start_us=i*10,completion_us=i*10+3) for i in range(4)]
        self.assertEqual(evaluate_cycles(rows,10,8,40)["verdict"],"PASS")
        self.assertEqual(evaluate_cycles(rows,10,8,50)["verdict"],"FAIL")
        self.assertEqual(evaluate_cycles(rows[:1]+rows[2:],10,8,40)["gaps"],1)
        rows[1]["completion_us"]=18
        self.assertEqual(evaluate_cycles(rows,10,8,40)["verdict"],"FAIL")
    def test_transport(self):
        rows=[dict(sent_us=i*1000,received_us=i*1000+100,tx_wire_bytes=10,rx_wire_bytes=10) for i in range(4)]
        result=evaluate_transport(rows,200,10000,1000000,10,100,4)
        self.assertEqual(result["verdict"],"PASS")
        self.assertEqual(result["max_tx_permille"],40)
        self.assertEqual(evaluate_transport(rows,100,10000,1000000,10,100,4)["verdict"],"FAIL")
        self.assertEqual(evaluate_transport(rows,200,10000,1000000,10,30,4)["verdict"],"FAIL")
    def test_profile_measurements_have_no_universal_pass_threshold(self):
        row=dict(profile="native-sim-edge",boot_ms=12.5,latency_us=140,jitter_us=8,
                 ram_bytes=4096,flash_bytes=16384,cpu_permille=725,power_mw=0)
        report=evaluate_profile([row],"native-sim-edge")
        self.assertEqual(report["verdict"],"RECORDED")
        self.assertEqual(report["metrics"]["boot_ms"]["maximum"],12.5)
        self.assertIn("no universal threshold",report["scope"])
        self.assertEqual(evaluate_profile([dict(row,boot_ms=1e30)],"native-sim-edge")["verdict"],"RECORDED")
    def test_profile_measurements_reject_invalid_or_mixed_samples(self):
        row=dict(profile="native-sim-edge",boot_ms=12,latency_us=140,jitter_us=8,
                 ram_bytes=4096,flash_bytes=16384,cpu_permille=725,power_mw=1)
        with self.assertRaises(ValueError):evaluate_profile([dict(row,power_mw=-1)],"native-sim-edge")
        with self.assertRaises(ValueError):evaluate_profile([dict(row,ram_bytes=1.5)],"native-sim-edge")
        with self.assertRaises(ValueError):evaluate_profile([dict(row,profile="other")],"native-sim-edge")
        with self.assertRaises(ValueError):evaluate_profile([],"native-sim-edge")
    @staticmethod
    def profile_plan(**overrides):
        plan=dict(profile="synthetic-profile",firmware_sha256="a"*64,toolchain="synthetic-toolchain",
            hardware_assumptions=["synthetic hardware fixture"],
            staffing_assumptions=["synthetic staffing fixture"],
            limitations=["synthetic measurements only"])
        plan.update(overrides)
        return plan
    def test_profile_plan_requires_caller_supplied_metadata(self):
        self.assertIsNone(validate_profile_plan(self.profile_plan()))
        for key in ("hardware_assumptions","staffing_assumptions","limitations"):
            for invalid in (None,[],[" "]):
                with self.subTest(key=key,invalid=invalid):
                    with self.assertRaisesRegex(ValueError,key):
                        validate_profile_plan(self.profile_plan(**{key:invalid}))
            with self.subTest(key=key,invalid="not a list"):
                with self.assertRaisesRegex(ValueError,key):
                    validate_profile_plan(self.profile_plan(**{key:"not a list"}))
        for missing in ("profile","toolchain","firmware_sha256"):
            plan=self.profile_plan();del plan[missing]
            with self.subTest(missing=missing),self.assertRaises(ValueError):
                validate_profile_plan(plan)
    def test_profile_plan_single_entry_metadata_is_valid_boundary(self):
        plan=self.profile_plan(hardware_assumptions=["synthetic"],
            staffing_assumptions=["synthetic"],limitations=["synthetic"])
        self.assertIsNone(validate_profile_plan(plan))
    def test_profile_cli_records_raw_trace_and_configuration(self):
        root=Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as d:
            work=Path(d);plan=work/"plan.json";trace=work/"profile.csv"
            plan.write_text(json.dumps(dict(configuration="edge-lab",board="native_sim/native/64",
                profile="native-sim-edge",firmware_sha256="a"*64,toolchain="gcc-15",
                clock="monotonic",instrument="simulator",procedure="scenario-1",
                hardware_assumptions=["synthetic simulator fixture"],
                staffing_assumptions=["synthetic test staffing"],
                limitations=["synthetic trace; not a physical benchmark"])),encoding="utf-8")
            with trace.open("w",newline="",encoding="utf-8") as f:
                writer=csv.DictWriter(f,fieldnames=["profile","boot_ms","latency_us","jitter_us",
                    "ram_bytes","flash_bytes","cpu_permille","power_mw"])
                writer.writeheader();writer.writerow(dict(profile="native-sim-edge",boot_ms=12,
                    latency_us=100,jitter_us=4,ram_bytes=4096,flash_bytes=16384,
                    cpu_permille=300,power_mw=0))
            plan_bytes=plan.read_bytes();trace_bytes=trace.read_bytes()
            result=subprocess.run([sys.executable,str(root/"tools/timing_evidence.py"),"profile",
                "--plan",str(plan),"--trace",str(trace)],capture_output=True,text=True,check=False)
        self.assertEqual(result.returncode,0,result.stderr)
        report=json.loads(result.stdout)
        self.assertEqual(report["verdict"],"RECORDED")
        self.assertEqual(report["profile"],"native-sim-edge")
        self.assertEqual(report["metrics"]["power_mw"]["unit"],"mW")
        self.assertEqual(len(report["trace_sha256"]),64)
        self.assertEqual(report["plan_sha256"],hashlib.sha256(plan_bytes).hexdigest())
        self.assertEqual(report["trace_sha256"],hashlib.sha256(trace_bytes).hexdigest())
        self.assertEqual(report["hardware_assumptions"],["synthetic simulator fixture"])
        self.assertEqual(report["staffing_assumptions"],["synthetic test staffing"])
        self.assertEqual(report["limitations"],["synthetic trace; not a physical benchmark"])
        self.assertFalse(report["approval_asserted"])
        self.assertIn("approval claim",report["scope"])
    def test_change(self):
        rows=[dict(Requirement="A",Candidate_code="src/a.c",Candidate_tests="tests/a.c"),
              dict(Requirement="B",Candidate_code="src/b.c",Candidate_tests="tests/b.c")]
        self.assertEqual(impact(rows,["src/a.c"])["direct"],["A"])
        self.assertEqual(impact(rows,["new.c"])["review_scope"],["A","B"])
        self.assertEqual(impact(rows,[])["compatibility"],"REVIEW_REQUIRED")

    @staticmethod
    def dep(name,version="1",source="https://example.invalid/repo",revision="a"*40,license="MIT"):
        return dict(name=name,version=version,source=source,revision=revision,license=license)

    @staticmethod
    def review(change,**overrides):
        record={k:change[k] for k in ("name","change","before","after")}
        record.update(compatibility="ACCEPTED",support="ACCEPTED",license="ACCEPTED",
                      reviewer="reviewer",evidence_reference="evidence/123",decision="ACCEPTED")
        record.update(overrides)
        return record

    def test_dependency_delta_detects_add_remove_version_and_license_changes(self):
        before=[self.dep("remove"),self.dep("version"),self.dep("license")]
        after=[self.dep("add"),self.dep("version","2"),self.dep("license",license="BSD-3-Clause")]
        changes=dependency_changes(before,after)
        self.assertEqual({(c["name"],c["change"]) for c in changes},
                         {("add","ADDED"),("remove","REMOVED"),
                          ("version","UPDATED"),("license","UPDATED")})
        self.assertEqual(dependency_changes(before,before),[])
        source_delta=dependency_changes([self.dep("source")],
            [self.dep("source",source="https://example.invalid/other")])
        revision_delta=dependency_changes([self.dep("revision")],
            [self.dep("revision",revision="b"*40)])
        self.assertEqual(source_delta[0]["change"],"UPDATED")
        self.assertEqual(revision_delta[0]["change"],"UPDATED")

    def test_dependency_inventory_rejects_duplicate_and_incomplete_records(self):
        with self.assertRaisesRegex(ValueError,"duplicate"):
            dependency_changes([self.dep("dup"),self.dep(" dup ")],[])
        with self.assertRaisesRegex(ValueError,"missing license"):
            dependency_changes([], [{k:v for k,v in self.dep("bad").items() if k!="license"}])
        with self.assertRaisesRegex(ValueError,"missing revision"):
            dependency_changes([self.dep("bad",revision="")],[])

    def test_dependency_reviews_fail_closed_for_unapproved_inventory(self):
        changes=dependency_changes([], [self.dep("new")])
        errors,report=validate_dependency_reviews(changes,[self.review(changes[0])],"CANDIDATE")
        self.assertTrue(any("approved dependency inventory" in e for e in errors))
        self.assertEqual(report["gate_status"],"BLOCKED")
        self.assertFalse(report["approval_asserted"])

    def test_dependency_reviews_require_one_matching_complete_acceptance(self):
        changes=dependency_changes([self.dep("old")],[self.dep("old","2")])
        complete=self.review(changes[0])
        errors,report=validate_dependency_reviews(changes,[complete],"APPROVED")
        self.assertEqual(errors,[])
        self.assertEqual(report["gate_status"],"REVIEW_RECORDS_COMPLETE")
        self.assertFalse(report["approval_asserted"])
        self.assertFalse(report["release_authorized"])
        for reviews in ([],[complete,complete],
                        [dict(complete,compatibility="REJECTED")],
                        [dict(complete,support="REJECTED")],
                        [dict(complete,license="REJECTED")],
                        [dict(complete,before=self.dep("old","0"))],
                        [dict(complete,evidence_reference="")]):
            errors,report=validate_dependency_reviews(changes,reviews,"APPROVED")
            self.assertTrue(errors)
            self.assertEqual(report["gate_status"],"BLOCKED")

    def test_no_dependency_change_needs_no_review_record(self):
        errors,report=validate_dependency_reviews([],[],"INCOMPLETE")
        self.assertEqual(errors,[])
        self.assertEqual(report["gate_status"],"NO_DEPENDENCY_CHANGE")
        errors,_=validate_dependency_reviews([], [self.review({"name":"orphan","change":"ADDED",
            "before":None,"after":self.dep("orphan")})],"APPROVED")
        self.assertTrue(errors)

    def test_dependency_change_cli_blocks_candidate_inventory(self):
        root=Path(__file__).resolve().parents[1]
        change={"name":"new","change":"ADDED","before":None,"after":self.dep("new")}
        with tempfile.TemporaryDirectory() as d:
            work=Path(d)
            before=work/"before.json";after=work/"after.json";reviews=work/"reviews.json"
            before.write_text("[]",encoding="utf-8")
            after.write_text(json.dumps([self.dep("new")]),encoding="utf-8")
            reviews.write_text(json.dumps([self.review(change)]),encoding="utf-8")
            result=subprocess.run([sys.executable,str(root/"tools/change_impact.py"),
                "--base","HEAD","--dependency-before",str(before),"--dependency-after",str(after),
                "--dependency-reviews",str(reviews),"--dependency-inventory-status","CANDIDATE"],
                capture_output=True,text=True,check=False)
            accepted=subprocess.run([sys.executable,str(root/"tools/change_impact.py"),
                "--base","HEAD","--dependency-before",str(before),"--dependency-after",str(after),
                "--dependency-reviews",str(reviews),"--dependency-inventory-status","APPROVED"],
                capture_output=True,text=True,check=False)
        self.assertEqual(result.returncode,1)
        report=json.loads(result.stdout)
        self.assertEqual(report["dependency_review"]["gate_status"],"BLOCKED")
        self.assertFalse(report["dependency_review"]["approval_asserted"])
        self.assertEqual(accepted.returncode,0,accepted.stderr)
        approved_report=json.loads(accepted.stdout)
        self.assertEqual(approved_report["dependency_review"]["gate_status"],"REVIEW_RECORDS_COMPLETE")
        self.assertFalse(approved_report["dependency_review"]["approval_asserted"])
        self.assertFalse(approved_report["dependency_review"]["release_authorized"])
if __name__=="__main__":unittest.main()
