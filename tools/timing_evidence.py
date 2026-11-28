"""Streaming measurement evaluator for approved, independently acquired bench data.
Measurements are observations, not WCET proofs. No physical commands are issued.
"""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path

def positive(x):
    return type(x) is int and x>0

def integer(row,key):
    value=int(row[key])
    if value<0:raise ValueError("negative "+key)
    return value

def evaluate_cycles(rows,period_us,deadline_us,duration_us):
    if not all(positive(x) for x in (period_us,deadline_us,duration_us)) or deadline_us>period_us:
        raise ValueError("invalid allocated timing")
    first=previous=None
    count=misses=gaps=max_response=0
    for row in rows:
        release,start,end=(integer(row,k) for k in ("release_us","start_us","completion_us"))
        if not release<=start<=end:raise ValueError("invalid cycle timestamps")
        if previous is not None:
            delta=release-previous
            if delta<=0 or delta%period_us:raise ValueError("release phase mismatch")
            gaps+=delta//period_us-1
        else:first=release
        previous=release;count+=1
        response=end-release
        max_response=max(max_response,response)
        misses+=response>=deadline_us
    span=0 if first is None else previous-first+period_us
    return dict(verdict="PASS" if count and span>=duration_us and not misses and not gaps else "FAIL",
                count=count,span_us=span,misses=misses,gaps=gaps,observed_max_response_us=max_response)

def evaluate_transport(rows,latency_limit_us,window_us,bit_rate,bits_per_byte,utilization_permille,minimum_samples):
    if not all(positive(x) for x in (latency_limit_us,window_us,bit_rate,bits_per_byte,utilization_permille,minimum_samples)) or utilization_permille>1000:
        raise ValueError("invalid transport allocation")
    previous=None;first=None;latest_end=0;count=late=max_latency=max_tx=max_rx=0
    current_window=None;tx=rx=0
    # Each row represents independent complete exchange, bytes on wire including
    # framing. Fixed nonoverlapping windows by transmission start; conservative
    # accounting charges the entire exchange to its start window.
    for row in rows:
        start,end,sent,received=(integer(row,k) for k in ("sent_us","received_us","tx_wire_bytes","rx_wire_bytes"))
        if end<start or previous is not None and start<previous:raise ValueError("unordered transport timestamps")
        if not sent and not received:raise ValueError("empty exchange")
        if first is None:first=start
        bucket=(start-first)//window_us
        if current_window is not None and bucket!=current_window:
            max_tx=max(max_tx,tx);max_rx=max(max_rx,rx);tx=rx=0
        current_window=bucket;previous=start;latest_end=max(latest_end,end)
        tx+=sent;rx+=received;count+=1
        latency=end-start;max_latency=max(max_latency,latency);late+=latency>=latency_limit_us
    max_tx=max(max_tx,tx);max_rx=max(max_rx,rx)
    denom=bit_rate*window_us
    tx_permille=(max_tx*bits_per_byte*1000000000+denom-1)//denom
    rx_permille=(max_rx*bits_per_byte*1000000000+denom-1)//denom
    return dict(verdict="PASS" if count>=minimum_samples and not late and max(tx_permille,rx_permille)<=utilization_permille else "FAIL",
                samples=count,observed_max_latency_us=max_latency,late=late,
                max_tx_permille=tx_permille,max_rx_permille=rx_permille,
                span_us=0 if first is None else latest_end-first)

PROFILE_METRICS=("boot_ms","latency_us","jitter_us","ram_bytes","flash_bytes","cpu_permille","power_mw")
PROFILE_INTEGER_METRICS={"ram_bytes","flash_bytes"}
PROFILE_UNITS={"boot_ms":"ms","latency_us":"us","jitter_us":"us",
               "ram_bytes":"bytes","flash_bytes":"bytes","cpu_permille":"permille",
               "power_mw":"mW"}

PROFILE_PLAN_METADATA=("hardware_assumptions","staffing_assumptions","limitations")

