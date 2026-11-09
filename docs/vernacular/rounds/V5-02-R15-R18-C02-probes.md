# C02 independent mutation probes

**Executed: 23 distinct mutants.** Twenty-two were killed by the existing
regression assets. The reason-vocabulary mutant survived the existing six-case
corruption probe, then was killed by an additional independent dual-corruption
case. The missing regression was reported to the coordinator and C03 coder for
retention in registered coverage. This report supplies evidence, not code approval.
C03's later vector/ledger-identity corrections are outside this candidate.

The prober changed only this report and
`tests/native/r15-r18-c02-evidence/probes/`. All mutations lived outside the
checkout. No installed binary, operator ledger, legacy tool or main build artifact
was changed. Lua policy probes used a temporary source copy. Native probes used
an APFS clone of the entire Debug tree, including PCMs, test objects, archives and
executables, plus copies of the mutated product translation units. This snapshot
was taken before the C03 lane started editing/building. Compiles and links were
serialized in the coordinator-granted second build lane, then that lane was returned.

Each native mutant first ran its selected unmodified baseline. The runner compiled
the changed translation unit into scratch, replaced its member in a scratch copy
of the owning archive, then linked the scratch test/CLI. Embedded-Lua mutations
changed both assembled bytes and original provenance bytes in the generated source
module; module output also went to scratch. No failure to compile, match tests or
establish a passing baseline counted as a kill. Automated input hashes and exact
compile/link/test commands are retained in `probes/native-results.json` (bulky,
local-only); the small `probes/RESULTS.json` and `probes/manifest.json` summarize and
pin the evidence. `probes/INVARIANTS.md` maps each defect to its mutation.

## Executed results

| Mutant | Passing baseline | Observed kill |
| --- | --- | --- |
| F1 allow missing generation | Actual Lua 15/15 | Missing-generation assertion |
| F1 ignore reviewer lineage | Actual Lua 15/15 | Recreated-reviewer and missing-generation assertions |
| F2 ignore captured producer vector | Actual Lua 15/15 | Stale vector incorrectly completes |
| F3 worker-only reviewer applicability | Actual Lua 15/15 | Immaterial, changed-target and restoration state assertions |
| F4 pre-result blocker snapshot | Actual Lua 15/15 | Completion and evidence-only blockers remain |
| F4 omit evidence-only observation | Actual Lua 15/15 | Evidence-only blocker remains |
| PR1 completion applicability instead of actual done state | Actual Lua 15/15 | Active worker incorrectly admits reviewer |
| F1 omit native generation serialization | 1 case / 34 assertions | Missing generation field and D24 size assertions |
| L1 remove terminal-token target routing | 1 case / 313 assertions | Refresh/release return state_conflict without interrupted state |
| RT1 drop recovery evidence | CLI 5/5 checks | Evidence array empty |
| RT1 drop recovery findings | CLI 5/5 checks | Findings array empty |
| RT2 disable open-reservation lookup | CLI 5/5 checks | Active and abandoned request lookup fail |
| RT3 skip every exact .git entry | 4 cases / 15 assertions | Regular/link/special entries incorrectly accepted |
| N1 bypass physical checked-root overlap refusal | 1 case / 16 assertions | Four same/ancestor/alias variants accepted |
| N4 use signal-derived status for timeout/output-limit | 1 case / 14 assertions | Both normal TERM-handler exits lose exit 7 |
| N2 bypass exact saved-response equality | Six corruption cases, status and replay | Wrong saved request identity accepted |
| N3 bypass required evidence membership | Six corruption cases, status and replay | Required evidence plus matching response entry deletion accepted |
| N3 bypass reason vocabulary | Existing six cases pass under mutant | **Survivor**; independent seventh case kills it, as detailed below |
| PR1 reverse worker/reviewer load order | CLI 3/3 scenarios | Inactive restored worker leaves reviewer blocked |
| PR2 omit recovery repair | CLI 3/3 scenarios | Recovery and preview current work identities differ |
| PR2 manifest repair zero | CLI 3/3 scenarios | Preview repair provenance differs from current work |
| Takeover retain old target run in proposed vector | CLI 3/3 scenarios | Actual takeover receives internal protocol refusal |
| R15-B R2/K6 stale complete-vector write | 1 case / 175 assertions | First claim succeeds; second claim refused by verifier |

The Lua logs and patches are reproduced by `probes/lua_probes.py`. Native patches,
commands and logs come from `probes/native_probes.py`; this helper requires the
coordinator-approved external build snapshot and must not be pointed at an active
main build directory.

## Faithful carried K6 probe

The mutant adds a complete-vector write after the legitimate delta write, but
**skips the target entry**, preserving its newly allocated run. Other stale entries
are written exactly as the old permissive-vector failure would write them.
`raw/K6.log` explicitly records `first.has_value()` passing. It then records
`second.has_value()` failing: 147 assertions passed and one failed. The second
claim's stale builder row conflicts with the retained active run; the shared
post-write cross-row verifier refuses the transaction. This is a verifier kill,
not proof that the delta write alone catches the mutant. The previously reported
first-claim SQL CHECK failure is not reused or counted.

## Reason-vocabulary test gap

The existing `invalid_reason` case edits only the retained evidence row. Even with
the vocabulary guard disabled, saved-response equality rejects that disagreement,
so all twelve existing status/replay assertions still pass. That survivor is
preserved in `raw/N3-reason.log`.

`probes/reason_probe.py` adds `invalid_reason_and_response`: change the evidence
reason and the matching saved-response evidence reason to `not_a_reason`, preserving
all other fields and restoring the exact immutable trigger definitions. The original
candidate rejects all seven corruptions through both status and replay (14 checks).
The same already-built mutant accepts both reads for the new case, yielding exactly
two failed assertions. Raw logs are `N3-reason-independent-{baseline,mutant}.log`;
commands/exits are in `reason-independent-results.json`. This demonstrates an
independent test for the reason guard. It does not retroactively turn the original
six-case probe into a kill or itself establish registered regression coverage.

## Baseline correction and limits

The first F1 native attempt stopped before mutation because the cloned snapshot
binary still contained an older D24 test helper (2722 versus 2348 bytes). With
coordinator authorization, `rebuild_snapshot_test.py` compiled the current
`snapshot_value.t.cpp` into the external clone and relinked only its test binary.
The updated baseline passes 34 assertions; the generation-omission mutant then
fails the intended field assertion. Both the invalid first baseline and successful
second attempt remain in evidence, with the rebuilt test source/binary hashes.

No exploratory race, kill-at-commit or disk-full probe was added here. F2 is an
actual-module policy mutation, not a timed native capture-to-snapshot race. This
is macOS Debug mutation evidence; full Debug/Release/Linux gates and independent
code approval remain coordinator-owned. Changed C03 verifier branches require
fresh probes against their own final candidate.
