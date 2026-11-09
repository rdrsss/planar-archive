# R15–R18 C01 independent policy/runtime review

Disposition: **request-changes**. Reviewed 2026-11-08 against HEAD
`ae02a12bf77c6b72d3060253dd47800c645796a7` plus the candidate in
`tests/native/r15-r18-c01-evidence/plan/candidate-review-1.json`.
The two findings below were reproduced through the native development CLI with
fresh temporary ledgers and roots. No product edits, builds, installed binaries,
legacy tools, operator database access or commits were performed. This report is
the reviewer's only repository write.

## PR1 — P2: require done worker states before admitting a reviewer

Location: `src/stdlib/planar/checks.lua:92-102`, particularly line 100.

`material` discards the `blocked` flag computed by `role_material` and rebuilds
the reviewer prerequisite using only `applicable_completion`. A worker can have
applicable retained evidence while its current run is still active. R18-1 item 1
requires every worker role to be **done**, and EX-4 gives an active run precedence
over applicable completion. The older state-based reviewer check remains in
`dispatch.lua:165-170`, but this hook replaces its result.

Reproduced lifecycle, using the single-worker/reviewer fixture from
`tests/native/r15-r18-c01-runtime.py`:

1. Load revision A, activate, claim worker and complete its passing check.
2. Change the material spec goal from `Draft check smoke` to `Changed goal`,
   load revision B, and claim the now-stale worker.
3. Restore the original goal and load revision A's declarations as revision 3.
4. Read status and claim the reviewer.

Observed status was `worker=running, drifted=true`, `reviewer=ready`; the reviewer
claim then succeeded (run 3). Both load operations and both claims exited zero.
This admits review while the worker still owns an active editing run, contrary
to the specified start prerequisite. The worker's retained A completion is
applicable again, but that does not end its B run.

Fix the reviewer-specific prerequisite to account for worker state, preserving
the separate completion-applicability rule for `done` dependency quality. Add
this restoration/active-run lifecycle to the native regression suite and check
both load and post-check derivation. The reproduction is native, not a synthetic
snapshot inference; scratch driver: `<scratch-root>/c01-review-readiness.py`.

## PR2 — P2: propagate the current repair revision into recovery and manifests

Locations: `src/stdlib/planar/commands.lua:325` and `:93-96`;
`src/stdlib/planar/dispatch.lua:323`.

Recovery calls `work_exec` without a repair argument, which defaults to `"0"`.
Actual dispatch work now uses `checks.current_work`, which reads the role's
repair revision from the snapshot. Consequently recovery computes the wrong
current work identity after the first admitted repair. The same missing context
field makes dispatch manifests record `inputs.repair="0"` even when their work
digest covers repair revision 1. P2-2 requires the manifest's actual repair
revision, P2-3 includes it in W, and R16-5 requires the current W identity.

Reproduced lifecycle: complete the worker, claim the reviewer, submit a current
reject with `repair=["worker"]`, then preview and recover the worker and claim
its repair run. The accepted rejection creates repair revision 1. Results:

```text
preview.work_fp = xxh3_128:b3712e0ba6e0e44cb1bfd1e603d9c16b
recover.current.work_fp = xxh3_128:550893ca03c7d112db9f310131d85d00
preview.manifest.inputs.repair = "0"
claim.manifest.inputs.repair = "0"
```

All commands exited zero. Recovery therefore describes the pre-repair work as
current, and the retained claim manifest misstates the inputs behind its digest.
Use the same snapshot-derived role repair context for recovery, manifest
provenance and dispatch W. Cover preview/recover equality after rejection and
the repair input of claim/refresh manifests. Scratch driver:
`<scratch-root>/c01-review-recovery.py`; its setup is the runtime test fixture through the
first worker claim, followed by a passing check and a reject with empty findings,
the exact observed target/policy and the worker in the repair list.

## Original findings and executed verification

- F2: producer evidence targets are now compared against current producer
  completion outputs before completion, including evaluation retries.
- F3: reviewer verdict applicability is now used by check/load/release paths;
  original immaterial-load and changed/restored-target cases pass the actual
  module regressions. PR1 identifies a separate start-prerequisite defect.
