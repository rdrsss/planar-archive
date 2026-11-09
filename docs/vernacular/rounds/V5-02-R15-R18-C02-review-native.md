# R15–R18 C02 independent native review

Disposition: **request-changes**. Two bounded native findings remain below.
The first is reproduced through the real CLI; the second is a direct missing
locked identity check and needs the corrective round's tests-first native proof.
Full gate and mutation evidence is still in progress, so this is not an approval.

## Candidate and scope

Reviewed the integrated R15–R18 native implementation, including execution
transactions/replay, check capture/runner, evidence/completion verification,
review lineage, recovery snapshot and codecs, against the execution contracts
and the repository implementation/roadmap/v5 instructions. The corrections are
identified by `tests/native/r15-r18-c02-evidence/plan/candidate-review-2.json`,
HEAD `ae02a12bf77c6b72d3060253dd47800c645796a7` plus working changes.
At final comparison every manifest-listed native product implementation file
matched. Only `src/engine/runtime/external_registration.t.cpp` (coordinator
aggregate-initializer correction) and `src/stdlib/planar/commands.lua` (policy
lane takeover correction) had changed under `src/`.

No product source or build tree was modified, and no builds were run. The
executed CLI probe used the source-built Debug binary, handoff SHA256
`6dad3ef0201f5ab62e1d2798eabcbede397b65bcbaa5e168825757df1ea02f4c`.
All database edits were confined to a fresh temporary fixture database. No
installed binary, legacy ledger or operator data was accessed.

## C02-N5 — P2: evidence vectors are not checked against their work or observation

Locations: `src/engine/ledger/execution.cpp:2104` (`verify_check_results`),
`:2400` onward (`read_check_rows`), and
`src/engine/ledger/execution_review.cpp:809-852` (`load_completions`).

C02 verifies evidence-row membership but only structurally decodes each
`output_vector_v1`. The completion reader unions its targets without proving
that the vector describes the evidence row's own unit or the captured outputs.
R17-1 requires the own target exactly when `output_vector` was not selected,
and own plus the required dependency producers otherwise. These associations
are necessary for the recorded completion's evidence applicability.

Executed reproduction (one fixture, exit 0): adapt the existing CLI smoke into
one worker owning two units, `unit` and `other`, each with the `test` check using
only `own_output`. Run real init/load/activate/claim/check and exact replay.
The completion has four evidence rows. Replace the two `unit` rows' `vector_id`
with the already existing `other` vector's record ID. Save/drop/restore the
exact immutable evidence trigger definitions around this corruption, leaving
the schema and referenced records intact. `status` then exits **0** and reports
worker **done**, although neither retained `unit` evidence vector names `unit`.
Expected: `ledger_corrupt`, including before replay.

Raw reproducible source and output:
`tests/native/r15-r18-c02-evidence/review-native/vector_probe.py` and
`vector_probe.log`. This does not claim a normal client can bypass immutable
triggers; it tests the reader's promised cross-row verification.

Bounded correction: derive each vector's expected membership from retained
reservation dispatch W and its check input declaration; verify sorted exact
membership and each output against that reservation's retained observation.
Boundary rows must have their own singleton target. Validate observation unit
membership/shape sufficiently to make that comparison meaningful. Keep the
native writer/post-write verifier and historical reader aligned. Regressions
should separately swap an own vector, omit/add a producer target, and substitute
an output from a different observation. Do not treat a saved-response mismatch
as sufficient coverage: vectors are not exposed in `check_v1.evidence`.

## C02-N6 — P2: evidence-only completion omits the locked ledger identity comparison

Location: `src/engine/ledger/execution.cpp:4897-4900`.

`complete_check` calls `check_snapshot_locked`, the helper that compares
`ledger_meta.identity` with `snapshot.identity`, only when `completion` is true.
With `completion=false`, the rest of the function never compares the supplied
snapshot identity. `check_run_locked` validates the run token/current generation
but has no identity parameter. `open_format3` verifies the opened ledger's own
metadata; it cannot establish that this is the ledger from the input snapshot.
Thus this path lacks the identity check required by EX-3 step 6, repeating the
class of defect already closed for general execution commits in R15-B R1.

This finding is source-proven, **not yet executed** in this review: both native
build lanes were reserved. The corrective test should take a valid open
reservation/result fixture, alter only the supplied snapshot identity, call
`complete_check(..., completion=false, ...)`, and require `path_denied` with
unchanged ledger rows. A complementary successful evidence-only case with an
older revision/fact snapshot should preserve the intentionally weaker
precondition set: fix the missing identity comparison without importing all
completion-mode revision/fact preconditions into evidence-only commits.

## Corrections and remaining evidence

The N1 descriptor ancestry/identity guard covers working/base and scratch roots;
N4 now preserves actual normal exit status independently of timeout/output-limit
cause. N2 reconstructs saved complete/interrupted response identities and row
references; N3 now derives required evidence membership and validates closed
reason vocabulary. These changes address their original counterexamples; N5
identifies a separate relationship still unchecked. C01 terminal-token routing
and verdict generation propagation remain present.

The native takeover validator explicitly expects the newly allocated target run
as an absent placeholder in the submitted vector (`execution.cpp:3396-3404`),
then fills the actual run ID during commit. The policy lane's correction to stop
carrying the old run ID is consistent with this native contract; this review did
not claim the policy test passed before the lane finished.

Inspection covered broader transaction, capture and runner paths, but no new
concurrency/kill-at-commit/disk-full experiment was run. Full Debug, Release,
Linux, no-change and mutation gates remain coordinator-owned and pending at this
review. The coordinator requested the next corrective cycle after these bounded
findings; no additional speculative findings are asserted.
