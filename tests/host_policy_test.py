import json
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "host"))
from zarc_host.security import Principal, Policy
from zarc_host.broker import Gateway, parse_command

class Backend:
    def __init__(self):
        self.calls = []
        self.fail = False
        self.invalidated = False
    def snapshot(self, cap):
        self.calls.append(("snapshot", cap))
        return {"status": "COMPLETED", "value": 1}
    def request(self, cap, desired, validity):
        self.calls.append(("request", cap, desired, validity))
        if self.fail:
            raise OSError("fixture loss")
        return {"status": "ACCEPTED"}
    def invalidate(self):
        self.invalidated = True

class Tests(unittest.TestCase):
    def test_policy(self):
        now = [1.0]
        observer = Principal("observer", frozenset({1}), frozenset(), 2)
        operator = Principal("operator", frozenset({1}), frozenset({1}), 1)
        policy = Policy({"1"*64: observer, "2"*64: operator}, lambda: now[0])
        backend = Backend()
        gateway = Gateway(policy, backend)
        write = b'{"operation":"request","capability":1,"desired":2,"validity":100}'
        read = b'{"operation":"snapshot","capability":1}'
        self.assertEqual(gateway.dispatch(observer, write)["status"], "DENIED")
        self.assertFalse(backend.calls)
        self.assertEqual(gateway.dispatch(observer, read)["status"], "COMPLETED")
        self.assertEqual(gateway.dispatch(observer, read)["status"], "COMPLETED")
        self.assertEqual(gateway.dispatch(observer, read)["status"], "DENIED")
        self.assertEqual(gateway.dispatch(operator, write)["status"], "ACCEPTED")
        self.assertEqual(gateway.dispatch(operator, write)["status"], "DENIED")
        now[0] += 1
        backend.fail = True
        self.assertEqual(gateway.dispatch(operator, write)["status"], "UNKNOWN_OUTCOME")
        self.assertTrue(backend.invalidated)
        self.assertRaises(PermissionError, policy.identify, object())
    def test_strict_schema(self):
        for raw in [b'{}', b'[]', b'{"operation":"snapshot","capability":true}',
                    b'{"operation":"snapshot","capability":1,"capability":2}',
                    b'{"operation":"snapshot","capability":1,"principal":"operator"}', b"x"*513]:
            with self.assertRaises(ValueError):
                parse_command(raw)

if __name__ == "__main__":
    unittest.main()
