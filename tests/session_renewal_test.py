"""Exercise real loopback retention saturation and explicit retirement."""
import subprocess
import sys
import time
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"host"))
from zarc_host.lab_backend import LabBackend
p=subprocess.Popen([sys.argv[1],"601"],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
backend=None
try:
    backend=LabBackend(int(p.stdout.readline()))
    assert backend.snapshot(1)["status"]=="COMPLETED"
    epoch=dict(boot=backend.boot,session=backend.session)
    first=None
    for i in range(16):
        accepted=backend.request(1,2 if i%2==0 else 1,1000)
        assert accepted["status"]=="ACCEPTED",accepted
        if first is None:first=accepted
        if i==0:
            assert backend.renew_session()["status"]=="BUSY"
            assert backend.session==epoch["session"]
        deadline=time.monotonic()+2
        while backend.request_status(accepted["request_id"],**epoch)["status"]!="COMPLETED":
            assert time.monotonic()<deadline
            time.sleep(.02)
    assert backend.request(1,2,1000)["status"]=="BUSY"
    assert backend.request_status(first["request_id"],**epoch)["status"]=="COMPLETED"
    renewed=backend.renew_session()
    assert renewed["status"]=="COMPLETED" and renewed["session"]==epoch["session"]+1
    assert backend.request_status(first["request_id"],**epoch)["status"]=="UNKNOWN_OUTCOME"
    assert backend.subscribe(1)["status"]=="COMPLETED"
    assert backend.request(1,2,1000)["status"]=="ACCEPTED"
finally:
    if backend is not None:backend.invalidate()
    p.terminate()
    try:p.communicate(timeout=3)
    except subprocess.TimeoutExpired:p.kill();p.communicate()
