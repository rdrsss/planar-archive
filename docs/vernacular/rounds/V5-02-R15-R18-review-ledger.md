# Integrated R15–R18 independent ledger review

Candidate: `ae02a12bf77c6b72d3060253dd47800c645796a7`; diff base: `1338637c`.
Disposition: **request-changes**. Review performed 2026-11-08.

Scope: native execution ledger, run lifecycle, reservation/completion commits, verdict reader/writer, lineage, replay and storage history expansions. Read the repository instructions, implementation/roadmap and integrated draft record, v5 baseline and execution/R16/R17/R18 contracts. Product files were read-only; this report is the only repository write. This is a bounded code review, not full matrix acceptance.

## L1 — P2: Map a terminal token's lock-time role mismatch to `fenced`

Location: `src/engine/ledger/execution.cpp:3418-3422` (`check_preconditions`).

A `refresh` or `release` can pass token authorization and build its change while the run is active, then lose the commit race to another `release --reason interrupted`. The winner changes the role's run to NULL without bumping fact versions. The loser's active revision, active job and fact versions still match, so it reaches the role-state precondition. That branch recognizes only claim/takeover as a target operation and returns `state_conflict` immediately for refresh/release, before checking `run_active`.

The required result is `fenced` with `run_state: interrupted`: R16's “Refusal order (token verbs)” step 10 explicitly maps a failed target role-state member whose locked run differs to `fenced` with the locked run state. Actual behavior reports a retryable state conflict with an empty state even though this token is permanently unusable. Drivers cannot distinguish loss of ownership from an ordinary retryable snapshot conflict using the promised error contract. The write is refused; this finding does not claim a stale write succeeds.

Fix the token-operation target mismatch mapping, reading the named run's locked terminal state before returning. Add deterministic native interleaving coverage for refresh/release losing to release, as well as takeover. The existing R16 wrong-secret test exercises only the later `run_active` path.

**Tested evidence:** an isolated copy of `execution.t.cpp` appended one Catch2 test using existing fixture helpers:

```cpp
for (auto op : {"refresh", "release"}) {
  scratch dir;
  auto path = activated_ledger(dir);
  auto claim = commit_claim(path, "claim-race", "alpha", "builder");
  REQUIRE(claim.token);
  auto stale = snapshot_of(path);
  auto change = recovery_change(stale, op, std::string(op) + "-race", *claim.token);
  REQUIRE(commit_execution(path, stale,
      recovery_change(stale, "release", "release-winner", *claim.token)));
  auto result = commit_execution(path, stale, change);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == execution_error_code::fenced);
  CHECK(result.error().state == "interrupted");
}
```

Both operations returned enum value 12 (`state_conflict`), rather than 14 (`fenced`), with empty state. One matched test, 311 assertions: 307 passed, four failed, zero skips. Raw scratch source/executable/log: `<temp-root>/planar-review-ledger-1140vw98/{test.cpp,tests,result.txt}`. Its inherited test title still says “takeover”; the actual inserted source uses the release winner shown above.

An earlier takeover variant also reproduced `state_conflict` with empty state for checkpoint, refresh and release (one matched test, 481 assertions, six failed). Its earlier fact-version/generation failures make the refusal-order interpretation less narrow, so L1 rests on the release interleaving above. Scratch: `<temp-root>/planar-review-ledger-zx6hfmla/`.

## Verification and limits

- Ran existing explicit candidate binary: `build/rewrite-debug/src/engine/planar_ledger_execution_tests 'R16*,R17*'`: six matched cases, 1015 assertions passed, zero skips.
- Compiled only scratch copies with the pinned compiler command from `build/rewrite-debug/compile_commands.json`, changing input and object output to scratch. Linked scratch executables against existing development archives using the link command obtained read-only through `ninja -t commands`, omitting its CMake test-discovery post-command. No shared build output was changed.
- Executed both scratch cases described above; each fixture creates and removes its own temporary ledger. No installed Planar or operator database was used.
- Existing archive/source correspondence was assumed from the integrated development build; no full candidate rebuild, release/Linux run or broad gate was performed. The finding is independently supported by the candidate source at the cited branch.
- Potential downstream Lua rederivation/current-vector issues were sent to the coordinator's policy review and are deliberately not duplicated here. No additional native finding is claimed from incomplete hypotheses.
