import struct
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"host"))
from zarc_host.aaos import *
from zarc_host.services import Binding, Service, sdv_availability, STATUSES
from zarc_host.security import Policy, Principal
from zarc_host.broker import Gateway

class Backend:
    def snapshot(self, capability):
        return {"status":"COMPLETED","value":2,"quality":1,"boot":101,"generation":2}
    def request(self, capability, desired, validity):
        return {"status":"ACCEPTED"}
    def invalidate(self):
        pass

class Tests(unittest.TestCase):
    def test_aaos(self):
        with self.assertRaises(ValueError):
            decode_report(AP_POWER_STATE_REQ,(1,0))
        self.assertFalse(decode_report(AP_POWER_STATE_REPORT,(7,0)).ready)
        self.assertTrue(decode_report(AP_POWER_STATE_REPORT,(2,15)).ready)
        with self.assertRaises(ValueError):
            decode_report(AP_POWER_STATE_REPORT,(4,5001))
        self.assertEqual(encode_request(Request.CANCEL_SHUTDOWN,0,Report.SHUTDOWN_PREPARE,allowed_shutdown_params={1}),
                         (AP_POWER_STATE_REQ,(2,0)))
        with self.assertRaises(ValueError):
            encode_request(Request.FINISHED,0,Report.SHUTDOWN_PREPARE,allowed_shutdown_params={1})
        with self.assertRaises(ValueError):
            encode_request(Request.SHUTDOWN_PREPARE,2,Report.ON,allowed_shutdown_params={1})
    def test_service(self):
        reader=Principal("observer",frozenset({1}),frozenset())
        gateway=Gateway(Policy({"1"*64:reader}),Backend())
        binding=Binding(0x1234,1,1,2,0x8001,1)
        service=Service(gateway,binding)
        def call(method,payload,major=1):
            return service.dispatch(reader,service=binding.service,instance=1,method=method,major=major,payload=payload)
        self.assertEqual(call(1,struct.pack(">I",1)),struct.pack(">BBBQQ",1,2,1,101,2))
        self.assertEqual(call(2,struct.pack(">IBI",1,2,100)),bytes([STATUSES.index("DENIED")]))
        self.assertEqual(call(1,b"bad"),bytes([STATUSES.index("INVALID")]))
        self.assertEqual(call(1,b"bad",2),bytes([STATUSES.index("INCOMPATIBLE")]))
        self.assertFalse(sdv_availability(True,None)["physical_available"])
        self.assertFalse(sdv_availability(True,{"status":"COMPLETED","quality":2})["physical_available"])
        self.assertTrue(sdv_availability(True,{"status":"COMPLETED","quality":1})["physical_available"])

if __name__=="__main__":
    unittest.main()
