# R13-D01 independent review evidence

Run from the repository root:

```sh
python3 tests/native/r13d01-review-evidence/audit.py
```

This is a read-only candidate/evidence inspection; it executes no builds or
product tests. `audit.json` records the successful result. The checker verifies
the exact one-line replacement, full 13-file candidate identity/live hashes,
12 unchanged candidate peers, all eight coder evidence entries, all 107 frozen
prior evidence files, both original evidence manifests, both frozen CTest
registries and every passing complete-gate status, and existing documentation
link targets. `initial-audit.py` and `initial-audit.log` preserve the initial
checker whitespace issue. Product files and previous evidence remain unchanged.

`evidence.json` maps SHA256 for files in this directory, relative to this directory,
excluding itself. The review report is outside this manifest and receives a
separate delivery hash. No runtime/platform coverage beyond the earlier frozen
macOS gates is claimed.
