# R15–R18 C01 ledger corrective

Candidate base: `ae02a12bf77c6b72d3060253dd47800c645796a7`. This is an implementation report, not approval. Independent integrated review and gates belong to the coordinating round.

## Changes

L1: the lock-time target-role mismatch handler now recognizes checkpoint, refresh and release as token operations. If the locked target no longer names the supplied run, it reads that run's persisted state and returns `fenced` with that state. Claim/takeover behavior and earlier EX-3 failed-member precedence remain intact. A release winner therefore fences stale refresh/release with `interrupted`; a takeover winner can still fail the earlier fact-version member, while a fresh authorization reports `fenced`.

F1 native transport: every selected verdict now retains its reviewer generation identifier and the activation preceding its verdict sequence. Both the Lua snapshot serializer and the D24 review accounting value carry those fields. Current-activation historical verdicts remain available for blockers/findings; policy can distinguish them from verdicts in the recreated reviewer's lineage. The parallel policy corrective owns Lua enforcement. Tests cover explicit generation/activation transport, absent activation, native stored context, and structure change followed by restoration retaining the original verdict generation.

The complete owned test binaries exposed stale integrated-draft fixtures. Remaining R18 SQL completion stubs were replaced with real reservation/completion transactions using the claim/takeover token; the verifier was not changed. The completed-run assertion now counts the completed builder and reviewer. Snapshot assertions now include the shipped R16–R18 transport members; D24 equality uses its reduced accounting projection, including omission of empty extensions and removal of supplemental lineage/recovery members. Semantic member/parameter documentation was added in `execution.cppm` and `storage_types.cppm` following coordinator assignment of preflight warnings.

## Evidence and commands

Raw evidence: `tests/native/r15-r18-c01-evidence/ledger/`.

- Before implementation changes, built the added regressions against base implementation with `cmake --build build/rewrite-debug --target planar_ledger_execution_tests planar_execution_snapshot_value_tests -j3`. Build succeeded (`red-build.txt`).
- `planar_ledger_execution_tests 'C01*'`: one matched case, 313 assertions, four failures demonstrating both release-winner refusals returned `state_conflict` with empty state (`red-ledger_execution.txt`).
- `planar_execution_snapshot_value_tests 'C01*'`: one matched case, ten assertions, two failures demonstrating absent generation/activation (`red-execution_snapshot_value.txt`). Neither red run skipped tests.
- Rebuilt the two targets successfully after product fixes (`green-build.txt`, `green-build-final.txt`). Whole-binary development execution selected 160 ledger cases: 156 passed, four stale R18 fixture failures; 99,605 assertions, four failures. The C01 token interleavings passed. Snapshot execution selected six cases: one passed, five failures from stale exact-key/D24 assumptions, including the new test's initial D24 assumption. These honest intermediate failures are retained in `baseline-fixture-failures-{ledger,snapshot}.txt`.
- Pinned `/opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror` passed for owned C++ files (`format.txt`); subsequently added test text was formatted with the same formatter.

The final fixture repairs, additional native F1 restoration case, and semantic documentation were handed to the coordinator for the combined rebuild and gate after releasing the exclusive Debug slot to the runtime lane. Final combined counts and independent-review disposition must be read from the integrated C01 record; the intermediate logs above do not establish a passing final gate. No verifier weakening, installed binary, legacy tool, operator database, commit, installation or publication was used.
