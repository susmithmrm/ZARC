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
        self.boot = 10
        self.session = 11
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
    def test_retained_request_ownership(self):
        a=Principal("a",frozenset({1}),frozenset({1}),100)
        b=Principal("b",frozenset({1}),frozenset({1}),100)
        class Retained(Backend):
            boot=10
            session=11
            def request(self,cap,desired,validity):
                return dict(status="ACCEPTED",boot=10,session=11,request_id=12)
            def request_status(self,rid,*,boot,session):
                self.calls.append((rid,boot,session))
                return dict(status="COMPLETED")
        backend=Retained()
        gateway=Gateway(Policy({"1"*64:a,"2"*64:b}),backend)
        write=b'{"operation":"request","capability":1,"desired":2,"validity":100}'
        query=b'{"operation":"request_status","capability":1,"boot":10,"session":11,"request_id":12}'
        self.assertEqual(gateway.dispatch(a,b'{"operation":"snapshot","capability":1}')["status"],"COMPLETED")
        backend.calls.clear()
        self.assertEqual(gateway.dispatch(a,write)["status"],"ACCEPTED")
        self.assertEqual(gateway.dispatch(b,query)["status"],"UNKNOWN_OUTCOME")
        self.assertFalse(backend.calls)
        self.assertEqual(gateway.dispatch(a,query)["status"],"COMPLETED")
        self.assertEqual(backend.calls,[(12,10,11)])
        self.assertEqual(gateway.dispatch(a,query.replace(b'"session":11',b'"session":12'))["status"],"UNKNOWN_OUTCOME")
    def test_restart_requires_fresh_snapshot_and_drops_old_owners(self):
        a=Principal("a",frozenset({1}),frozenset({1}),100)
        class EpochBackend(Backend):
            def __init__(self):
                super().__init__();self.boot=10;self.session=11
            def snapshot(self,cap):
                self.calls.append(("snapshot",cap))
                return {"status":"COMPLETED","value":1}
            def request(self,cap,desired,validity):
                self.calls.append(("request",cap,desired,validity))
                return {"status":"ACCEPTED","boot":self.boot,
                        "session":self.session,"request_id":12}
            def request_status(self,rid,*,boot,session):
                self.calls.append(("request_status",rid,boot,session))
                return {"status":"COMPLETED"}
        backend=EpochBackend()
        policy=Policy({"1"*64:a})
        old=Gateway(policy,backend)
        write=b'{"operation":"request","capability":1,"desired":2,"validity":100}'
        status=b'{"operation":"request_status","capability":1,"boot":10,"session":11,"request_id":12}'
        self.assertEqual(old.dispatch(a,write)["status"],"UNAVAILABLE")
        self.assertFalse(any(call[0]=="request" for call in backend.calls))
        self.assertEqual(old.dispatch(a,b'{"operation":"snapshot","capability":1}')["status"],"COMPLETED")
        self.assertEqual(old.dispatch(a,write)["status"],"ACCEPTED")

        # Reconstructing the broker represents process restart: owner state is
        # empty even while the downstream endpoint still retains the old ID.
        backend.session=12
        restarted=Gateway(policy,backend)
        self.assertEqual(restarted.dispatch(a,status)["status"],"UNKNOWN_OUTCOME")
        calls=len(backend.calls)
        self.assertEqual(restarted.dispatch(a,write)["status"],"UNAVAILABLE")
        self.assertEqual(len(backend.calls),calls)
        self.assertEqual(restarted.dispatch(a,b'{"operation":"snapshot","capability":1}')["status"],"COMPLETED")
        self.assertEqual(restarted.dispatch(a,write)["status"],"ACCEPTED")
        self.assertEqual(backend.calls[-1][0],"request")
    def test_strict_schema(self):
        for raw in [b'{}', b'[]', b'{"operation":"snapshot","capability":true}',
                    b'{"operation":"snapshot","capability":1,"capability":2}',
                    b'{"operation":"snapshot","capability":1,"principal":"operator"}', b"x"*513]:
            with self.assertRaises(ValueError):
                parse_command(raw)

if __name__ == "__main__":
    unittest.main()
