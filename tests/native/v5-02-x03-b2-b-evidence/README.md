# X03-B2-B author handoff — native acceptance candidate

Candidate only; author does not approve it. Shared checkout path base is
`<repo>`, branch `dev/planar-v2-vernacular`,
HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Evidence names below resolve
relative to `tests/native/v5-02-x03-b2-b-evidence/` unless stated otherwise.
No Planar tools, skills, queue, installed binary, operator data, commits,
installation, publication, merge or subagents were used.

## Implementation

- `request_intent::create_v2` constructs an immutable exact request-v2 selector
  from normalized explicit workspace/registry and optional profile paths. The
  existing v1 `create` function is byte-identical to an exact archived copy whose
  SHA256 matches the complete pre-edit baseline (`legacy-constructor-audit.json`).
  Existing record-v1 and pure request-v2 codecs remain unchanged.
- Native interning dispatches complete bytes and digest by typed codec, binds
  exact `(codec,kind,digest)` SQL buckets and compares every bucket member's
  complete encoding. `intern_request_v2` is a distinct typed seam, preserving old
  braced-record calls. Format1 refuses codec2 before insertion; format2 forbids
  request children. Request collision diagnostics retain no private digest.
- Locked acceptance retains identity recheck and exact replay/conflict before
  fresh format/selector checks, CAS and prepared-candidate access. Format1 plus
  request-v2 saved-v1 ID conflicts; unknown ID refuses unsupported_version before
  CAS/candidate access. Fresh format2 requires request-v2 embedded; fresh-v1 and
  profile acceptance refuse. Equal replay precedes stale CAS, moved candidate or
  changed receipts and preserves original saved bytes/receipt authority.
- Matching typed readers/writers permit native format2 writable access and
  explicit recovery. Commit evidence, rollback/uncertain behavior and saved-byte
  fallback machinery remain unchanged. No public CLI, init selection or Lua
  cutover was made; public init still creates format1. X06 profile provenance and
  interpretation remain unsupported.

## Scope and preservation

`baseline.json` was frozen before the first edit. Its root-only exclusions are
`.git/`, `build/`, and this new evidence directory. It covers 15,123 regular files
and two separately recorded symlinks, including historical nested build files.
`baseline.diff`, `status.txt` and `head.txt` retain the existing intentional rewrite.

`final-source-manifest.json` froze the complete source filesystem before final
verification; all 15,125 files and two symlinks still match. No baseline file was
removed. `candidate-manifest.json` and `candidate-source/` hold exact final copies
of all changed/new files. Their `path_base` is the repository root above.
`preservation-audit.json` identifies all changed paths. The coordinator's round
record changed during coding and is separately identified; the author did not
edit it. Coordinator confirmed no further round edits during final verification.

All author changes are within the assigned product/test/status/support scope.
The coordinator additionally authorized only the two obsolete temporary-guard
assertions in `src/engine/ledger/history_verification.t.cpp`; their replacement
scopes the writable owner, aborts the writer and retains unchanged-byte checks.
No B1 support file required adaptation. The existing format.t.cpp temporary-guard
case keeps its original CTest name while requiring native writable success and
invalid-journal refusal without mutation. All read/privacy/publication cases remain.

`baseline-copies/` contains exact archived original copies whose hashes match the
pre-edit baseline for every author-changed existing file. These copies were found
without reconstructing source after editing. `available-baseline.diff` is the
complete author diff plus new support files. Only the coordinator-owned round
record has baseline hash-only availability (`diff-availability.json`). Exact final
snapshots remain authoritative. Active AGENTS.md remains a symlink to CLAUDE.md.

## Verification

Commands were serialized in `build/rewrite-debug`; no source edits occurred during
active commands. `*-build.json`, `*-focused.json` and other command JSONs retain
exact argv/cwd/start/end/status. Logs retain stdout/stderr. XML and LastTest copies
were saved immediately after each test command, before final inventory.

