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
        return {"status": "ACCEPTED", "boot": self.boot,
                "session": self.session, "request_id": 12}
    def invalidate(self):
        self.invalidated = True

class Tests(unittest.TestCase):
    def test_principal_policy_is_immutable_and_strictly_typed(self):
        with self.assertRaises(ValueError):
            Principal("mutable", {1}, frozenset({1}))
        with self.assertRaises(ValueError):
            Principal("mutable", frozenset({1}), {1})
        with self.assertRaises(ValueError):
            Principal("boolean-rate", frozenset({1}), frozenset(), True)
        with self.assertRaises(ValueError):
            Principal(" ", frozenset({1}), frozenset())
        with self.assertRaises(ValueError):
            Principal("invalid-update", frozenset({1}), frozenset(), update_read=1)
        for rate in (0,101,-1,1.0):
            with self.subTest(rate=rate),self.assertRaises(ValueError):
                Principal("invalid-rate",frozenset({1}),frozenset(),rate)
        for capability in (0,0x100000000):
            with self.subTest(capability=capability),self.assertRaises(ValueError):
                Principal("invalid-capability",frozenset({capability}),frozenset())
        self.assertEqual(Principal("minimum",frozenset({1}),frozenset(),1).rate,1)
        self.assertEqual(Principal("maximum",frozenset({0xFFFFFFFF}),frozenset(),100).rate,100)
        principal=Principal("valid",frozenset({1}),frozenset({2}))
        self.assertEqual(principal.read,frozenset({1}))
        self.assertEqual(principal.write,frozenset({2}))

    def test_policy_rejects_malformed_config_and_unregistered_equal_principal(self):
        principal=Principal("operator",frozenset({1}),frozenset({1}))
        for invalid in (None, [], {}):
            with self.subTest(certificates=invalid),self.assertRaises(ValueError):
                Policy(invalid)
        with self.assertRaises(ValueError):
            Policy({None:principal})
        with self.assertRaises(ValueError):
            Policy({"1"*64:object()})
        for fingerprint in ("1"*63,"A"*64,"1"*65):
            with self.subTest(fingerprint=fingerprint),self.assertRaises(ValueError):
                Policy({fingerprint:principal})
        with self.assertRaises(ValueError):
            Policy({"1"*64:principal},clock=None)
        entries={f"{value:064x}":principal for value in range(64)}
        self.assertEqual(len(Policy(entries)._entries),64)
        entries[f"{64:064x}"]=principal
        with self.assertRaises(ValueError):
            Policy(entries)

        backend=Backend()
        gateway=Gateway(Policy({"1"*64:principal}),backend)
        forged=Principal("operator",frozenset({1}),frozenset({1}))
        request=b'{"operation":"snapshot","capability":1}'
        self.assertEqual(gateway.dispatch(forged,request)["status"],"DENIED")
        self.assertFalse(backend.calls)

    def test_policy_der_lookup_rejects_non_der_inputs(self):
        principal=Principal("operator",frozenset({1}),frozenset({1}))
        policy=Policy({"1"*64:principal})
        for invalid in (None,"certificate",b""):
            with self.subTest(der=invalid),self.assertRaises(PermissionError):
                policy.identify_der(invalid)

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
    def test_accepted_request_requires_valid_retained_identity(self):
        principal=Principal("operator",frozenset({1}),frozenset({1}),100)
        valid={"status":"ACCEPTED","boot":10,"session":11,"request_id":12}
        malformed=(
            {"status":"ACCEPTED"},
            {**valid,"boot":True},
            {**valid,"boot":0},
            {**valid,"session":False},
            {**valid,"session":0},
            {**valid,"request_id":True},
            {**valid,"request_id":0},
            {**valid,"request_id":"12"},
        )
        for response in malformed:
            with self.subTest(response=response):
                class MalformedAccepted(Backend):
                    def request(self,cap,desired,validity):
                        return response
                backend=MalformedAccepted()
                gateway=Gateway(Policy({"1"*64:principal}),backend)
                snapshot=b'{"operation":"snapshot","capability":1}'
                write=b'{"operation":"request","capability":1,"desired":2,"validity":100}'
                self.assertEqual(gateway.dispatch(principal,snapshot)["status"],"COMPLETED")
                self.assertEqual(gateway.dispatch(principal,write),{"status":"UNKNOWN_OUTCOME"})
                self.assertTrue(backend.invalidated)
                self.assertFalse(gateway._owners)
                self.assertFalse(gateway._snapshot_ready)
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
