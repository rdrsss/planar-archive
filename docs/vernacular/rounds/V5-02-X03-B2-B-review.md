# V5-02-X03-B2-B independent review

Decision: **request-changes** for B2B-R1, the selected-format continuity check.
The focused native suite passes; it does not resolve this exact-contract gap.
No other blocking finding was identified. This review does not close X03-B,
X06, the mandatory external-profile gate, or public format2 admission.

Reviewed on the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Reviewer ownership is limited to
this report and `tests/native/v5-02-x03-b2-b-review-evidence/`. Product code,
source tests, author evidence and status documents were read-only. No subagents,
Planar tools/skills/queue, installed binary, operator data, commit, installation,
publication or merge. Debug build/test commands were serialized.

## Required correction

**B2B-R1 — P2: compare the selected ledger format at the locked metadata boundary.**

[X-STORE](../external-profile-contract.md) lines 642–646 requires an identity/format recheck
inside the transaction. At `src/engine/ledger/format.cpp:1362`, `begin_records`
verifies the database after acquiring the write transaction, but the continuity
comparison at line 1368 checks only file identity, ledger identity, workspace and
creation time. It omits `checked->format != state->info.format` and returns the
owner with its earlier `state->info` unchanged. Fresh acceptance at line 1601 then
uses that retained format to choose which request codec is admissible. Verifying
that the locked database has *some supported exact schema* does not establish
that it still has the selected format used by this decision.

The same omission remains in the metadata comparisons in `record_writer::commit`
at line 1527 and final acceptance at line 1731. Other existing continuity boundaries
already compare format: `open_existing`'s read-to-write transition at line 1221,
staged publication at line 1875, and `same_metadata` used by recovery at line 2057.
The format discriminator was added in B1; B2-B now removes the writable guard and
uses it for fresh request admission, so this is relevant to the candidate's
complete typed-write boundary.

Enforce selected-format continuity in the locked check and corresponding
precommit metadata checks. Preserve saved-ID exact replay/conflict before fresh
selector checks, CAS or candidate access, and preserve receipt/recovery authority.
A fresh corrective round must review the change before integrated verification.

This finding is based on the explicit contract and source checks. No timing
reproduction, outside-writer simulation, new executable test or observed runtime
failure is claimed. There is no supported format-conversion API. The coordinator
ended a contemplated reproduction investigation before any such asset was
created or executed; no conclusion here depends on that investigation.

## Exact candidate and preservation

Repository path base: `<repo>`.
Author names below resolve under `tests/native/v5-02-x03-b2-b-evidence/`.

| Frozen identity | SHA256 |
| --- | --- |
| `candidate-manifest.json` | `b924e8f31db8b6749f6e8297f2be15f3af94cd6041e2880d592d721f8f3cdfde` |
| `final-source-manifest.json` | `3d946a5396bd8c282211ce8dda55da83a0ef8f2ec848aa86115861b65aeeaa67` |
| `available-baseline.diff` | `aa7abedcd20e0256312b3672a346c8c481d564789557953d1c955215bd2441e0` |
| `artifact-manifest.json` | `2de6935eb7564697b815ad3d3492ea4a60157e729d832939bd751194296c7b3c` |

Initial, final and handoff audits verify all 24 candidate members against both
working files and exact candidate snapshots, and all 119 author artifacts against
their recorded evidence path base. Every one of the 15,125 final filesystem files
and both separately recorded symlinks matches the pre-verification snapshot.
A root-only `.git/`/`build/` exclusion walk, additionally excluding this round's
author/review evidence and review report, finds exactly that inventory, including
nested historical build files. AGENTS.md remains a symlink to CLAUDE.md.

The supplied pre-edit baseline contains 15,123 files and two symlinks. No baseline
file was removed; changed/new paths exactly match the candidate list. Exact
archived pre-edit copies match baseline hashes for every author-changed existing
file; independent unified-diff reconstruction matches `available-baseline.diff`
byte for byte. Only the coordinator-owned round record has baseline hash-only
availability, as disclosed in `diff-availability.json`. This report does not claim
to reconstruct its historical text. The legacy `request_intent::create` function
is independently byte-identical to its exact archived baseline span.

## Other implementation and test findings

Read the active guide, implementation plan, roadmap, v5 direction, assigned round
and narrow expectation authorization, X-STORE and bounded implementation path,
B1/B2-A reviews, author handoff, candidate diff and raw verification artifacts.
Outside B2B-R1, the reviewed implementation follows the bounded native scope:

- `prepared.cpp:69` retains existing request validation, adds normalized optional
  profile validation and uses the approved full-frame request-v2 encoder and
  aggregate bounds. Typed data is held through private immutable ownership;
  existing v1 framing and both pure codecs are unchanged.
