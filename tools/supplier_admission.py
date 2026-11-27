"""Candidate supplier admission checks. Acceptance requires independent review."""
import argparse
import json
from pathlib import Path
import re

def validate(record):
    errors=[]
    if not isinstance(record,dict):
        return ["record must be an object"]
    for name in ("supplier","component","version","maintainer","support_policy","change_policy","license",
                 "api_contract","resource_budget","limitations","evidence_reference"):
        if not isinstance(record.get(name),str) or not record[name].strip():
            errors.append("missing "+name)
    if not re.fullmatch(r"[0-9a-f]{40}",str(record.get("revision",""))):
        errors.append("immutable source revision required")
    if record.get("status")!="CANDIDATE":
        errors.append("this tool does not authorize supported or qualified status")
    dependencies=record.get("dependencies")
    if not isinstance(dependencies,list):
        errors.append("dependency list required")
    else:
        seen=set()
        for item in dependencies:
            if not isinstance(item,dict):
                errors.append("invalid dependency")
                continue
            name=item.get("name")
            if not isinstance(name,str) or not name.strip() or name in seen:
                errors.append("missing or duplicate dependency name")
            else:
                seen.add(name)
            for key in ("version","license","review_reference"):
                if not isinstance(item.get(key),str) or not item[key].strip():
                    errors.append("dependency missing "+key)
    return errors

def main():
    p=argparse.ArgumentParser()
    p.add_argument("submission",type=Path)
    a=p.parse_args()
    errors=validate(json.loads(a.submission.read_text(encoding="utf-8")))
    if errors:
        print("\n".join(errors))
        return 1
    print("Candidate submission structurally complete; independent acceptance still required")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
