import subprocess
import sys
import time
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"host"))
from zarc_host.lab_backend import LabBackend
p=subprocess.Popen([sys.argv[1],"301"],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
backend=None
try:
    backend=LabBackend(int(p.stdout.readline()))
    snapshot=backend.subscribe(1)
    assert snapshot["status"]=="COMPLETED" and snapshot["value"]==1
    assert backend.capabilities=={1:True}
    assert backend.snapshot(99)["status"]=="UNAVAILABLE"
    epoch=dict(boot=backend.boot,session=backend.session)
    accepted=backend.request(1,2,1000)
    assert accepted["status"]=="ACCEPTED"
    assert backend.request_status(accepted["request_id"],**epoch)["status"]=="ACCEPTED"
    until=time.monotonic()+3
    cursor=snapshot["sequence"]
    while True:
        event=backend.event(cursor,**epoch)
        if event["status"]=="COMPLETED":
            assert event["sequence"]==cursor+1
            assert event["value"]==2 and event["quality"]==1
            break
        assert event["status"]=="BUSY" and time.monotonic()<until
        time.sleep(.02)
    assert backend.request_status(accepted["request_id"],**epoch)["status"]=="COMPLETED"
    assert backend.event(event["sequence"]+1,**epoch)["status"]=="UNKNOWN_OUTCOME"
    backend.invalidate()
    assert backend.event(cursor,**epoch)["status"]=="UNKNOWN_OUTCOME"
    assert backend.capabilities=={}
    fresh=backend.subscribe(1)
    assert fresh["status"]=="COMPLETED" and fresh["value"]==2
    assert backend.session>epoch["session"] and backend.boot==epoch["boot"]
    assert backend.capabilities=={1:True}
    assert backend.request_status(accepted["request_id"],**epoch)["status"]=="UNKNOWN_OUTCOME"
finally:
    if backend is not None:backend.invalidate()
    p.terminate()
    try:p.communicate(timeout=3)
    except subprocess.TimeoutExpired:p.kill();p.communicate()
