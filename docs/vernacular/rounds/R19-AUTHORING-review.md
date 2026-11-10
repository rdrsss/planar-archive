# R19-AUTHORING independent final review

Disposition: **request-changes**. R1 is a source defect and blocks approval,
the actual approval verdict and the local commit. This is not a records-only
review. No implementation was changed by this reviewer.

Fresh ordinary reviewer, using the available current model in place of the
unavailable prescribed Claude reviewer. The checkout was shared; writes were
limited to this report and ignored `review-verification/` evidence. No installed
or legacy executable, operator database, publication or merge was used. The
actual repair ledger was read through its supplied packets/results and was not
mutated by the reviewer.

## R1 — high: embedded identity preparation undercharges cumulative native work

Location: `src/engine/runtime/runtime.cpp:6033`–`6036`.
Affected requirement: canonical I15 and D13, the approved scoped amendment's
allocation boundary, and `command-contract.md:207`–`210` (512 MiB charged native
allocation, counting transfers cumulatively).

The new binder charges the assembly and each original payload **once**, plus a
fixed 65,536-byte allowance, path bytes and one `tree_file` structure per original.
It then calls `history::prepare_stdlib`. That helper performs at least three
distinct logical payload constructions:

1. `history::own` first calls `digest_tree`
   (`src/engine/history/provenance_codec.cpp:55`–`56`). Tree construction creates
   `file_record{file.mode, std::string(file.payload)}` for every file
   (`src/engine/history/tree_core.cpp:163`–`165`).
2. `digest_record` creates a separate encoded hash buffer with
   `encode(input, true)` (`src/engine/history/record.cpp:256`–`258`).
3. After hashing, `own` constructs the returned owned payload strings
   (`src/engine/history/provenance_codec.cpp:60`–`62`). Temporary destruction does
   not refund a cumulative budget.

Those files include both every original and the complete assembly. Canonical
metadata, tree nodes/maps, file framing and directory records add further work.
The existing profile preparation accounting explicitly recognizes the same
two-digest-copy plus owned-copy model (`runtime.cpp:4614` onward: four digests and
three owned trees require eleven payload passes), so its accounting cannot be
replaced by a one-pass reserve here.

The reviewer independently counted the actual generated embedded arrays. For
the authoring bundle, originals total **268,346 bytes**, assembly **268,363
bytes**, and their combined payload P is **536,709 bytes**. The new code pays P
once; the three explicit payload passes alone require at least **1,610,127
bytes**, before the additional metadata/tree work. The missing two payload
passes total **1,073,418 bytes**; the fixed 65,536-byte allowance and small
path/structure entries do not cover them. Production has the same defect:
P=463,972 and three payload passes=1,391,916 bytes. These are source-derived
logical-allocation lower bounds, not an observed process-RSS or allocator-
exhaustion measurement. Details are retained in
`review-verification/ACCOUNTING-LOWER-BOUND.json`.

This permits a fresh bound execution to proceed with insufficient accounting
for its own provenance preparation, violating the declared cumulative ceiling.
The similar older load code does not establish correctness for this new path;
the repository explicitly requires reviewing reused libraries at their new
boundary. The existing I15 probes check charge presence/order and owner lifetime,
not charge quantity. Their passing results therefore do not close this defect.

Required correction:

- Derive and reserve a conservative complete cost for this one preparation:
  payload passes, canonical metadata, tree/hash framing and scratch/owned
  structures. Use checked multiplication/addition before allocation and the
  existing typed native resource failure. An alternative that avoids copies must
  account for the copies it actually retains and preserve the exact digest.
- Keep derivation after early replay, preserve profile binding and exempt reads,
  and retain consumed/failed-owner checks and terminal release. Do not restore a
  production fallback or change EX-6 equality.
- Add a meaningful quantity/budget regression that the present one-pass formula
  fails. A private compiled budget fixture is acceptable; it must distinguish a
  sufficient reservation from the old under-reservation, with an insufficient
  budget refusing before provenance/native service work and remaining terminal.
  Do not add a public product fault hook or merely assert that a `charge` call
  exists. Extend the invariant/probe row with a quantity mutant.
- Keep the correction scoped to the new path unless a searched, reviewed shared
  accounting factor requires another caller. Rerun affected probes and the
  required gates for the changed candidate; retain this candidate's successful
  results as historical evidence.

## Other source and contract findings

No other blocking source issue was found in the reviewed scope.

- All twelve ordinary execution/read branches forward the admitted VM. The
  copied handler modules and generated invocation/runtime types remain typed;
  no cast or runtime injection API was introduced. The authoring bootstrap's
  twelve execution profile routes intentionally retain the original closed
  profile owner, with reversible protected transformations and explicit links.
- `execution_options.cppm` contains data-only invocation storage below handlers.
  Independent comparison confirms twelve CLI states, all 78 field slots/types/
  defaults, unchanged CLI11 attach bodies, and complete field-order mappings in
  the six former direct-intent handlers. The other six implementations are
  unchanged. No shadow parser or new authoring macro site was found.
