"""Conservative requirement change-impact inventory, not ABI compatibility approval."""
import argparse
import csv
import json
from pathlib import Path
import subprocess

DEPENDENCY_FIELDS = ("name", "version", "source", "revision", "license")

def impact(rows,paths):
    by_path={}
    ids=set()
    for row in rows:
        rid=row["Requirement"];ids.add(rid)
        for field in ("Candidate_code","Candidate_tests"):
            for path in filter(None,row[field].split(";")):
                by_path.setdefault(path,set()).add(rid)
    direct=set();unmapped=[]
    for path in sorted(set(paths)):
        if path in by_path:direct.update(by_path[path])
        else:unmapped.append(path)
    # Even mapped implementation edits can have transitive effects. This tool
    # never declares unaffected requirements accepted or compatibility proven.
    return dict(direct=sorted(direct),unmapped=unmapped,
                review_scope=sorted(ids if unmapped else direct),
                compatibility="REVIEW_REQUIRED")

def _dependency_map(items):
    if not isinstance(items,list):
        raise ValueError("dependency inventory must be an array")
    result={}
    for index,item in enumerate(items):
        if not isinstance(item,dict):
            raise ValueError(f"dependency {index} must be an object")
        missing=[field for field in DEPENDENCY_FIELDS
                 if not isinstance(item.get(field),str) or not item[field].strip()]
        if missing:
            raise ValueError(f"dependency {index} missing {', '.join(missing)}")
        name=item["name"].strip()
        if name in result:
            raise ValueError(f"duplicate dependency name: {name}")
        result[name]={field:item[field].strip() for field in DEPENDENCY_FIELDS}
    return result

def dependency_changes(before,after):
    """Return exact dependency additions, removals and metadata changes."""
    old=_dependency_map(before)
    new=_dependency_map(after)
    changes=[]
    for name in sorted(set(old)|set(new)):
        previous,current=old.get(name),new.get(name)
        if previous==current:
            continue
        if previous is None:
            kind="ADDED"
        elif current is None:
            kind="REMOVED"
        else:
            kind="UPDATED"
        changes.append({"name":name,"change":kind,"before":previous,"after":current})
    return changes

def _review_key(item):
    return json.dumps({"name":item.get("name"),"change":item.get("change"),
        "before":item.get("before"),"after":item.get("after")},sort_keys=True,separators=(",",":"))

def validate_dependency_reviews(changes,reviews,inventory_status):
    """Check structural review coverage without authenticating approval evidence."""
    errors=[]
    if inventory_status not in ("CANDIDATE","INCOMPLETE","APPROVED"):
        errors.append("dependency inventory status must be CANDIDATE, INCOMPLETE or APPROVED")
    if not isinstance(changes,list) or not isinstance(reviews,list):
        errors.append("dependency changes and reviews must be arrays")
        changes=[] if not isinstance(changes,list) else changes
        reviews=[] if not isinstance(reviews,list) else reviews
    if not changes:
        if reviews:
            errors.append("review record supplied without a dependency change")
        return errors,{"gate_status":"NO_DEPENDENCY_CHANGE" if not errors else "BLOCKED",
                       "change_count":0,"review_records":0,"approval_asserted":False}
    if inventory_status!="APPROVED":
        errors.append("approved dependency inventory required for release review")

    expected={}
    for index,change in enumerate(changes):
        if not isinstance(change,dict) or not change.get("name") or change.get("change") not in ("ADDED","REMOVED","UPDATED"):
            errors.append(f"dependency change {index} is malformed")
            continue
        key=_review_key(change)
        if key in expected:
            errors.append(f"duplicate dependency change: {change['name']}")
        expected[key]=change

    actual={}
    for index,review in enumerate(reviews):
        if not isinstance(review,dict):
            errors.append(f"dependency review {index} must be an object")
            continue
        key=_review_key(review)
        if key in actual:
            errors.append(f"duplicate dependency review: {review.get('name','')}")
        actual[key]=review
        if key not in expected:
            errors.append(f"unexpected or mismatched dependency review: {review.get('name','')}")

    for key,change in expected.items():
        review=actual.get(key)
        if review is None:
            errors.append(f"missing dependency review: {change['name']}")
            continue
        for field in ("compatibility","support","license"):
            if review.get(field)!="ACCEPTED":
                errors.append(f"{change['name']}: {field} review must be ACCEPTED")
        for field in ("reviewer","evidence_reference"):
            if not isinstance(review.get(field),str) or not review[field].strip():
                errors.append(f"{change['name']}: {field} required")
        if review.get("decision")!="ACCEPTED":
            errors.append(f"{change['name']}: explicit ACCEPTED decision required")

    status="BLOCKED" if errors else "REVIEW_RECORDS_COMPLETE"
    return errors,{"gate_status":status,"change_count":len(changes),
        "review_records":len(reviews),"inventory_status":inventory_status,
        "approval_asserted":False}

def _read_json(path):
    return json.loads(path.read_text(encoding="utf-8"))

def main(argv=None):
    p=argparse.ArgumentParser()
    p.add_argument("--base",required=True)
    p.add_argument("--head",default="HEAD")
    p.add_argument("--dependency-before",type=Path,help="JSON array of dependency records before the change")
    p.add_argument("--dependency-after",type=Path,help="JSON array of dependency records after the change")
    p.add_argument("--dependency-reviews",type=Path,help="JSON array of matched compatibility/support/license reviews")
    p.add_argument("--dependency-inventory-status",choices=("CANDIDATE","INCOMPLETE","APPROVED"),
                   help="inventory status supplied by its responsible reviewer")
    a=p.parse_args(argv)
    root=Path(__file__).resolve().parents[1]
    def revision(ref):
        return subprocess.check_output(["git","-C",str(root),"rev-parse","--verify",ref+"^{commit}"],text=True).strip()
    base,head=revision(a.base),revision(a.head)
    names=subprocess.check_output(["git","-C",str(root),"diff","--no-renames","--name-only","-z",base,head,"--"]).decode().split("\0")
    with (root/"docs/REQUIREMENT-TRACE.csv").open(newline="",encoding="utf-8") as f:
        result=impact(csv.DictReader(f),filter(None,names))
    result.update(base=base,head=head,scope="review transitive dependencies and version decisions; no ABI approval inferred")
    dependency_args=(a.dependency_before,a.dependency_after,a.dependency_reviews,a.dependency_inventory_status)
    if any(value is not None for value in dependency_args):
        if any(value is None for value in dependency_args):
            raise SystemExit("all dependency review inputs and inventory status are required together")
        try:
            before=_read_json(a.dependency_before)
            after=_read_json(a.dependency_after)
            reviews=_read_json(a.dependency_reviews)
            changes=dependency_changes(before,after)
            errors,report=validate_dependency_reviews(changes,reviews,a.dependency_inventory_status)
        except (OSError,ValueError,TypeError) as exc:
            raise SystemExit(f"invalid dependency review input: {exc}") from exc
        result["dependency_review"]={**report,"changes":changes,"errors":errors}
        if errors:
            print(json.dumps(result,indent=2))
            return 1
    print(json.dumps(result,indent=2))
    return 0
if __name__=="__main__":raise SystemExit(main())
