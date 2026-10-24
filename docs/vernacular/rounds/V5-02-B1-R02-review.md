# V5-02-B1 R02 independent review

Disposition: **request-changes**. Do not advance B2 on this candidate.

This operator-requested retry completes a bounded source review using retained
evidence. The checkout is shared; ownership is limited to this new report and
`tests/native/v5-02-b1-r02-review-evidence/`. No product/source fixes, builds,
tests, generated test programs, additional experiments, network work, installed
binaries, real databases, Planar tooling, commits or installation were used.
Prior evidence and the interrupted review record remain intact.

Reviewed candidate: 11 files at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, on
`dev/planar-v2-vernacular`. Candidate-manifest SHA256:
`91ea6874381d3d37570eacd0243e812e1fa2526a3888ef049097770daf6f03aa`;
owned-diff SHA256:
`295a89f4ff4c4952a2875634fc91e8710fc0780a3768be6b5a278cb7c4078f7a`.
Current candidate bytes match the retained snapshots. All 77 retained coder
artifacts match their manifest; its SHA256 is
`1e2073dd2ad40bd641618dfefb4bcbc90be754f2f01f753877e74bcd268d542f`.

## Findings

1. **High — captured module state remains mutable after sealing.**
   `src/engine/runtime/runtime.cpp:892` seals registration and global tables;
   `freeze_table` at line 851 follows table-valued entries only (lines 867–869).
   Neither path walks the registered function references or their transitive
   Lua closure upvalues. Retention at lines 736–739 preserves those upvalues;
   it does not freeze them. Module-local schema tables are real retained state:
   `src/stdlib/planar/schema.lua:57` defines `workspace_schema`, used by
   `route_workspace` at line 204, which the registered render handler calls at
   `src/stdlib/planar/commands.lua:8`.

   Observed retained evidence:
   `tests/native/v5-02-b1-review-evidence/probe-results.json`, `upvalue`, records
   successful creation followed by a handler result containing
   `changed-after-seal`. The B1 round identifies this as mutation of a captured
   module-local schema table. This retry did not rerun it. Independent source
   inspection explains that result: a table reachable only through a closure
   is outside the sealing roots/traversal. This violates
   `command-contract.md:118`–121, which requires trusted module/global tables
   frozen before invocation. No claim is made that the unaltered shipped handler
   currently performs that mutation.

2. **High — native sealing traversal lacks an explicit traversal/stack bound.**
   `src/engine/runtime/runtime.cpp:851`–890 recursively traverses each previously
   unseen table, retaining Lua temporaries and native recursion frames. Its memo
   prevents revisiting table cycles, but there is no depth/work counter, live
   stack-limit check or stack-capacity check in this traversal. The instruction
   and stack checks at lines 58–67 run through Lua hooks; direct C++ recursion
   does not execute Lua instructions or make a Lua function call. The reservation
   at line 1394 does not impose a 2048-slot ceiling. Thus the source does not
   enforce the contract's 2048-slot registration ceiling throughout sealing or
   guarantee a typed resource refusal for a sufficiently deep registration table
   graph.

   This is a source-level acceptance gap. Its precise runtime failure mode is
   unverified here. The retained `deep` result is an `internal` refusal; the
   retained `deep_large` result is only a 15-second timeout. Neither establishes
   a crash, its cause, or a completed regression test. The timeout is not the
   basis for this finding.

## Lifecycle and error observations

The callable-lifetime design is coherent on inspection: `luaL_ref` retains each
function in the registering VM; execution fetches that exact reference at
`runtime.cpp:1282`. A stable heap owner contains the allocator context and Lua
state (`runtime.cpp:1344`); move operations transfer the owner. Terminal cleanup
closes Lua before releasing its referenced native state (`runtime.cpp:1428`).
No additional lifetime defect was identified within this bounded review.

Service conversion/native owners unwind before the allocating response frame
(`runtime.cpp:1155`, 1262–1275). Native errors are retained before creating the
Lua response (1225–1228) and win after the protected return (1463–1465).
Bridge failures and exhaustion remain terminal; export publication state is set
before later result validation (1213–1215). These source observations and the
retained tests support preserving this behavior during correction; they do not
establish real storage adapters, write-lock behavior or CLI integration.

## Corrective acceptance

- Seal all registration-owned tables reachable from retained callables, including
  transitive helper closures and module-local schema tables, while allowing
  invocation-local scratch state. Test existing-key replacement, added keys and
  nested/shared state with ordinary bounded cases; preserve alias/cycle handling.
- Bound the complete native sealing walk, including its Lua temporaries and
  native work/storage. Refuse over-limit cases with the existing redacted typed
  resource policy; do not rely on hook callbacks or reserved capacity. Use small,
  deterministic boundary regressions, without another deep stress run.
- Preserve schema iteration/length/read behavior, exact same-bundle dispatch,
  move/destruction/one-shot lifetime, native failure precedence, caught exhaustion
  and export uncertainty. Retain the prior 83 focused checks, current golden
  bytes and provenance rules; add the corrective regressions and independently
  review the resulting exact candidate in a fresh corrective round.

## Evidence and limits

Independent byte checks verified 11 current files and their snapshots, 77 coder
artifacts and 5,643 protected baseline files. Parsing retained `verified.xml`
confirmed 83 cases, zero failures and zero skips, including four actual CLI
suites; retained inventories contain all 337 earlier names plus 17 new names,
354 total. These are retained coder test results, not tests executed by this
reviewer. Existing lint reports 87 sources, vectors report 12/20/41, and the
no-change build log reports no work; none was rerun.

`tests/native/v5-02-b1-r02-review-evidence/input-sha256.json` hashes the review
inputs and retained artifacts; `audit.json` records the independent byte/count
checks. `output-sha256.json` hashes this report and the new audit records. The
previous review's `independent-audit.json` and `probe-results.json` remain
unchanged and are included as inputs. No full 354-test run, release/Linux result,
B2 CLI integration, workspace naming, load or external-profile support is claimed.
