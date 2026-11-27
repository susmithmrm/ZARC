"""Collect build provenance; inventory is not a complete dependency SBOM or approval."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import subprocess

def digest(path):
    h=hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda:stream.read(1024*1024),b""):
            h.update(chunk)
    return h.hexdigest()

def file_record(root,path):
    p=path.resolve(strict=True)
    if not p.is_relative_to(root.resolve()) or not p.is_file():
        raise ValueError("artifact outside build root or not a file")
    return {"path":p.relative_to(root.resolve()).as_posix(),"sha256":digest(p),"bytes":p.stat().st_size}

def inventory(root,build_dir,configuration):
    root=root.resolve()
    build_dir=build_dir.resolve()
    if not build_dir.is_relative_to(root):
        raise ValueError("build directory must be in checkout")
    def git(*args):
        return subprocess.check_output(["git","-C",str(root),*args])
    commit=git("rev-parse","HEAD").decode().strip()
    if not re.fullmatch(r"[0-9a-f]{40}",commit):
        raise ValueError("invalid source revision")
    tracked=git("ls-files","-z").decode("utf-8").split("\0")
    source=[file_record(root,root/name) for name in tracked if name]
    artifacts=[]
    for p in sorted(build_dir.rglob("*")):
        if p.is_file() and (p.name in {"CMakeCache.txt",".config","zephyr.dts","zephyr.map","zephyr.elf",
           "zephyr.hex","zephyr.bin","ctest.xml","twister.json","twister.log"}
           or p.name.startswith("CMakeCCompiler.") or p.name.startswith("CMakeCXXCompiler.")
           or p.name.startswith(("zarc_","libzarc_")) and p.suffix in {"",".exe",".lib",".a"}):
            artifacts.append(file_record(root,p))
    if not artifacts:
        raise ValueError("no build outputs found")
    changes=git("status","--porcelain","--untracked-files=no").decode("utf-8")
    # The repository declares text normalization; only CR-at-EOL differences
    # may be tolerated, while actual source bytes are still hashed above.
    diff=subprocess.run(["git","-C",str(root),"diff","--ignore-cr-at-eol","--quiet","HEAD","--"],check=False)
    if diff.returncode not in (0,1):
        raise ValueError("git diff failed")
    return {"schema":1,"configuration":configuration,"commit":commit,
        "tracked_source_clean":diff.returncode==0,"tracked_checkout_status":changes,
        "host":platform.platform(),"python":platform.python_version(),"source":source,"build_artifacts":artifacts,
        "scope":"candidate provenance and tracked-source inventory; external dependency and license review required"}

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--root",type=Path,default=Path.cwd())
    p.add_argument("--build",type=Path,required=True)
    p.add_argument("--configuration",required=True)
    p.add_argument("--out",type=Path,required=True)
    a=p.parse_args()
    record=inventory(a.root,a.build,a.configuration)
    a.out.write_text(json.dumps(record,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    if not record["tracked_source_clean"]:
        raise SystemExit("tracked source modified; evidence not admissible:\n"+record["tracked_checkout_status"])

if __name__=="__main__":
    main()