- The embedded digest is derived from the invoked VM's retained originals,
  separator and assembly using the accepted provenance recipe. It is not the
  profile-only effective digest or registration seal. Native services fail
  closed without binding; the former production-source fallback is removed.
  Profile `run_known` retains its verified effective value. Early replay/body
  conflict remains before fresh invocation; bare status/request lookup and
  check-recover skip the new derivation. History/export and authoring ledger
  guidance remain exempt.
- Generated sources retain exact inverse checks, test copies are excluded from
  default `all`, and the new independent X08 inventory requires exactly 43 C++
  inputs. The repair changes only its explicit expected family tuple; missing,
  extra and altered-file controls remain meaningful.
- I5's corrected wording is appropriate: a stale verdict can be historical but
  cannot establish current acceptance. No new blanket stale-verdict refusal was
  required. The existing reviewer-packet receipt wording is carried separately;
  the actual supplied packet contains the exact output/W, and the three check
  receipts were inspected independently.

## A1 — low, records: appended canonical rows fall outside Markdown tables

`plan/DECISIONS.md:18` and `plan/INVARIANTS.md:20` insert blank lines before D11
and I13. Those lines end their Markdown tables, so the appended rows render as
pipe text rather than table rows. Remove the separators or provide a new proper
header/delimiter. The raw rows were readable for this review; this is not an
additional source blocker.

## Candidate and exact review target

Reviewed HEAD: `d6c989ed9e196bf9b357019ea946e6ff18c47f18`, branch
`dev/planar-v2-vernacular`. The frozen 305-file code/support aggregate is
`161c575fc532e61ef6debd2183e019bd6d9357b17e98f255c8fce57a52d29b3b`.
The identity document's SHA256 is
`b74a68206dd8afc07421f8858ca7a5f184d837de17013aaa773d1d6870eb47b9`.
`REVIEW-MANIFEST.json` supplies the concrete source/record candidate, including
new untracked sources. All 305 code hashes, 55 published candidate hashes and
12 pinned local artifacts matched when checked. The source diff includes the
seven new code/support files; review was not limited to `git diff`'s tracked
output.

The actual claimed reviewer target is:

- Work `retained_vm`, output
  `xxh3_128:6cec859929e48c6048a9c613a863d6c5`.
- Worker W `xxh3_128:6e5ad347ad1ac281b62a9fbd6b82c812`.
- Reviewer policy `xxh3_128:2de11651fcbaab87ac9982b6e196ef5c`.

The actual `authoring.check.2` result is completed and has passing host_debug,
host_release and automatic planar.boundary_v1 evidence. Its output/W and the
retained reviewer packet match this target. This review withholds approval of
that exact candidate because of R1; it does not fabricate a product verdict.

## Independently verified evidence

The reviewer parsed both actual CTest inventories and JUnit reports: each
configuration has **1,849/1,849**, exact name equality, zero failure/error/skip/
disabled entries. Both 801-entry before/after source freezes are equal, both
lint logs report **251** sources, and both repeat outputs are exactly the known
glob probe followed by Ninja idle. Publication redaction changed only records
after the gates; the 305 code/support hashes remain exact.

All **2,178** entries in the independent probe raw manifest matched their local
artifact hashes. The 31 tested groups are recorded honestly: 30 assertion/
source/graph/native-work observations and one isolated consumed-owner mutant
SIGSEGV. No tested survivor is hidden. Their quantity-coverage gap is R1, not a
contradiction of the reported kills. Failed injection/setup attempts and the
interrupted first gate are retained, not counted as successful gates.

The reviewer additionally executed:

```sh
python3 tests/native/r19-authoring-support/lifecycle.py \
  --authoring build/rewrite-debug/src/cmd/planar/planar_authoring_stack \
  --production build/rewrite-debug/bin/planar \
  --evidence tests/native/r19-authoring-evidence/review-verification/lifecycle
python3 tests/native/r19-authoring-support/wiring.py --build build/rewrite-debug
python3 tests/native/r19-authoring-support/wiring.py --build build/rewrite-release
python3 tests/native/r19-authoring-evidence/probes/field_audit.py --repo . \
  --output tests/native/r19-authoring-evidence/review-verification/FIELDS.json
git diff --check
```

All exit 0: five actual lifecycle scenarios with zero failures/skips, fifteen
bootstraps/180 family bindings in each configuration, and all fields/adapters.
`review-verification/AUDIT.json` and `PROBE-INDEX.json` retain the independent
hash/JUnit/index checks. No full suite was rerun unnecessarily and no native
build was performed by this reviewer.

The real baseline activation assertion-red, later candidate/profile failures,
temporary-path setup-only correction, and strict inventory repair are kept
distinct. Initial bootstrap repair preceded successful authoring dispatch; the
actual later check/reviewer handoff is not presented as pre-code dispatch.
Linux and the remaining full R19 quota/grouping obligations remain unproved by
this host result. R1 requires a fresh corrective round and fresh review before
approval or commit.
