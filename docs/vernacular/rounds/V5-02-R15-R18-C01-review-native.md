# R15–R18 C01 independent native review

Disposition: **request-changes**. Four new findings remain. The reviewed C01
terminal-run fencing and native verdict-lineage corrections are sound within the
checks described below; this review is not an approval of the integrated candidate.

## Candidate and scope

Base comparison: `1338637c`; candidate HEAD:
`ae02a12bf77c6b72d3060253dd47800c645796a7`, plus C01 working changes identified by
`tests/native/r15-r18-c01-evidence/plan/candidate-review-1.json`.
Read the repository instructions, implementation/roadmap/v5 baseline, round brief,
execution contracts and relevant render/ledger contracts. Reviewed the native
execution ledger and transaction/replay paths, R17 evidence/completion storage,
R18 admission/lineage storage, execution codecs, check capture/process adapter,
recovery capture and their invocation connections against the full R15–R18 change,
not just the original findings. No product edits or builds were made.

At final identity check, all manifest-listed product C++/Lua implementation files
still matched. Differences were `src/cmd/planar/CMakeLists.txt`,
`tests/native/r13b1-support/generate.py`, `tests/native/r14c3b2-support/generate.py`,
and `tests/native/r15-r18-c01-policy-cli.py`. These are coordinator gate/verification
changes; refresh the manifest before subsequent approval. CMake changes are test
registration. The CLI probes used the existing source-built Debug binary with
SHA256 `1592b365cdc13632fc3322b95edb3789996929c93e0630ee2bc72a45b2a04207`.

## Findings

### N1 — P1: aliased base and working roots bypass the boundary gate

Location: `src/engine/execution/check.cpp:251-276`, especially line 265.

`prepare_check` canonicalizes all paths but tests only scratch versus the checked
roots. It never rejects overlap between root and base. Passing the same directory
for both makes the comparison tree equal the working tree, so `boundary_gate`
sees no modifications and approves an out-of-ownership change. The existing
`native_capture_session::add_root` explicitly rejects overlapping capture roots
(`src/engine/capture/native.cpp:517-524`); v5 A18 requires root-alias refusal and
R17-4 reuses capture refusals. The new adapter loses that protection.

Executed reproduction: reuse the setup in `tests/native/r17-r18-cli-draft.py`
through worker claim (unit owns only `src/`). Create `current/outside` containing
`unauthorized change`; invoke check with `--root current --base-root current`
and a disjoint scratch directory. Exit 0, result:

```text
ALIASED_ROOT_CHECK completed [('planar.boundary_v1', 'pass'), ('test', 'pass')]
```

Reject overlapping/identical checked roots before reserve using descriptor-backed
identity/ancestry validation, including aliases. Add regression coverage for the
same directory, ancestor overlap and path aliases. This is ordinary CLI behavior,
not a database-corruption-only scenario.

### N2 — P2: malformed saved check responses pass ledger verification and replay

Location: `src/engine/ledger/execution.cpp:1216-1223` (`check_request`).

For both completed check requests and check-recover requests, the verifier only
requires a JSON object whose `kind` is `check`. It does not validate the required
`check_v1` fields or their links to retained reservation/evidence/completion rows.
A malformed stored result is consequently returned as successful machine output.
This falls short of the exact R17-9 response/replay contract and the ledger's
reader verification obligation. Other operation branches validate typed results.

Executed reproduction: after the N1 check, open only the temporary fixture ledger
with Python sqlite3. Save the exact SQL of the immutable `requests` triggers,
drop those triggers, perform the following update, restore the exact SQL, commit,
and close. Schema verification still sees the original schema.

```sql
UPDATE requests SET result = CAST('{"kind":"check"}' AS BLOB)
WHERE request_id = 'check';
```

Both `status --ledger <scratch ledger>` and an identical `check` retry exit 0.
The retry prints exactly `{"kind":"check"}\n`. Expected: `ledger_corrupt`, before
returning a saved result. This is a reader-integrity probe, not a claim that normal
SQL API clients can bypass the immutable triggers. Validate the full response and
its identity/reference relationships, including interrupted-check responses.

### N3 — P2: R17 evidence verification accepts invalid reasons and missing required checks

Locations: `src/engine/ledger/execution.cpp:2156-2204` and
`src/engine/ledger/execution_review.cpp:809-850`.

