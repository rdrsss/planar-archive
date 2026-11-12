import sys, subprocess, re
from pathlib import Path
sys.path.insert(0, "tests/native/r19-accept-support")
sys.dont_write_bytecode = True
import run_acceptance as ra
roots = sys.argv[1:]
out = []
for r in roots:
    p = Path(r)
    files = [p] if p.is_file() else [f for f in p.rglob("*") if f.is_file()]
    for f in files:
        rel = f.as_posix()
        if ra.commit_list_path_rules(rel):
            continue
        out.append(rel)
for x in sorted(set(out)):
    print(x)
