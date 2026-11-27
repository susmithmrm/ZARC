"""Streaming measurement evaluator for approved, independently acquired bench data.
Measurements are observations, not WCET proofs. No physical commands are issued.
"""
import argparse
import csv
import hashlib
import json
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

def digest(path):
    h=hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda:f.read(1024*1024),b""):h.update(chunk)
    return h.hexdigest()

def main():
    p=argparse.ArgumentParser()
    p.add_argument("kind",choices=("cycles","transport"))
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
        else:
            result=evaluate_transport(rows,*(plan[k] for k in ("latency_limit_us","window_us","bit_rate","bits_per_byte","utilization_permille","minimum_samples")))
    result.update(configuration=plan["configuration"],plan_sha256=digest(a.plan),trace_sha256=digest(a.trace),
                  scope="observed measurements only; responsible reviewer must establish trace completeness and clock uncertainty")
    print(json.dumps(result,indent=2))
    return 0 if result["verdict"]=="PASS" else 1
if __name__=="__main__":raise SystemExit(main())
