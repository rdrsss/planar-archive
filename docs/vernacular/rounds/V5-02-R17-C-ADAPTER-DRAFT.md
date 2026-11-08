# R17-C check adapter draft

Resumed 2026-11-08 under the operator's code/feature completion before review instruction.
This draft implements the native digest-only tree capture, execution adapter and trusted Lua
check planning/applicability/evaluation. Ledger transactions and CLI integration are separate
concurrent packages. It is unreviewed and does not claim the full R17 acceptance gate.

## Implemented

- Descriptor-relative streaming `file_bytes_v1` hashing, stable-file and directory checks,
  normalized modes, exclusions, portable/hardlink alias refusals, capture ceilings and sorted diff.
- Pre-reservation root/base/scratch capture, disjoint scratch, exact contract delivery,
  producer vector checks, bounded facet retention and native observations.
- Required command execution, explicit environment and reserved variables, unique scratch
  directories, strict Glaze result parsing including 53-bit counts, outcome precedence,
  boundary gate per own unit, post-run recapture and retained facet publications.
- Pure `checks.lua` planning from accepted material, current/applicable completions,
  observed/output-vector mismatches, evidence prose and completion evaluation.

Checks run trusted registry commands with bounded time, output, cwd and environment and no OS
sandbox; escaping descendants are not tracked.

## Integration seam

`execution/check.cppm` exports `prepare_check`, `check_observation`, `run_checks`, `classify`
and `boundary_gate`. Preparation consumes `check_plan_v1` plus root ID, base label,
registry identity, units as `{work,paths}` and contract accepted bytes. Native invocation
must validate the plan against the accepted interpretation/registry before preparation.
The prepared object owns descriptors until completion. The caller supplies the committed
reservation ID to `run_checks`. Native facts contain definitions/vectors as immutable values,
retained process output and publication files for ledger conversion.

Embed checks.lua after dispatch.lua, before review.lua. Wire review's current_work hook
to checks.current_work, and checks' acceptance/blocker hooks to review. The completion evaluator
returns facts, transitions, vector and withheld reasons; lock preconditions are owned by the
native transaction API. The integration package must apply downstream reconciliation and
populate accepted snapshot observed/evidence/publication rows.

## Draft verification

- `cmake --preset debug`: exit 0.
- `cmake --build build/rewrite-debug --target planar_execution_check_tests -j 3`: exit 0.
- `ctest --test-dir build/rewrite-debug -R 'R17-C ' --output-on-failure`: 5 matched, 5 passed,
  zero skips. Real command, count-file, mutation/recapture, digest identity, alias, overlap
  and result-precedence cases.
- Lua syntax parsed successfully using the local Lua interpreter.
- No full suite, Linux, mutation probes, review or acceptance validation run.

The five focused checks preceded the final small edits preserving matched counts on a failed
process and enforcing aggregate facet bounds before reserve; final compilation is coordinated
with the ledger package. Those edits require the final integration check.

## Completion/review integration follow-up

The R18 completion reader now requires real R17 check_recorded events and completed
reservations, verifies the acknowledged dispatch preceding reserve, and reads passing
evidence target vectors into completions. The former structural-only completion stub is
removed. Old R18 tests that manufacture a completion by SQL without a check event need
conversion to the real R17 reserve/complete API in the final fixing work.

checks.lua rederives downstream role state over a hypothetical completion, including
producer publication changes and evidence-vector staleness. review.lua uses the real
check completion policy and installs acceptance/blocker functions after embedding.
`lua tests/native/r17-checks-policy.lua .` passed four policy scenarios: vector staleness,
identity restoration, lineage filtering and downstream evidence invalidation. Native
compilation is coordinated with the ledger package; full verification remains deferred.
