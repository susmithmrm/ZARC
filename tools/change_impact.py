"""Conservative requirement change-impact inventory, not ABI compatibility approval."""
import argparse
import csv
import json
from pathlib import Path
import subprocess

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
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--base",required=True)
    p.add_argument("--head",default="HEAD")
    a=p.parse_args()
    root=Path(__file__).resolve().parents[1]
    def revision(ref):
        return subprocess.check_output(["git","-C",str(root),"rev-parse","--verify",ref+"^{commit}"],text=True).strip()
    base,head=revision(a.base),revision(a.head)
    names=subprocess.check_output(["git","-C",str(root),"diff","--no-renames","--name-only","-z",base,head,"--"]).decode().split("\0")
    with (root/"docs/REQUIREMENT-TRACE.csv").open(newline="",encoding="utf-8") as f:
        result=impact(csv.DictReader(f),filter(None,names))
    result.update(base=base,head=head,scope="review transitive dependencies and version decisions; no ABI approval inferred")
    print(json.dumps(result,indent=2))
if __name__=="__main__":main()
