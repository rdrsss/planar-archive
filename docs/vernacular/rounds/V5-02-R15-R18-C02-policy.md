# R15–R18 C02 policy corrective

Status: implemented; focused verification passes; awaiting independent review.
This is coder evidence, not approval. Scope follows the two findings PR1 and PR2
in [the C01 policy/runtime review](V5-02-R15-R18-C01-review-policy-runtime.md).
The shared checkout starts at `ae02a12bf77c6b72d3060253dd47800c645796a7`
with the C01 candidate and other agents' changes retained.

## Changes

PR1: `checks.material` requires each current worker's actual state to be `done`
before admitting review. Active runs retain precedence even when an older
completion becomes applicable again. Dependency quality `done` retains its
separate completion-applicability semantics. The coordinator extended ownership
to `reconcile.lua`: load now projects worker states before reviewer readiness,
updates the hypothetical view, and includes newly added workers. This allows
inactive restored completions to unblock review in the same load, without
inferring worker states in ordinary material reads. Role declaration order does
not affect this projection.

PR2: `checks.repair_revision` supplies the same snapshot-derived role repair
revision to work calculation, dispatch contexts and recovery. Dispatch manifest
provenance reads the repair value of the actual hashed work object, so preview,
claim, refresh and takeover cannot describe a different repair revision.

Extended takeover coverage exposed a pre-existing acquisition protocol defect:
`acquire_command` copied the old target run ID into the proposed next vector,
but takeover allocates a new run and native admission requires `run="none"`
for that target. The coordinator authorized the narrow correction in the owned
commands file. Other roles retain their existing run IDs. The failure was
reproduced before this edit (`red-takeover.log`); LLDB isolated the refusal to
`check_snapshot`'s vector equality check in `execution.cpp` (line 3416 at the
candidate). No debug instrumentation was added to product files.

The new CLI fixture declares the reviewer before the worker and covers active
restoration (blocked reviewer and rejected claim), inactive restoration (done
worker and claimable reviewer), and repaired work identities/provenance across
preview, recover, claim, refresh and takeover. Existing actual-Lua regressions
gain active-run/evidence-only rederivation, inactive restoration, and independent
`done` dependency-quality checks.

## Verification

Raw output: `tests/native/r15-r18-c02-evidence/policy/`.

- Before product edits, `python3 tests/native/r15-r18-c02-policy-cli.py`:
  2 tests, 2 expected failures, 0 skips (`red-cli.log`). Native restoration
  incorrectly yielded `reviewer=ready` while `worker=running,drifted=true`;
  repaired recovery disagreed with preview. This was the initial two-case
  fixture, before adding inactive restoration and later lifecycle assertions.
- Before product edits, `lua tests/native/r15-r18-c01-policy.lua`:
  14 tests, 1 expected failure, 0 skips (`red-lua.log`), on the active worker's
  retained completion admitting review.
- After edits, `lua tests/native/r15-r18-c01-policy.lua`:
  15 tests, 15 pass, 0 failures, 0 skips (`green-lua.log`).
- `python3 -m py_compile tests/native/r15-r18-c02-policy-cli.py`: pass.
- `cmake --build --preset debug --target planar`: pass (`build.log`), under the
  coordinator's exclusive Debug slot; no parallel build in this directory.
- `python3 tests/native/r15-r18-c02-policy-cli.py`: 3 scenarios, 3 pass,
  0 failures, 0 skips (`green-cli.log`), including the additional takeover fix.
- `python3 tests/native/r15-r18-c01-policy-cli.py --binary build/rewrite-debug/bin/planar`:
  3 scenarios, 3 pass, 0 failures, 0 skips (`c01-cli.log`). An initial invocation
  omitted the mandatory `--binary` and exited with argparse usage; the corrected
  invocation is the verification result.
- `python3 tests/native/r15-r18-c01-runtime.py`: 5 checks, 5 pass,
  0 failures (`c01-runtime.log`).
- Scoped `git diff --check`: pass.

`candidate.json` pins SHA256 for the four product Lua files, both policy test
files and the tested Debug executable. The coordinator registered the new CLI
suite in CTest. The Debug slot was returned immediately after these checks.

All CLI exercises use explicit source-built candidate paths and fresh temporary
roots/ledgers. Tokens are kept inside the process and are not logged. No installed
binary, operator database, installation, commit, publish or merge was used.
Full gate, release and Linux validation remain coordinator-owned.