| Final check | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target all planar_tests` | pass |
| CTest filter `ledger\|history\|export\|record\|request\|prepared\|acceptance\|publication\|atomic` | 206/206 pass, zero skips/errors, 74.77 s |
| `python3 tests/vectors/records/generate.py --check` | 10 independent record-v1 vectors |
| `python3 tests/vectors/records/generate-request-v2.py --check` | 2 independent request-v2 vectors |
| pinned `rewrite-check.py lint --build-dir build/rewrite-debug` | formatting --Werror/Doxygen pass, 98 active sources |
| pinned formatting for three changed support C++ files | pass |
| final `all planar_tests` build | `ninja: no work to do.` |
| final CTest inventory | 427 names; all 421 baseline names retained, six added |
| complete filesystem/source audit | all frozen files/symlinks match |

All 14 existing native acceptance tests now run formats1/2 (28 generated format
instances), including chronology/noop/restoration, replay, CAS, three concurrent
request races, identity-bound receipts, capture/unavailable candidate, native
commit-busy and mutation rollback paths. A new mismatch test runs both formats.
The native-only format2 test uses `initialize_new_v2` plus `complete_acceptance`,
then removes live inputs and exercises four history selectors, exact binary source
export and no-overwrite export, native backup/new-target restore and saved replay
on original/backup/restored ledgers. It does not use SQL conversion fixtures.

The two existing D8 real SQLite VFS tests now run both formats. Exact final raw
rows and parser assertions are in `final-focused-LastTest.log` and `fault-counts.json`:

| Format | Journal faults | Database faults | Process boundaries |
| --- | --- | --- | --- |
| 1 | 38, ordinals 0–37; success at 38 | 15, ordinals 0–14; success at 15 | 0,1,2,3,4 |
| 2 | 38, ordinals 0–37; success at 38 | 15, ordinals 0–14; success at 15 | 0,1,2,3,4 |

Every faulted ordinal reports a hit, storage_full, unchanged complete relational
state and same-key retry/replay. Boundaries 0/1 preserve main bytes and refuse
open/recovery/retry for ambiguous non-hot sidecars; boundaries 2/3 explicitly
recover a recognized hot journal then retry the same request; boundary 4 replays
the already committed result. Child fork output repeats some buffered boundary
lines; the audit checks unique boundaries, not inflated line totals.

The atomic production-source variant requires exactly two codec1 and two codec2
hash substitutions (each reader and writer), with exact counts, source/generated
hashes and byte-reversible forward/reverse reproduction. Tests prove codec2
same-bucket unequal collision rollback/private redaction, cross-codec same forced
digest separation and reuse, and same-ID version body conflict. Prior targeted
file/directory/value/request collision and final-receipt mutation cases remain.
All four storage variant generation reports were independently checked against
current source and generated bytes, forward and reverse (`all-variant-audit.json`).
CMake's 21 exact runtime/interface variant audit is also retained in final-build.log.

## Retained failures and coverage correction

- `red-format.t.cpp` was frozen before `red-build`/`red-test`. One selected test
  failed at the native format2 writer guard before the product implementation.
- `first-build` failed because the new test digest module lacked a request-v2
  module dependency. Narrow CMake wiring corrected it; second build passed.
- `first-focused` selected 37 tests: only the two obsolete B2-A guard assertions
  failed. Coordinator approved their narrow adaptation; evidence is retained.
- `third-build` failed on an unqualified test reference alias. Qualifying it fixed
  the compile error; fourth build and second-focused (40/40) passed.
- The 40-case filter covered D8 process death (its test name ends in "atomic")
  but omitted D8 SQLITE_FULL. An earlier progress claim grouped FULL with that
  pass incorrectly. This correction is explicit: only the final 206-case gate
  establishes both complete D8 fault matrices. No second-focused FULL claim.
- The first final audit script assumed a line break removed by clang-format and
  failed before its comparison. `audit-first.py` and final-audit log are retained;
  `audit.py` fixes only the extraction boundary. `final-audit2` passes, including
  exact legacy constructor, filesystem, fault-count, test and inventory assertions.

## Limits and handoff

This is focused macOS ARM64 debug evidence. No complete release/full milestone,
Linux, hardware power-loss, public format2 init/load admission, profile acceptance
or effective provenance/interpretation support is claimed. Existing conservative
non-hot-journal refusal and cooperative-filesystem assumptions remain. Independent
review and the subsequent integrated verification round must precede X03-B closure;
the mandatory external-profile gate and RA01/dependent work remain open.

All reports and manifests were finalized before release of ownership. The evidence
artifact manifest uses this evidence directory as `path_base`, excludes itself,
and covers every other report, log, XML and snapshot. No author edits follow the
handoff. Coordinator owns the round record and subsequent review dispatch.
