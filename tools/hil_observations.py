"""Evaluate independently acquired benign indicator bench traces.

This script never commands hardware. It compares externally recorded
admission, observed feedback and disconnect timing against an approved plan.
"""
import argparse
import json
from pathlib import Path
import hashlib
import math

def evaluate(plan, events):
    if not isinstance(events,list) or not events:
        raise ValueError("missing independent trace")
    for key in ("board","firmware_sha256","configuration","instrument","clock","procedure"):
        if not isinstance(plan.get(key),str) or not plan[key].strip():
            raise ValueError("missing bench identity: "+key)
    limit=plan.get("max_feedback_ms")
    if type(limit) not in (float,int) or not math.isfinite(limit) or limit<=0:
        raise ValueError("approved positive feedback deadline required")
    expected=plan.get("expected_value")
    if type(expected) is not int or expected not in (1,2):
        raise ValueError("benign indicator only")
    last=-1
    seen={}
    for event in events:
        tick=event.get("time_ms")
        if type(tick) not in (int,float) or not math.isfinite(tick) or tick<0 or tick<last:
            raise ValueError("invalid trace clock")
        last=tick
        name=event.get("event")
        if name in ("admitted","link_disconnected","feedback"):
            if name in seen:
                raise ValueError("ambiguous observation")
            seen[name]=event
    required={"admitted","link_disconnected","feedback"}
    if set(seen)!=required:
        return {"verdict":"FAIL","reason":"missing required observation"}
    a,d,f=(seen[k] for k in ("admitted","link_disconnected","feedback"))
    delta=f["time_ms"]-a["time_ms"]
    passed=(a["time_ms"]<=d["time_ms"]<=f["time_ms"] and 0<=delta<limit
            and f.get("source")=="independent_instrument" and f.get("value")==expected
            and f.get("quality")=="VALID")
    return {"verdict":"PASS" if passed else "FAIL","feedback_ms":delta,
            "limit_ms":limit,"configuration":plan["configuration"]}

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--plan",type=Path,required=True)
    p.add_argument("--trace",type=Path,required=True)
    args=p.parse_args()
    raw=args.trace.read_bytes()
    result=evaluate(json.loads(args.plan.read_text()),json.loads(raw))
    result["trace_sha256"]=hashlib.sha256(raw).hexdigest()
    print(json.dumps(result,indent=2))
    return 0 if result["verdict"]=="PASS" else 1

if __name__=="__main__":
    raise SystemExit(main())