- `format.cpp:1388` dispatches exact encoding/digest by typed alternative, refuses
  codec2 insertion into format1, and forbids either request kind as a format2
  directory child. Buckets bind codec/kind/digest and compare every member's
  complete bytes. Collision detail omits request digests. `intern_request_v2`
  remains a distinct native seam and rolls back the owner on refusal.
- Acceptance keeps expected ledger identity before request replay, full-body
  replay/conflict before fresh checks, and fresh format/profile refusal before
  CAS or prepared-candidate access. Known format1 saved-v1 IDs conflict with v2;
  unknown IDs refuse unsupported version. Fresh format2 v1/profile requests
  refuse. Native completion/rollback/uncertainty/fallback machinery is unchanged.
- Native format2 recovery uses the existing complete scratch verification and
  same-metadata check before original recovery. Conservative non-hot journal
  refusal and explicit hot-journal recovery remain intact. Public init/CLI/Lua
  were not cut over; profile accepted history still refuses until X06.

All 14 prior native acceptance cases retain their assertions and are parameterized
for both formats, including all three concurrency scenarios and identity-bound
receipts. The added mismatch case verifies refusal ordering with a moved candidate.
The new format2 workflow uses actual `initialize_new_v2` and `complete_acceptance`,
then removes live inputs and tests all four history selectors, exact binary source
export/no-overwrite, native backup/new-target restore and exact saved replay on all
three ledgers. It does not use the B2-A SQL reader fixture as acceptance proof.

The only changed `history_verification.t.cpp` assertions are the two authorized
temporary guard expectations, replaced by scoped writer abort/writable success
with unchanged-byte checks. The format test likewise retains invalid-journal
refusal and unchanged snapshots after removing the temporary guard. Prior
lifecycle/publication/collision cases and their names remain.

All four generated storage/export variants independently reproduce their eight
source files forward and backward with matching hashes. The atomic variant has
exactly two codec1 and two codec2 digest substitutions, covering reader and writer.
Its tests retain prior collision/receipt proofs and add codec2 collision rollback,
private digest redaction, forced equal-digest cross-codec bucket separation/reuse,
and same-ID version-body conflict.

## Independent verification

Review evidence uses its own directory as path base. Exact argv, cwd, timestamps,
exit status, raw logs, JUnit and copied LastTest are preserved.

| Check | Independent result |
| --- | --- |
| Debug `all planar_tests` build | Pass; no work after normal Ninja glob recheck |
| CTest regex `ledger\|history\|export\|record\|request\|prepared\|acceptance\|publication\|atomic` | 206/206 pass, zero failures/errors/skips, 74.00 s |
| Record-v1 / request-v2 vector oracles | 10 + 2 verified |
| Pinned format `--Werror` and Doxygen | Pass, 98 active sources |
| Pinned formatting of three changed support C++ files | Pass |
| Final debug `all planar_tests` build | No work |
| CTest inventory | 427 names; all 421 baseline names retained, six added |
| Final/handoff source, artifact and reversible-variant audit | Pass |

Independent raw D8 output establishes the following, rather than inferring
format coverage from unchanged CTest names:

| Format | Journal faults | Database faults | Unique process boundaries |
| --- | --- | --- | --- |
| 1 | 38, ordinals 0–37; success at 38 | 15, ordinals 0–14; success at 15 | 0,1,2,3,4 |
| 2 | 38, ordinals 0–37; success at 38 | 15, ordinals 0–14; success at 15 | 0,1,2,3,4 |

Every faulted ordinal reports a hit and fails with storage_full; existing
assertions verify initial history/row counts, same-key retry and exact replay.
Process boundaries 0/1 retain unchanged main/non-hot sidecars and refuse ordinary
open, explicit recovery and retry. Boundaries 2/3 explicitly recover recognized hot
journals then retry; boundary 4 replays the committed result. Repeated buffered
fork output is counted by unique boundaries, not inflated line totals.

The author's initial guard regression, two compile failures, obsolete guard
assertions and audit-extractor failure are retained and disclosed. The earlier
40-case filter omitted SQLITE_FULL; its corrected coverage claim is honored.
Only the final author and independent 206-case runs establish both full fault
matrices. Reviewer evidence also retains an initial audit script that skipped
r13b1's nested report map and a result-parser failure caused by Ninja's extra glob
line. Corrected independent audits handle both; no product/test changes were made.

Review `artifact-manifest.json` freezes 37 other review artifacts and excludes
itself. Its SHA256 is
`40aa6e6e32a4e3bd2db085d4e1bf547e3f28893e5e3dd7e3dec8ff0df933648e`.
Reports/manifests are frozen before ownership release.

This remains macOS ARM64 focused debug verification. Full release/integrated
milestone, Linux, hardware power-loss, public format2 init/load admission,
profile acceptance and effective interpretation/provenance are not established.
B2B-R1 requires a fresh corrective round; approval and subsequent integrated
verification are still required before X03-B completion.
