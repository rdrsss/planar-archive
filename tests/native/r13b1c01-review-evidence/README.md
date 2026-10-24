# R13-B1-C01 independent review evidence

Reviewer `/root/r13b1c01_reviewer`; shared checkout
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Only this directory and `docs/vernacular/rounds/R13-B1-C01-review.md` were written.
No product edits, Planar tools, operator data, commits or deployment actions.
Debug checks were direct, serialized and exclusively owned; directory released
on delivery.

`focused-command.json` records exact CTest argv and exit. `focused.log` and
`focused.xml` retain the independent 79-case, zero-failure/skip run.
`checks-command.json` records audit, both vector checks, active lint, final
no-work build and real-fault snapshot run, all exit zero. `helper-lint-command.json`
records the supplementary two-file check. `build.log`/`snapshot.log` retain the
initial quick no-work build and 59-assertion snapshot pass; final repetitions
are `build-final.log`/`snapshot-final.log` with explicit command records.

Reproduce the independent read-only audit with:

```sh
python3 tests/native/r13b1c01-review-evidence/audit.py
```

It verifies the eleven live candidate files and snapshots, candidate identity,
48 correction evidence entries, 50 original B1 evidence entries, 16 original
review entries, all 1,834 protected files and both distinct baseline diffs.
It freshly generates C2/B1 variants only in temporary directories and compares
them to configured bytes/manifests, checks two redirected digest sites and the
normal non-mutated variant. It audits independent JUnit and retained real-fault
RED/GREEN prerequisites. Initial and final logs both pass. It never rewrites
prior evidence. Candidate keys are repository-relative; all evidence manifest
keys here are relative to this folder. `evidence.json` hashes every retained
artifact except itself. The review report sits outside that manifest and has a
separate delivery hash.

Host: macOS Apple Silicon debug. Full integrated R13 debug/release, Linux, B2
trees, provenance and R14 acceptance are outside this review's executed coverage.
