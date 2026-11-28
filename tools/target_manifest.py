"""Validate candidate product/target composition and generate immutable C tables.

No board or timing claim is inferred from a descriptor. Only independently
approved evidence may promote a configuration beyond CANDIDATE.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

PROFILES={"edge": {"core","health"}, "companion":{"core","health","power","broker"},
          "zonal":{"core","health","endpoint"}, "integrated":{"core","health","ipc"}}
PHASES={"reset","boot","run","maintenance","shutdown"}
CONTEXTS={"I0","R1","R2","C3","O4","M5"}
HARDWARE_LIFECYCLE_FIELDS={"schema","board_revision","silicon_stepping","errata",
                           "vendor_safety_docs","components","review_status",
                           "review_reference"}
def number(x,minimum=1,maximum=0xffffffff):
    return type(x) is int and minimum<=x<=maximum

def _validate_hardware_lifecycle(record):
    """Validate a supplied lifecycle snapshot; never infer hardware facts."""
    if not isinstance(record,dict) or set(record)!=HARDWARE_LIFECYCLE_FIELDS:
        raise ValueError("hardware lifecycle record fields")
    if record["schema"]!=1 or type(record["schema"]) is not int:
        raise ValueError("hardware lifecycle schema")
    for key in ("board_revision","silicon_stepping"):
        if not isinstance(record[key],str) or not record[key].strip():
            raise ValueError("hardware lifecycle missing "+key)
    if record["review_status"] not in ("UNREVIEWED","REVIEWED"):
        raise ValueError("hardware lifecycle review status")
    ref=record["review_reference"]
    if not isinstance(ref,str) or (record["review_status"]=="REVIEWED" and not ref.strip()):
        raise ValueError("hardware lifecycle review reference")
    collections=(
        ("errata",{"id","reference"}),
        ("vendor_safety_docs",{"id","revision","reference"}),
        ("components",{"id","revision","substitutions"}),
    )
    for name,fields in collections:
        rows=record[name]
        if not isinstance(rows,list):
            raise ValueError("hardware lifecycle "+name+" must be a list")
        seen=set()
        for row in rows:
            if not isinstance(row,dict) or set(row)!=fields:
                raise ValueError("hardware lifecycle "+name+" record fields")
            identity=row["id"]
            if not isinstance(identity,str) or not identity.strip() or identity in seen:
                raise ValueError("hardware lifecycle duplicate/invalid "+name+" id")
            seen.add(identity)
            for key in fields-{"id","substitutions"}:
                if not isinstance(row[key],str) or not row[key].strip():
                    raise ValueError("hardware lifecycle missing "+name+" "+key)
            if "substitutions" in fields:
                values=row["substitutions"]
                if (not isinstance(values,list) or
                    any(not isinstance(value,str) or not value.strip() for value in values) or
                    len(values)!=len(set(values))):
                    raise ValueError("hardware lifecycle substitutions")

def hardware_lifecycle_delta(before,after):
    """Report target lifecycle changes; no hardware acceptance is inferred.

    ``before`` and ``after`` are hardware_lifecycle records from two candidate
    target manifests. Missing snapshots, unreviewed records or any delta remain
    REVIEW_REQUIRED. A no-change result means only that the supplied records
    match and both carry review references; it is not board or vendor approval.
    """
    for record in (before,after):
        if record is not None:
            _validate_hardware_lifecycle(record)
    changes=[]
    if before is None:
        changes.append({"field":"hardware_lifecycle","change":"MISSING_BEFORE"})
    if after is None:
        changes.append({"field":"hardware_lifecycle","change":"MISSING_AFTER"})
    if before is not None and after is not None:
        for field in ("board_revision","silicon_stepping"):
            if before[field]!=after[field]:
                changes.append({"field":field,"change":"CHANGED",
                                "before":before[field],"after":after[field]})
        for name,key_fields in (("errata",("reference",)),
                                ("vendor_safety_docs",("revision","reference")),
                                ("components",("revision","substitutions"))):
            old={row["id"]:row for row in before[name]}
            new={row["id"]:row for row in after[name]}
            for identity in sorted(old.keys()|new.keys()):
                if identity not in old:
                    changes.append({"field":name,"id":identity,"change":"ADDED"})
                elif identity not in new:
                    changes.append({"field":name,"id":identity,"change":"REMOVED"})
                else:
                    for field in key_fields:
                        if old[identity][field]!=new[identity][field]:
                            changes.append({"field":name,"id":identity,"change":"CHANGED",
                                            "attribute":field,"before":old[identity][field],
                                            "after":new[identity][field]})
        for side,record in (("before",before),("after",after)):
            if record["review_status"]!="REVIEWED" or not record["review_reference"].strip():
                changes.append({"field":"review_status","record":side,
                                "change":"UNREVIEWED"})
    verdict="REVIEW_REQUIRED" if changes else "NO_CHANGE_RECORDED"
    return {"verdict":verdict,"changes":changes,
            "hardware_acceptance":"NOT_ASSERTED",
            "scope":"manifest delta only; vendor facts and selected-target acceptance remain external"}

def validate_execution(m):
    """Validate declared candidate architecture, not silicon support or safety evidence."""
    e=m.get("execution")
    if not isinstance(e,dict):
        raise ValueError("execution descriptor required")
    for k in ("isa","core","port_owner","port_reference","abi"):
        if not isinstance(e.get(k),str) or not e[k].strip():
            raise ValueError("missing execution "+k)
    mode=e.get("mode")
    if mode not in ("single","smp","amp","lockstep","split_lock","simulator"):
        raise ValueError("execution mode")
    count=e.get("core_count")
    if not number(count,1,256):
        raise ValueError("core count")
    if mode=="single" and count!=1:
        raise ValueError("single execution requires one core")
    if mode in ("smp","amp","lockstep","split_lock") and count<2:
        raise ValueError("multicore mode requires multiple cores")
    if e.get("byte_order") not in ("little","big","host"):
        raise ValueError("byte order")
    if e.get("word_bits") not in (16,32,64) or type(e.get("word_bits")) is not int:
        raise ValueError("word bits")
    if not number(e.get("alignment_bytes"),1,64) or e["alignment_bytes"] & (e["alignment_bytes"]-1):
        raise ValueError("alignment")
    sim=m.get("board")=="native_sim/native/64"
    if sim!=(mode=="simulator") or (not sim and e["byte_order"]=="host"):
        raise ValueError("simulator execution mismatch")
    if m["features"]["smp"]!=(mode=="smp"):
        raise ValueError("SMP execution mismatch")
    if mode in ("lockstep","split_lock") and (not isinstance(e.get("mode_reference"),str) or not e["mode_reference"].strip()):
        raise ValueError("vendor mode reference required")
    validate_memory(m,e,sim)

def validate_memory(m,e,sim):
    """Check declared address allocation; actual linker/MPU evidence is separate."""
    regions=m.get("memory_regions")
    if regions is None and sim:
        return
    if not isinstance(regions,list) or not 1<=len(regions)<=64:
        raise ValueError("physical target requires memory region inventory")
    limit=1<<e["word_bits"]
    names=set(); spans=[]; types=set()
    for region in regions:
        if not isinstance(region,dict):
            raise ValueError("invalid memory region")
        name=region.get("name")
        kind=region.get("kind")
        start=region.get("base")
        size=region.get("size")
        alignment=region.get("alignment")
        if not isinstance(name,str) or not re.fullmatch(r"[a-z][a-z0-9_]*",name) or name in names:
            raise ValueError("invalid or duplicate memory name")
        if kind not in ("code","ram","tcm","external_ram","nonvolatile","shared","device"):
            raise ValueError("unknown memory kind")
        if not number(start,0,limit-1) or not number(size,1,limit) or start+size>limit:
            raise ValueError("memory address overflow")
        if not number(alignment,1,limit) or alignment&(alignment-1) or start%alignment or size%alignment:
            raise ValueError("memory alignment")
        for key in ("read","write","execute","cacheable","dma"):
            if type(region.get(key)) is not bool:
                raise ValueError("memory permission or attribute missing")
        if not isinstance(region.get("owner"),str) or not region["owner"].strip():
            raise ValueError("memory ownership missing")
        if not region["read"] or (kind=="device" and (region["execute"] or region["cacheable"])):
            raise ValueError("invalid memory attributes")
        if kind=="shared" and (not isinstance(region.get("coherency"),str) or not region["coherency"].strip()):
            raise ValueError("shared memory requires coherency contract")
        if kind=="external_ram" and (not isinstance(region.get("initialization"),str) or not region["initialization"].strip()):
            raise ValueError("external memory requires initialization dependency")
        names.add(name);types.add(kind);spans.append((start,start+size))
    spans.sort()
    if any(spans[i][0]<spans[i-1][1] for i in range(1,len(spans))):
        raise ValueError("overlapping memory allocations")
    if not sim and (not any(r["execute"] and r["kind"]!="device" for r in regions) or
                    not any(r["write"] and r["kind"] in ("ram","tcm","external_ram") for r in regions)):
        raise ValueError("missing code or writable runtime memory")

def validate(m):
    if not isinstance(m,dict):
        raise ValueError("manifest object")
    for key in ("target","board","soc","revision","zephyr","toolchain","maintainer"):
        if not isinstance(m.get(key),str) or not m[key].strip():
            raise ValueError("missing "+key)
    if m.get("schema")!=1 or m.get("api_major")!=1 or m.get("maturity")!="CANDIDATE":
        raise ValueError("only schema/API 1 and candidate scope admitted")
    if "hardware_lifecycle" in m:
        _validate_hardware_lifecycle(m["hardware_lifecycle"])
    profile=m.get("profile")
    if profile not in PROFILES:
        raise ValueError("unknown profile")
    services=m.get("services")
    if not isinstance(services,list) or not 1<=len(services)<=16:
        raise ValueError("service count")
    ids=set();names=set();graph={}
    for s in services:
        if not number(s["id"]) or s["id"] in ids or not isinstance(s["name"],str) or not re.fullmatch(r"[a-z][a-z0-9_]*",s["name"]) or s["name"] in names:
            raise ValueError("duplicate/invalid service")
        ids.add(s["id"]);names.add(s["name"])
        deps=s["dependencies"]
        if not isinstance(deps,list) or len(deps)>8 or any(not number(d) for d in deps) or len(set(deps))!=len(deps):
            raise ValueError("invalid dependencies")
        if type(s["required"]) is not bool or type(s["supported"]) is not bool:
            raise ValueError("invalid readiness")
        if s["required"] and not s["supported"]:
            raise ValueError("unsupported mandatory service")
        graph[s["id"]]=set(deps)
    if not PROFILES[profile]<={s["name"] for s in services if s["required"] and s["supported"]}:
        raise ValueError("profile mandatory services missing")
    support={s["id"]:s["supported"] for s in services}
    for s in services:
        if not graph[s["id"]]<=ids or (s["supported"] and any(not support[d] for d in graph[s["id"]])):
            raise ValueError("missing or unsupported dependency")
    pending=dict(graph);ordered=[]
    while pending:
        ready=sorted(k for k,v in pending.items() if v<=set(ordered))
        if not ready:
            raise ValueError("cyclic dependencies")
        for k in ready:
            ordered.append(k);del pending[k]
    resources=m.get("resources")
    if not isinstance(resources,list) or not resources:
        raise ValueError("missing resource inventory")
    seen=set()
    for resource in resources:
        name=resource["name"]
        if not isinstance(name,str) or not name or name in seen:
            raise ValueError("duplicate resource ownership")
        seen.add(name)
        owners=resource["owners"]
        if set(owners)!=PHASES or any(not isinstance(v,str) or not v for v in owners.values()):
            raise ValueError("incomplete phase ownership")
        if owners["run"] not in names:
            raise ValueError("unknown runtime owner")
    features=m.get("features")
    if not isinstance(features,dict) or any(type(v) is not bool for v in features.values()):
        raise ValueError("missing feature resolution")
    for feature in ("uart","can","ipc","watchdog","flash","mcuboot","pm","smp","zbus"):
        if feature not in features:
            raise ValueError("undeclared feature "+feature)
    validate_execution(m)
    if features["mcuboot"] and not features["flash"]:
        raise ValueError("MCUboot requires flash")
    if profile=="integrated" and not features["ipc"]:
        raise ValueError("integrated profile needs IPC")
    if m.get("uart_console_shared",False) and features["uart"]:
        raise ValueError("UART ownership conflicts with console")
    tasks=m.get("tasks")
    if not isinstance(tasks,list) or not tasks:
        raise ValueError("missing scheduling inventory")
    task_names=set()
    for t in tasks:
        if t["name"] in task_names or t["context"] not in CONTEXTS:
            raise ValueError("task identity/context")
        task_names.add(t["name"])
        for k in ("period_us","deadline_us","execution_us","stack_bytes"):
            if not number(t[k]):
                raise ValueError("missing positive task budget")
        if not number(t["blocking_us"],0) or not number(t["jitter_us"],0) or not number(t["priority"],0,255):
            raise ValueError("invalid scheduling allocation")
        if t["context"]=="I0" and t["blocking_us"]:
            raise ValueError("ISR cannot block")
    budgets=m.get("budgets",{})
    for k in ("ram_bytes","flash_bytes","core_queue","bulk_queue"):
        if not number(budgets.get(k)):
            raise ValueError("missing resource budget "+k)
    if sum(t["stack_bytes"] for t in tasks)>=budgets["ram_bytes"]:
        raise ValueError("stack allocation exceeds RAM budget")
    runtime=m.get("benign_runtime")
    if runtime is not None:
        if profile!="edge" or m["board"]!="native_sim/native/64" or not isinstance(runtime,dict):
            raise ValueError("benign runtime is restricted to the simulator edge profile")
        fields=("endpoint_id","period_ms","deadline_ms","feedback_max_age_ms","health_limit_ms","max_validity_ms")
        if set(runtime)!=set(fields) or any(not number(runtime[k]) for k in fields):
            raise ValueError("invalid benign runtime allocation")
        if runtime["deadline_ms"]>runtime["period_ms"] or min(runtime["health_limit_ms"],runtime["feedback_max_age_ms"])<=runtime["period_ms"]:
            raise ValueError("invalid reference timing relationship")
        if len(tasks)!=2 or tasks[1]["name"]!="virtual_device" or tasks[1]["context"]!="R2" or tasks[1]["priority"]<=tasks[0]["priority"] or tasks[0]["name"]!="controller_owner" or tasks[0]["context"]!="R1" or tasks[0]["period_us"]!=runtime["period_ms"]*1000 or tasks[0]["deadline_us"]!=runtime["deadline_ms"]*1000:
            raise ValueError("runtime/task allocation mismatch")
        if len(services)!=2 or {s["id"]:s["name"] for s in services}!={1:"core",2:"health"} or ordered!=[1,2]:
            raise ValueError("reference composition mismatch")
        if any(v!=(k in {"zbus","pm","flash"}) for k,v in features.items()):
            raise ValueError("reference runtime requires simulated Zbus, device PM and NVS only")
    profile_runtime=m.get("profile_runtime")
    if profile_runtime is not None:
        if runtime is not None or profile=="edge" or m["board"]!="native_sim/native/64" or not isinstance(profile_runtime,dict):
            raise ValueError("profile runtime restricted to non-edge simulator fixtures")
        fields={"endpoint_ids","period_ms","deadline_ms","feedback_max_age_ms","health_limit_ms","max_validity_ms"}
        ids=profile_runtime.get("endpoint_ids")
        if set(profile_runtime)!=fields or not isinstance(ids,list) or not 1<=len(ids)<=4 or any(not number(x) for x in ids) or len(set(ids))!=len(ids):
            raise ValueError("invalid profile endpoints")
        if any(not number(profile_runtime[k]) for k in fields-{"endpoint_ids"}):
            raise ValueError("invalid profile timing")
        if profile_runtime["deadline_ms"]>profile_runtime["period_ms"] or min(profile_runtime["health_limit_ms"],profile_runtime["feedback_max_age_ms"])<=profile_runtime["period_ms"]:
            raise ValueError("invalid profile timing relationship")
        if len(tasks)!=2 or tasks[1]["name"]!="virtual_device" or tasks[1]["context"]!="R2" or tasks[1]["priority"]<=tasks[0]["priority"] or tasks[0]["name"]!="controller_owner" or tasks[0]["context"]!="R1" or tasks[0]["period_us"]!=profile_runtime["period_ms"]*1000 or tasks[0]["deadline_us"]!=profile_runtime["deadline_ms"]*1000:
            raise ValueError("profile/task allocation mismatch")
        expected={"companion":["core","health","power","broker"],"zonal":["core","health","endpoint"],"integrated":["core","health","ipc"]}[profile]
        if len(services)!=len(expected) or {s["id"]:s["name"] for s in services}!={i+1:name for i,name in enumerate(expected)} or ordered!=list(range(1,len(expected)+1)):
            raise ValueError("profile composition mismatch")
        if any(v!=(k in {"zbus","pm","flash"} or (k=="ipc" and profile=="integrated")) for k,v in features.items()):
            raise ValueError("unsupported simulator profile feature")
        if len(resources)!=len(ids):
            raise ValueError("simulator resource allocation mismatch")
    limitations=m.get("limitations")
    if not isinstance(limitations,list) or not limitations or any(not isinstance(v,str) or not v for v in limitations):
        raise ValueError("explicit limitations required")
    return ordered

def reconcile_kconfig(m,path):
    values={}
    for line in path.read_text().splitlines():
        if line.startswith("CONFIG_") and "=" in line:
            k,v=line.split("=",1);values[k]=v=="y"
    mapping={"uart":"CONFIG_UART_ASYNC_API","can":"CONFIG_CAN","ipc":"CONFIG_IPC_SERVICE",
             "watchdog":"CONFIG_WATCHDOG","flash":"CONFIG_NVS","mcuboot":"CONFIG_MCUBOOT_IMG_MANAGER",
             "pm":"CONFIG_PM","smp":"CONFIG_SMP","zbus":"CONFIG_ZBUS"}
    for feature,k in mapping.items():
        if m["features"][feature]!=(values.get(k,False) or (feature=="pm" and values.get("CONFIG_PM_DEVICE_RUNTIME",False))):
            raise ValueError("resolved Kconfig conflicts with product: "+k)

def generate(m):
    ordered=validate(m)
    canonical=json.dumps(m,sort_keys=True,separators=(",",":"),ensure_ascii=True).encode()
    digest=hashlib.sha256(canonical).hexdigest()
    runtime_macros=""
    if "benign_runtime" in m or "profile_runtime" in m:
        runtime=m.get("benign_runtime",m.get("profile_runtime"))
        ids=runtime.get("endpoint_ids",[runtime.get("endpoint_id")])
        kind={"edge":0,"companion":1,"zonal":2,"integrated":3}[m["profile"]]
        runtime_macros+=f"#define ZARC_PROFILE_KIND {kind}u\n"
        runtime_macros+=f'#define ZARC_PROFILE_NAME "{m["profile"]}"\n'
        runtime_macros+=f"#define ZARC_PROFILE_ENDPOINT_COUNT {len(ids)}u\n"
        runtime_macros+="#define ZARC_PROFILE_ENDPOINTS {"+",".join("{"+str(id)+"u,"+str(runtime["feedback_max_age_ms"])+"u}" for id in ids)+"}\n"
        for key,value in runtime.items():
            if key=="endpoint_ids":continue
            runtime_macros+=f"#define ZARC_RUNTIME_{key.upper()} {value}u\n"
        runtime_macros+=f'#define ZARC_RUNTIME_MAIN_STACK {m["tasks"][0]["stack_bytes"]}u\n'
        runtime_macros+=f'#define ZARC_RUNTIME_MAIN_PRIORITY {m["tasks"][0]["priority"]}u\n'
        runtime_macros+=f'#define ZARC_RUNTIME_WORKER_STACK {m["tasks"][1]["stack_bytes"]}u\n'
        runtime_macros+=f'#define ZARC_RUNTIME_WORKER_PRIORITY {m["tasks"][1]["priority"]}u\n'
        runtime_macros+=f"#define ZARC_RUNTIME_CONFIG_DIGEST UINT64_C(0x{digest[:16]})\n"
    return ("#ifndef ZARC_GENERATED_MANIFEST_H\n#define ZARC_GENERATED_MANIFEST_H\n"
            "#include <stdint.h>\n"
            f'#define ZARC_MANIFEST_SHA256 "{digest}"\n'
            f'#define ZARC_MANIFEST_SERVICE_COUNT {len(ordered)}u\n'
            +runtime_macros+
            "static const uint32_t zarc_manifest_service_order[] = {"+
            ",".join(str(x)+"u" for x in ordered)+"};\n#endif\n")

def main():
    p=argparse.ArgumentParser()
    p.add_argument("manifest",type=Path,nargs="?")
    p.add_argument("--kconfig",type=Path)
    p.add_argument("--header",type=Path)
    p.add_argument("--compare-lifecycle",nargs=2,type=Path,metavar=("BEFORE","AFTER"))
    args=p.parse_args()
    try:
        if args.compare_lifecycle:
            if args.manifest or args.kconfig or args.header:
                p.error("--compare-lifecycle cannot be combined with manifest generation options")
            old_manifest,new_manifest=(json.loads(path.read_text(encoding="utf-8"))
                                       for path in args.compare_lifecycle)
            if not isinstance(old_manifest,dict) or not isinstance(new_manifest,dict):
                raise ValueError("target manifests must be objects")
            result=hardware_lifecycle_delta(old_manifest.get("hardware_lifecycle"),
                                            new_manifest.get("hardware_lifecycle"))
            print(json.dumps(result,indent=2,sort_keys=True))
            return 2 if result["verdict"]=="REVIEW_REQUIRED" else 0
        if args.manifest is None:
            p.error("manifest is required unless --compare-lifecycle is used")
        m=json.loads(args.manifest.read_text(encoding="utf-8"))
        header=generate(m)
        if args.kconfig:reconcile_kconfig(m,args.kconfig)
        if args.header:
            args.header.parent.mkdir(parents=True,exist_ok=True)
            args.header.write_text(header,encoding="utf-8")
        print("Candidate composition admitted; no target support claim")
    except (KeyError,TypeError,ValueError,OSError) as e:
        print("Rejected:",e)
        return 1
    return 0

if __name__=="__main__":raise SystemExit(main())
