"""Fail-closed structural evidence gate. Not a substitute for engineering review."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]

def read_csv(path):
    with path.open(encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))

def trace_errors(root):
    baseline = read_csv(root / "docs/architecture/12-Requirements-Verification.csv")
    trace = read_csv(root / "docs/REQUIREMENT-TRACE.csv")
    base = {r["ID"]: r for r in baseline}
    records = {r["Requirement"]: r for r in trace}
    errors = []
    if len(base) != len(baseline) or len(records) != len(trace) or set(base) != set(records):
        errors.append("trace must contain exactly the unique baseline requirements")
    for rid, row in records.items():
        if rid not in base:
            continue
        b = base[rid]
        if (row["Design"],row["Verification"],row["Method"],row["Acceptance"]) != (
                b["Design_ref"],b["Verification_ID"],b["Method"],b["Acceptance"]):
            errors.append(f"{rid}: baseline mismatch")
        if row["Status"] not in {"OPEN","PARTIAL_OPEN"} or not row["Open_evidence"]:
            errors.append(f"{rid}: unsupported status or missing gap")
        for field in ("Candidate_code","Candidate_tests"):
            for value in filter(None,row[field].split(";")):
                candidate = (root / value).resolve()
                if not candidate.is_relative_to(root.resolve()) or not candidate.is_file():
                    errors.append(f"{rid}: unresolved path {value}")
    return base, errors

def evidence_errors(base, record, directory, configuration, commit):
    errors = []
    if not re.fullmatch(r"[0-9a-f]{40}",commit):
        errors.append("invalid commit identity")
    if record.get("configuration") != configuration or record.get("commit") != commit:
        errors.append("configuration or commit mismatch")
    for key in ("manifest_sha256","toolchain","reviewer","approval_record"):
        if not isinstance(record.get(key),str) or not record[key].strip():
            errors.append(f"missing {key}")
    if not re.fullmatch(r"[0-9a-f]{64}",str(record.get("manifest_sha256",""))):
        errors.append("invalid manifest digest")
    manifest_path = record.get("manifest_artifact","")
    if not isinstance(manifest_path,str) or not manifest_path:
        errors.append("missing configuration manifest artifact")
    else:
        p = (directory / manifest_path).resolve()
        if not p.is_relative_to(directory.resolve()) or not p.is_file():
            errors.append("missing configuration manifest artifact")
        elif hashlib.sha256(p.read_bytes()).hexdigest() != record.get("manifest_sha256"):
            errors.append("configuration manifest hash mismatch")
    items = record.get("requirements",[])
    if not isinstance(items,list) or any(not isinstance(x,dict) for x in items):
        return errors+["invalid requirements collection"]
    by_id = {x.get("id"):x for x in items}
    if len(items)!=len(by_id) or set(by_id)!=set(base):
        errors.append("evidence must address each baseline requirement exactly once")
    for rid,row in by_id.items():
        if rid not in base:
            continue
        if row.get("verification") != base[rid]["Verification_ID"] or row.get("method") != base[rid]["Method"]:
            errors.append(f"{rid}: wrong verification identity or method")
        if row.get("applicability") == "NOT_APPLICABLE":
            if base[rid]["Profiles"] == "ALL":
                errors.append(f"{rid}: mandatory ALL requirement cannot be excluded")
            if not row.get("rationale") or not row.get("reviewer") or not row.get("approval"):
                errors.append(f"{rid}: unreviewed exclusion")
            continue
        if row.get("applicability") != "APPLICABLE" or row.get("verdict") != "PASS":
            errors.append(f"{rid}: missing passing evidence")
        for key in ("environment","procedure","expected","actual","reviewer","approval"):
            if not isinstance(row.get(key),str) or not row[key].strip():
                errors.append(f"{rid}: missing {key}")
        path = row.get("artifact","")
        digest = row.get("sha256","")
        if not isinstance(path,str) or not isinstance(digest,str):
            errors.append(f"{rid}: invalid artifact reference")
            continue
        p = (directory / path).resolve()
        if not path or not p.is_relative_to(directory.resolve()) or not p.is_file():
            errors.append(f"{rid}: missing raw artifact")
            continue
        if hashlib.sha256(p.read_bytes()).hexdigest()!=digest:
            errors.append(f"{rid}: artifact hash mismatch")
    return errors

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--trace-only",action="store_true")
    parser.add_argument("--evidence",type=Path)
    parser.add_argument("--configuration")
    parser.add_argument("--commit")
    args=parser.parse_args()
    baseline, errors=trace_errors(ROOT)
    if not args.trace_only:
        if not args.evidence or not args.configuration or not args.commit:
            errors.append("release requires evidence file, configuration and exact commit")
        else:
            try:
                record=json.loads(args.evidence.read_text(encoding="utf-8"))
                errors.extend(evidence_errors(baseline,record,args.evidence.resolve().parent,args.configuration,args.commit))
            except (ValueError,OSError,TypeError,AttributeError) as e:
                errors.append(f"invalid evidence: {e}")
    if errors:
        print("\n".join(errors))
        return 1
    print("Trace complete; no product approval asserted" if args.trace_only else
          "Structural evidence gate passed; responsible reviewer must confirm authenticity and acceptance")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