def validate_profile_plan(plan):
    """Require caller-supplied context without inferring or approving evidence."""
    if not isinstance(plan,dict):
        raise ValueError("profile plan must be an object")
    for key in ("profile","toolchain"):
        if not isinstance(plan.get(key),str) or not plan[key].strip():
            raise ValueError("missing "+key)
    digest_value=plan.get("firmware_sha256")
    if (not isinstance(digest_value,str) or len(digest_value)!=64 or
            any(c not in "0123456789abcdef" for c in digest_value)):
        raise ValueError("invalid firmware SHA-256")
    for key in PROFILE_PLAN_METADATA:
        values=plan.get(key)
        if (not isinstance(values,list) or not values or
                any(not isinstance(value,str) or not value.strip() for value in values)):
            raise ValueError("missing or invalid "+key)

def evaluate_profile(rows,profile):
    """Summarize an acquired per-profile trace without imposing universal limits."""
    if not isinstance(profile,str) or not profile.strip():
        raise ValueError("profile identity required")
    values={key:[] for key in PROFILE_METRICS}
    count=0
    for index,row in enumerate(rows,1):
        if row.get("profile")!=profile:
            raise ValueError(f"row {index}: profile identity mismatch")
        for key in PROFILE_METRICS:
            raw=row.get(key)
            try:
                value=float(raw)
            except (TypeError,ValueError):
                raise ValueError(f"row {index}: invalid {key}") from None
            if not math.isfinite(value) or value<0:
                raise ValueError(f"row {index}: invalid {key}")
            if key in PROFILE_INTEGER_METRICS and not value.is_integer():
                raise ValueError(f"row {index}: {key} must be an integer")
            values[key].append(value)
        count+=1
    if not count:
        raise ValueError("profile trace has no measurements")
    summary={}
    for key,samples in values.items():
        summary[key]={"unit":PROFILE_UNITS[key],
                      "count":len(samples),"minimum":min(samples),"maximum":max(samples),
                      "mean":sum(value/len(samples) for value in samples)}
    return {"profile":profile,"samples":count,"metrics":summary,
            "verdict":"RECORDED",
            "scope":"observed measurements only; no universal threshold, WCET, product-support or approval claim",
            "approval_asserted":False}

def digest(path):
    h=hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda:f.read(1024*1024),b""):h.update(chunk)
    return h.hexdigest()

def main():
    p=argparse.ArgumentParser()
    p.add_argument("kind",choices=("cycles","transport","profile"))
    p.add_argument("--plan",type=Path,required=True)
    p.add_argument("--trace",type=Path,required=True)
    a=p.parse_args()
    plan=json.loads(a.plan.read_text(encoding="utf-8"))
    for key in ("configuration","board","clock","instrument","procedure"):
        if not isinstance(plan.get(key),str) or not plan[key].strip():raise ValueError("missing "+key)
    with a.trace.open(newline="",encoding="utf-8") as f:
        rows=csv.DictReader(f)
        if a.kind=="cycles":
            if plan.get("profile")=="G1-C-LAB" and (plan.get("duration_us",0)<86400000000 or plan.get("deadline_us")!=10000):
                raise ValueError("G1-C-LAB requires 24 h and 10 ms deadline")
            result=evaluate_cycles(rows,plan["period_us"],plan["deadline_us"],plan["duration_us"])
        elif a.kind=="transport":
            result=evaluate_transport(rows,*(plan[k] for k in ("latency_limit_us","window_us","bit_rate","bits_per_byte","utilization_permille","minimum_samples")))
        else:
            validate_profile_plan(plan)
            result=evaluate_profile(rows,plan["profile"])
            for key in PROFILE_PLAN_METADATA:
                result[key]=plan[key]
    scope=(result["scope"] if a.kind=="profile" else
           "observed measurements only; responsible reviewer must establish trace completeness and clock uncertainty")
    result.update(configuration=plan["configuration"],plan_sha256=digest(a.plan),trace_sha256=digest(a.trace),scope=scope)
    print(json.dumps(result,indent=2))
    return 0 if a.kind=="profile" or result["verdict"]=="PASS" else 1
if __name__=="__main__":raise SystemExit(main())
