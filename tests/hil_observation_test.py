"""Synthetic test of the independent-trace evaluator, not HIL evidence."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from hil_observations import evaluate

class Tests(unittest.TestCase):
    def test_oracle(self):
        plan=dict(board="synthetic",firmware_sha256="a"*64,configuration="synthetic",
                  instrument="fixture",clock="fixture",procedure="fixture",
                  max_feedback_ms=10,expected_value=2)
        a={"event":"admitted","time_ms":0}
        d={"event":"link_disconnected","time_ms":1}
        f={"event":"feedback","time_ms":9,"value":2,"quality":"VALID","source":"independent_instrument"}
        self.assertEqual(evaluate(plan,[a,d,f])["verdict"],"PASS")
        self.assertEqual(evaluate(plan,[a,d,{**f,"time_ms":10}])["verdict"],"FAIL")
        self.assertEqual(evaluate(plan,[a,d,{**f,"source":"controller"}])["verdict"],"FAIL")
        self.assertEqual(evaluate(plan,[a,d])["verdict"],"FAIL")
        with self.assertRaises(ValueError):evaluate(plan,[d,a,f])

if __name__=="__main__":
    unittest.main()
