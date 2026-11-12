import re, sys
H = re.compile(r"[0-9a-fA-F]{64}")
T = re.compile(r"REPORT|-review[.]md|roadmap[.]md|rounds/R19-ACCEPT-A[.]md|R19-ACCEPT-A[.]md")
for path in open(sys.argv[1]).read().split():
    if "/index/" in path: continue
    try: lines = open(path, encoding="utf-8").read().splitlines()
    except Exception: continue
    for i, l in enumerate(lines):
        if not H.search(l): continue
        win = lines[max(0,i-1):i+2]
        if any(T.search(w) for w in win):
            print(f"{path}:{i+1}: {l[:160]}")
