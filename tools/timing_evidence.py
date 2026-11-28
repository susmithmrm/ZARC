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
TIMESTAMP_EVENTS={"acquisition","output"}

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

def validate_timestamp_plan(plan):
    """Require explicit, caller-supplied timestamp allocations and transition method."""
    if not isinstance(plan,dict):
        raise ValueError("timestamp plan must be an object")
    for key in ("configuration","board","clock","instrument","procedure"):
        value=plan.get(key)
        if not isinstance(value,str) or not value.strip():
            raise ValueError("missing "+key)
    for key in ("timer_accuracy_us","capture_sync_uncertainty_us","allocated_jitter_us"):
        value=plan.get(key)
        if type(value) is not int or value<0:
            raise ValueError("invalid "+key)
    epochs=plan.get("clock_epochs_required")
    if type(epochs) is not int or epochs<2:
        raise ValueError("clock_epochs_required must cover a transition")
    samples=plan.get("minimum_samples_per_event_per_epoch")
    if type(samples) is not int or samples<1:
        raise ValueError("minimum_samples_per_event_per_epoch must be positive")
    for key in ("capture_method","clock_transition_method"):
        value=plan.get(key)
        if not isinstance(value,str) or not value.strip():
            raise ValueError("missing "+key)

def _signed_integer(row,key,index):
    raw=row.get(key)
    if type(raw) is int:
        return raw
    if not isinstance(raw,str) or not raw or raw.strip()!=raw:
        raise ValueError(f"row {index}: invalid {key}")
    digits=raw[1:] if raw.startswith("-") else raw
    if not digits.isascii() or not digits.isdigit():
        raise ValueError(f"row {index}: invalid {key}")
    return int(raw)

def _unsigned_integer(row,key,index):
    raw=row.get(key)
    if type(raw) is int:
        value=raw
    elif isinstance(raw,str) and raw and raw.strip()==raw and raw.isascii() and raw.isdigit():
        value=int(raw)
    else:
        raise ValueError(f"row {index}: invalid {key}")
    if value<0:
        raise ValueError(f"row {index}: negative {key}")
    return value

def evaluate_timestamps(rows,plan):
    """Compare caller-provided external timestamp observations with declared allocations.

    The input records must already contain measurements made by the named
    external instrument. This evaluator checks trace shape and supplied error
    values; it does not authenticate acquisition or approve a target.
    """
    validate_timestamp_plan(plan)
    previous_external=previous_scheduled=None
    epochs=set()
    counts={}
    samples_by_epoch={}
    max_timer_error=max_capture_error=max_jitter=0
    timer_violations=capture_violations=jitter_violations=0
    count=0
    last_epoch=-1
    for index,row in enumerate(rows,1):
        epoch=row.get("clock_epoch")
        if type(epoch) is not int:
            if not isinstance(epoch,str) or not epoch or epoch.strip()!=epoch or not epoch.isascii() or not epoch.isdigit():
                raise ValueError(f"row {index}: invalid clock_epoch")
            epoch=int(epoch)
        if epoch<0:
            raise ValueError(f"row {index}: invalid clock_epoch")
        if epoch<last_epoch or epoch>last_epoch+1:
            raise ValueError(f"row {index}: clock epochs must be contiguous and ordered")
        event=row.get("event")
        if not isinstance(event,str) or event not in TIMESTAMP_EVENTS:
            raise ValueError(f"row {index}: invalid event")
        scheduled=_unsigned_integer(row,"scheduled_us",index)
        external=_unsigned_integer(row,"external_us",index)
        if previous_scheduled is not None and scheduled<previous_scheduled:
            raise ValueError(f"row {index}: scheduled timestamps must not decrease")
        if previous_external is not None and external<=previous_external:
            raise ValueError(f"row {index}: external timestamps must increase")
        previous_external=external;previous_scheduled=scheduled
        timer_error=_signed_integer(row,"timer_error_us",index)
        capture_error=_signed_integer(row,"capture_sync_error_us",index)
        jitter=external-scheduled
        max_timer_error=max(max_timer_error,abs(timer_error))
        max_capture_error=max(max_capture_error,abs(capture_error))
        max_jitter=max(max_jitter,abs(jitter))
        timer_violations+=abs(timer_error)>plan["timer_accuracy_us"]
        capture_violations+=abs(capture_error)>plan["capture_sync_uncertainty_us"]
        jitter_violations+=abs(jitter)>plan["allocated_jitter_us"]
        epochs.add(epoch)
        counts[(epoch,event)]=counts.get((epoch,event),0)+1
        samples_by_epoch.setdefault(epoch,{"acquisition":0,"output":0})[event]+=1
        count+=1
        last_epoch=epoch
    if not count:
        raise ValueError("timestamp trace has no measurements")
    expected_epochs=set(range(max(epochs)+1))
    if epochs!=expected_epochs or len(epochs)<plan["clock_epochs_required"]:
        raise ValueError("timestamp trace does not cover required clock transitions")
    for epoch in epochs:
        for event in TIMESTAMP_EVENTS:
            if counts.get((epoch,event),0)<plan["minimum_samples_per_event_per_epoch"]:
                raise ValueError(f"insufficient {event} samples in clock epoch {epoch}")
    violations={"timer_accuracy":timer_violations,
                "capture_synchronization":capture_violations,
                "allocated_jitter":jitter_violations}
    return {"verdict":"RECORDED","within_allocations":not any(violations.values()),
            "samples":count,"clock_epochs":sorted(epochs),"samples_by_epoch":samples_by_epoch,
            "observed_max_timer_error_us":max_timer_error,
            "timer_accuracy_us":plan["timer_accuracy_us"],
            "observed_max_capture_sync_error_us":max_capture_error,
            "capture_sync_uncertainty_us":plan["capture_sync_uncertainty_us"],
            "observed_max_jitter_us":max_jitter,
            "allocated_jitter_us":plan["allocated_jitter_us"],
            "clock":plan["clock"],"instrument":plan["instrument"],
            "capture_method":plan["capture_method"],
            "clock_transition_method":plan["clock_transition_method"],
            "violations":violations,"approval_asserted":False,
            "scope":"caller-supplied external observations and allocations only; acquisition authenticity, target acceptance and independent approval are not asserted"}

def digest(path):
    h=hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda:f.read(1024*1024),b""):h.update(chunk)
    return h.hexdigest()

def main():
    p=argparse.ArgumentParser()
    p.add_argument("kind",choices=("cycles","transport","profile","timestamps"))
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
        elif a.kind=="profile":
            validate_profile_plan(plan)
            result=evaluate_profile(rows,plan["profile"])
            for key in PROFILE_PLAN_METADATA:
                result[key]=plan[key]
        else:
            result=evaluate_timestamps(rows,plan)
    scope=(result["scope"] if a.kind in ("profile","timestamps") else
           "observed measurements only; responsible reviewer must establish trace completeness and clock uncertainty")
    result.update(configuration=plan["configuration"],plan_sha256=digest(a.plan),trace_sha256=digest(a.trace),scope=scope)
    print(json.dumps(result,indent=2))
    if a.kind=="profile":return 0
    if a.kind=="timestamps":return 0 if result["within_allocations"] else 1
    return 0 if result["verdict"]=="PASS" else 1
if __name__=="__main__":raise SystemExit(main())
