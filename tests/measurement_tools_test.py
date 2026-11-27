import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from timing_evidence import evaluate_cycles,evaluate_transport
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
    def test_change(self):
        rows=[dict(Requirement="A",Candidate_code="src/a.c",Candidate_tests="tests/a.c"),
              dict(Requirement="B",Candidate_code="src/b.c",Candidate_tests="tests/b.c")]
        self.assertEqual(impact(rows,["src/a.c"])["direct"],["A"])
        self.assertEqual(impact(rows,["new.c"])["review_scope"],["A","B"])
        self.assertEqual(impact(rows,[])["compatibility"],"REVIEW_REQUIRED")
if __name__=="__main__":unittest.main()
