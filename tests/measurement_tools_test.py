import csv
import json
import subprocess
import sys
import tempfile
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from timing_evidence import evaluate_cycles,evaluate_transport,evaluate_profile
from change_impact import impact
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
    def test_profile_cli_records_raw_trace_and_configuration(self):
        root=Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as d:
            work=Path(d);plan=work/"plan.json";trace=work/"profile.csv"
            plan.write_text(json.dumps(dict(configuration="edge-lab",board="native_sim/native/64",
                profile="native-sim-edge",firmware_sha256="a"*64,toolchain="gcc-15",
                clock="monotonic",instrument="simulator",procedure="scenario-1")),encoding="utf-8")
            with trace.open("w",newline="",encoding="utf-8") as f:
                writer=csv.DictWriter(f,fieldnames=["profile","boot_ms","latency_us","jitter_us",
                    "ram_bytes","flash_bytes","cpu_permille","power_mw"])
                writer.writeheader();writer.writerow(dict(profile="native-sim-edge",boot_ms=12,
                    latency_us=100,jitter_us=4,ram_bytes=4096,flash_bytes=16384,
                    cpu_permille=300,power_mw=0))
            result=subprocess.run([sys.executable,str(root/"tools/timing_evidence.py"),"profile",
                "--plan",str(plan),"--trace",str(trace)],capture_output=True,text=True,check=False)
        self.assertEqual(result.returncode,0,result.stderr)
        report=json.loads(result.stdout)
        self.assertEqual(report["verdict"],"RECORDED")
        self.assertEqual(report["profile"],"native-sim-edge")
        self.assertEqual(report["metrics"]["power_mw"]["unit"],"mW")
        self.assertEqual(len(report["trace_sha256"]),64)
    def test_change(self):
        rows=[dict(Requirement="A",Candidate_code="src/a.c",Candidate_tests="tests/a.c"),
              dict(Requirement="B",Candidate_code="src/b.c",Candidate_tests="tests/b.c")]
        self.assertEqual(impact(rows,["src/a.c"])["direct"],["A"])
        self.assertEqual(impact(rows,["new.c"])["review_scope"],["A","B"])
        self.assertEqual(impact(rows,[])["compatibility"],"REVIEW_REQUIRED")
if __name__=="__main__":unittest.main()
