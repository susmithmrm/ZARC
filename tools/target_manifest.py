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
def number(x,minimum=1,maximum=0xffffffff):
    return type(x) is int and minimum<=x<=maximum

def validate(m):
    if not isinstance(m,dict):
        raise ValueError("manifest object")
    for key in ("target","board","soc","revision","zephyr","toolchain","maintainer"):
        if not isinstance(m.get(key),str) or not m[key].strip():
            raise ValueError("missing "+key)
    if m.get("schema")!=1 or m.get("api_major")!=1 or m.get("maturity")!="CANDIDATE":
        raise ValueError("only schema/API 1 and candidate scope admitted")
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
        if len(tasks)!=1 or tasks[0]["name"]!="controller_owner" or tasks[0]["context"]!="R1" or tasks[0]["period_us"]!=runtime["period_ms"]*1000 or tasks[0]["deadline_us"]!=runtime["deadline_ms"]*1000:
            raise ValueError("runtime/task allocation mismatch")
        if len(services)!=2 or {s["id"]:s["name"] for s in services}!={1:"core",2:"health"} or ordered!=[1,2]:
            raise ValueError("reference composition mismatch")
        if any(features.values()):
            raise ValueError("reference runtime has no native transport or physical driver")
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
        if m["features"][feature]!=values.get(k,False):
            raise ValueError("resolved Kconfig conflicts with product: "+k)

def generate(m):
    ordered=validate(m)
    canonical=json.dumps(m,sort_keys=True,separators=(",",":"),ensure_ascii=True).encode()
    digest=hashlib.sha256(canonical).hexdigest()
    runtime_macros=""
    if "benign_runtime" in m:
        for key,value in m["benign_runtime"].items():
            runtime_macros+=f"#define ZARC_RUNTIME_{key.upper()} {value}u\n"
        runtime_macros+=f'#define ZARC_RUNTIME_MAIN_STACK {m["tasks"][0]["stack_bytes"]}u\n'
        runtime_macros+=f'#define ZARC_RUNTIME_MAIN_PRIORITY {m["tasks"][0]["priority"]}u\n'
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
    p.add_argument("manifest",type=Path)
    p.add_argument("--kconfig",type=Path)
    p.add_argument("--header",type=Path)
    args=p.parse_args()
    try:
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
