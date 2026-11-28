"""Candidate supplier admission checks. Acceptance requires independent review."""
import argparse
import json
from pathlib import Path
import re

def validate_api_lifecycle(policy):
    """Validate an explicitly declared exact-version compatibility policy.

    This checks the declaration and upgrade decisions supplied by the product
    owner. It does not infer semantic compatibility from version strings.
    """
    if not isinstance(policy,dict) or set(policy)!={
            "supported_versions","deprecated_versions","compatible_upgrades"}:
        raise ValueError("invalid API lifecycle policy fields")
    supported=policy["supported_versions"]
    deprecated=policy["deprecated_versions"]
    upgrades=policy["compatible_upgrades"]
    def version_list(values,name):
        if (not isinstance(values,list) or
                any(not isinstance(v,str) or not v.strip() for v in values) or
                len(set(values))!=len(values)):
            raise ValueError("invalid "+name)
    version_list(supported,"supported API versions")
    version_list(deprecated,"deprecated API versions")
    if not supported or not set(deprecated)<=set(supported):
        raise ValueError("deprecated versions must be supported versions")
    if not isinstance(upgrades,list):
        raise ValueError("invalid compatible upgrades")
    seen=set()
    for pair in upgrades:
        if (not isinstance(pair,list) or len(pair)!=2 or
                any(not isinstance(v,str) or not v.strip() for v in pair)):
            raise ValueError("invalid compatible upgrade pair")
        source,target=pair
        if source not in supported or target not in supported:
            raise ValueError("compatible upgrade references unsupported version")
        if source==target or target in deprecated:
            raise ValueError("compatible upgrade has invalid target")
        key=(source,target)
        if key in seen:
            raise ValueError("duplicate compatible upgrade")
        seen.add(key)
    return True

def upgrade_is_compatible(policy,source_version,target_version):
    """Return whether the exact declared upgrade fixture is supported."""
    validate_api_lifecycle(policy)
    if (not isinstance(source_version,str) or not isinstance(target_version,str) or
            not source_version.strip() or not target_version.strip()):
        return False
    return [source_version,target_version] in policy["compatible_upgrades"]

def validate(record):
    errors=[]
    if not isinstance(record,dict):
        return ["record must be an object"]
    for name in ("supplier","component","version","maintainer","support_policy","change_policy","license",
                 "api_contract","resource_budget","limitations","evidence_reference"):
        if not isinstance(record.get(name),str) or not record[name].strip():
            errors.append("missing "+name)
    try:
        validate_api_lifecycle(record.get("api_lifecycle"))
    except ValueError as exc:
        errors.append(str(exc))
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
