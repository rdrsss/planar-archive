# C01 independent CLI verification

Verification author only; this report does not approve implementation. Shared
checkout, no product edits, commits or builds. Used the coordinator-provided
source-built Debug candidate after the C01 runtime/ledger/policy fixes. Read the
three integrated review reports, C01 policy/ledger coder reports, round brief,
repository guidance and applicable execution contracts.

## Executed evidence

`python3 tests/native/r15-r18-c01-policy-cli.py --binary build/rewrite-debug/bin/planar`

**3 scenarios, 0 failures, 0 skips.** Log:
`tests/native/r15-r18-c01-evidence/verification/policy-cli.log`.
Each scenario creates and removes a separate temporary ledger, workspace, source,
base and scratch directory. Every command invokes the explicit candidate binary
with a 30-second timeout; registry checks also have a ten-second limit. No SQL
injection, native snapshot fixture, Lua shim, installed binary or operator data
is used. The registry check is a deliberately small passing check-result writer;
one scenario adds an out-of-ownership path to make the actual native boundary
check fail. The scenarios test policy lifecycle, not a substantive project suite.

1. **F1 and F3:** Complete worker, approve review, accept an immaterial rationale
   change. Reviewer remains done and acceptance remains true, without another
   review. Rename reviewer, accept, restore original name, accept. Worker remains
   done but acceptance is false and the recreated reviewer is ready. A fresh
   approval restores acceptance.
2. **F4 and F3:** Complete worker and claim reviewer. Change material work, claim
   stale worker, restore original declaration, then submit an unowned rejection
   from the still-current reviewer run. Assert current verdict and open blocker.
   Refresh worker using its acknowledged fingerprint and check changed output.
   That same completing check clears the blocker and makes reviewer stale. A new
   approval accepts the changed target.
3. **F4 evidence-only:** Repeat the second lifecycle but introduce an unowned path
   before checking. The boundary failure withholds completion, leaves worker
   running and reviewer blocked, but the newly recorded own-output observation
   clears the obsolete unowned-rejection blocker in the same commit.

The first development executions exposed harness mistakes only: verdict
applicability is nested under `verdict`, and refresh expects the acknowledged
fingerprint. Those were corrected against response schemas and R16, not by
weakening lifecycle expectations. These CLI tests were authored after product
fixes, so they supply integration coverage, not red-first reproduction; the coder
reports preserve pre-fix regression evidence.

## Invariants and mutation recommendations

| Finding | Invariant and focused mutation |
| --- | --- |
| F1 | Recreated reviewer lineage cannot inherit approval. Drop verdict generation in the native serializer, or allow absent generation in policy. The first CLI scenario must fail before the fresh approval. |
| F2 | Captured integration targets must equal current producer publications at evaluation/retry. Remove the captured-target comparison. The actual-module stale-vector regression must fail; add a native barrier after recapture but before the post-run snapshot for a deterministic concurrent-publication CLI test. |
| F3 | Applicable reviewer verdict survives immaterial load; changed target makes verdict inapplicable and reviewer stale. Restore unconditional worker completion lookup in reconcile, or omit reviewer reconciliation in check. Scenarios one and two should fail respectively. |
| F4 | A check clears rejection against its post-result observations/completions, including evidence-only recording. Pass the pre-check snapshot to blocker clearing, or suppress observation insertion when completion is absent. Scenarios two and three should fail respectively. |
| L1 | A release winner fences stale token operations with persisted terminal state. Remove token-operation mapping at locked role mismatch; the native release-interleaving regression must fail on category and run state. |
| RT1/RT2/RT3 | Forward recovery evidence/findings; expose open reservation lookup; skip only `.git` directories. Restore each old adapter/capture branch independently and run the runtime coder's real CLI/native regressions. |
| R15-B R2 | A stale complete-vector write cannot overwrite another successful claim. The faithful mutant must preserve the newly allocated target run (skip target entry or substitute its allocated run ID), then write stale unrelated entries. Assert first claim succeeds before attempting second claim; report whether second commit is refused by the verifier or whether a committed overwrite violates the running/token assertion. A first-claim SQL CHECK failure is not a valid lost-update kill. |

No mutations were executed by this verification lane. In particular, the carried
R15-B R2 faithful mutant still requires a separately recorded run; a proposed
mutation is not evidence of a kill. The first native-policy race test should pause
*after* recapture. Pausing the external consumer check instead pauses before
recapture: changing producer checkout there exercises A15 recapture detection,
not F2's capture-to-post-run-snapshot interval. This lane does not claim a timed
F2 native race reproduction. Full platform gates and independent review remain
coordinator-owned.