- F4: the hypothetical snapshot now includes own-unit observations in both
  completion and evidence-only modes before blocker clearing and derivation.
- RT1: native evidence/findings forwarding passes actual check/reject/recover.
- RT2: active and abandoned reservations return the exact in-progress shape;
  completed request lookup still passes. The two SQL lookup branches share a
  read transaction.
- RT3: no-follow type inspection precedes the exact `.git` directory exclusion;
  existing portable path policy refuses regular `.git` files, links and special
  files. This matches render's policy, as corrected in the coder's report.

Executed independently, without rebuilding:

| Command | Result |
| --- | --- |
| `lua tests/native/r15-r18-c01-policy.lua` | 12 tests, 12 pass, 0 failures, 0 skips |
| `python3 tests/native/r15-r18-c01-runtime.py` | 5 checks, 5 pass, 0 failures; active/abandoned reservation, completed result, evidence and findings |
| `build/rewrite-debug/src/engine/planar_capture_tree_digest_tests` | 4 cases, 15 assertions pass, 0 skips |
| `python3 <scratch-root>/c01-review-readiness.py` | PR1 reproduced through successful native lifecycle; incorrect reviewer claim succeeds |
| `python3 <scratch-root>/c01-review-recovery.py` | PR2 reproduced through successful native lifecycle; fingerprint mismatch and both incorrect manifest values |

CLI reproduction programs use temporary fixture directories and clean them up.
They never print tokens. The existing development binary used for these runs
has SHA256 `1592b365cdc13632fc3322b95edb3789996929c93e0630ee2bc72a45b2a04207`.
This reviewer did not establish a fresh binary build provenance; both observed
defects also follow directly from the candidate source.

## Scope and candidate identity

Read the repository guide, implementation/roadmap and v5 direction, execution
contracts including R16/R17/R18, original review and C01 corrective reports.
Inspected integrated checks/review/reconcile/dispatch/command policy, native
invocation service adapters, runtime service sequencing and response admission,
bootstrap command/profile routing, binding and replay preludes, root/file slot
admission, and the corrected digest capture. This was a broad integration review,
not merely a diff check of the original findings. No additional concrete defect
was established in token authorization, retry sequencing or profile capability
isolation within the inspected paths.

The following inspected product identities matched the review-1 manifest at
the end of the review:

| File | SHA256 |
| --- | --- |
| `src/stdlib/planar/checks.lua` | `c974be533907cc86d7e32ed6176434ecde85443da9f8c96d70e26ad05b227992` |
| `src/stdlib/planar/review.lua` | `abb96f1edef2210408a1bdf9069f8de9c279714c590f3f39c4d8297c0ba5bf99` |
| `src/stdlib/planar/reconcile.lua` | `f2fe3ab975b27d467d52e4e4a94ee9c496e17c0de77759182841864d742319c1` |
| `src/stdlib/planar/dispatch.lua` | `62096d53f5b51de63069ba33082469c13de5653f2ce4a58c4c586284606a700f` |
| `src/stdlib/planar/commands.lua` | `09e93622002b7b52fa1b01703e5d2f7df61c22fdd9de0e8490cccaefc5c37b85` |
| `src/cmd/planar/invocation.cpp` | `34b97f04becc685363ac925613526931175600a17fe9a2a823e1624cb3e34618` |
| `src/cmd/planar/bootstrap.cpp` | `4058033e61a956d94d7c29a4e7fc4634b6fb2b3d647a68fc2948a741eb7594b9` |
| `src/engine/runtime/runtime.cpp` | `f2b5d5b4d31cb485ae6696d8f7ea42fb81386b4fa4ae09e69fc1aa00fd0a81bd` |
| `src/engine/capture/tree_digest.cpp` | `16417f86f367403995005b040cffb54d92c9265d5a8d30db0e7100da5518e292` |

The coordinator reported verification-generator repairs while this review ran;
the listed product bytes remained unchanged. No shared build, full gate,
release/Linux execution, allocation/race injection, exhaustive schema fuzzing or
external-profile adversarial rerun belongs to this review. Native ledger
transaction completeness is the separate ledger review's scope. Passing focused
tests here does not approve the whole gate or close PR1/PR2. Product changes to
these paths require a fresh review candidate and independent recheck.