The evidence reader reads `reason` without enforcing its outcome-dependent closed
set. Completion verification merely requires the remaining evidence rows to pass
and the merged vector map to be nonempty. It does not establish that all required
registry checks and per-unit boundary rows still exist. A completion remains
readable after removal of the required test's evidence, as long as a boundary row
remains. R17-1 explicitly says its reason vocabulary is reader-verified and defines
one evidence row per planned check plus one gate per unit. Retained completion
must not silently survive loss of required evidence (A23/A29).

Executed separately on a normal, disjoint-root smoke completion. For each mutation,
save/drop/restore the exact immutable `evidence` trigger SQL as above:

```sql
UPDATE evidence SET reason = 'not_a_reason' WHERE "check" = 'test';
-- status exit 0, rather than ledger_corrupt
DELETE FROM evidence WHERE "check" = 'test';
-- status exit 0, rather than ledger_corrupt
```

The second mutation removes the entire invalid row, so the missing-evidence probe
is independent of the invalid reason. Derive the expected evidence membership
from retained accepted inputs/dispatch and verify membership and typed row
relationships, rather than relying on the writer's original checks. Add separate
corruption regressions for the closed reason set and missing required rows.

### N4 — P2: timeout/output-limit classification discards a normal termination's exit code

Location: `src/engine/execution/check.cpp:172-175`; underlying result assembly:
`src/engine/process/runner.cpp:407-423`.

The runner retains a normal leader exit code, then overrides `how` to `timed_out`
or `output_limit` when that cause occurred first. `classify` interprets every such
result as signal termination and assigns `-r.signal`, which is 0 when the child
handled TERM and exited normally. The real exit code is therefore lost from
retained evidence. R17-1/DR17-2 requires the normal exit code (or negated signal),
while the reason precedence must still remain timeout/output_limit.

Executed reproduction: replace the smoke registry check argv with
`['-c', "trap 'exit 7' TERM; while :; do :; done"]`, timeout 100 ms. Complete the
normal setup and run check. Result:

```text
TIMEOUT_TRAP_EXIT_7 [('planar.boundary_v1', 'passed', 'none'), ('test', 'timeout', '0')]
```

Expected test evidence is `reason:"timeout", exit_status:"7"`. Preserve actual
termination status independently of the cause and test a nonzero normal exit
from a TERM handler for both stop causes.

## Correction recheck and verification

L1: `target_conflict` now selects the token run for checkpoint/refresh/release and
looks up its terminal state when the role no longer points at it. The changed
precondition routing includes all three operations. Existing source-built
`build/rewrite-debug/src/engine/planar_ledger_execution_tests 'C01*'` passed:
**2 test cases, 640 assertions, exit 0**, no skips reported.

Native F1: `read_review` now retains the verdict run's generation and the preceding
job activation; both the public snapshot and accounting representation include
those values. The C01 test checks those fields and absent activation. Running the
existing `planar_execution_snapshot_value_tests 'C01*'` matched **1 test case,
21 assertions, 1 failed, exit 42** at an obsolete accounting assertion (binary
reports source lines 262/288, while current source has the C01 test at 302 and an
updated accounting projection). This binary is stale; it is not evidence of a
current-source defect. Coordinator was notified to obtain the rebuilt gate result.

Three isolated Python probe executions adapted the existing smoke fixture:
alias plus saved-response mutation, normal completion plus evidence mutations,
and timeout exit-status retention. Each initialized, loaded, activated and claimed
a fresh temporary ledger with the explicit source-built binary. All mutations
were confined to those temporary fixture databases and the temporary directories
were removed. Probe outputs are reproduced above; no installed binary, operator
data or legacy database was accessed.

Full Debug/Release/Linux gates belong to the coordinator and were not rerun in
shared build directories. This review adds source inspection and targeted executed
counterexamples, not a claim of exhaustive concurrency/crash/platform coverage.
No fresh kill-at-commit/disk-full experiments were run. No new defect was established
in runner group cleanup/output bounds, immutable record collision comparison,
R18 atomic verdict/repair writes or retained dispatch/provenance linkage by this
pass; that statement is limited to inspected paths. Pending gates are a validation
limit, not an additional code finding. Only this review report was written.
