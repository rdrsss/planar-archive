# R14-D1 candidate evidence

Candidate identity and owned source hashes are in `candidate.json`; `candidate/`
contains exact reviewable files and `baseline/` preserves prior owned files.
`before-protected.json` records 4,828 untouched files. Coordinator planning records
are excluded from that manifest. `audit.py` proves exact private helper extraction,
render-only call/import changes, protected files, exact generated variants,
retained case inventory and the actual native path test link.

The complete debug gate passes 309 registered cases with zero skips, checks
74 active C++ source files with the pinned formatter/Doxygen and rebuilds with no
work. This includes all retained 151 history, 25 capture, 12 prepared and 12 atomic
cases, plus eight new host/intent path cases. `cli-complete.log` pins 247 processes
and 60 host-path checks; all prior goldens and canonical wire checks remain active.
`vectors.log` preserves the independent vector check. `report.json` lists commands,
counts, preliminary failures and platform limits. Final source edits precede all
passing gate evidence; subsequent changes are evidence-only.

Preliminary build/path/CLI logs are preserved honestly: incorrect new expectations
were corrected to existing resolver behavior, and the exact 64 MiB root-join test
caught and corrected conservative separator counting in the new pure normalizer.
There was no change to the extracted host algorithms. The normalizer is deliberately
unused by render and adds no load policy or service.

This is a coder candidate, awaiting independent review. No installation, commit,
publishing, operator-state mutation or Planar tooling occurred.
