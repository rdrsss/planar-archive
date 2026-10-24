# R14-D8 verification author report

Candidate only; independent review is required. Work used the shared
`dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional rewrite.
No product C++/Lua, storage schema/codec, W recipe, fixture golden, old generator
or prior evidence was changed. No Planar tools/skills/queue, installed binary,
operator data, commits, install, publication or merge was used.

## Change and exact ownership

- Extended `tests/acceptance/bootstrap_load.py` with public initial/noop/comment/
  prose/semantic/restore chronology and original historical reads/export after
  workspace/registry removal under both ordinary and changed-current-policy test
  binaries. Each saved load result is exact native stored bytes; revisions,
  parents, event IDs, requests, provenance references and W relationships are
  asserted. Prose render bytes may change with W stable. Export checks exact
  original closure bytes and executable/nonexecutable file modes; an unrelated
  sibling is excluded. Initial same-ID response replays byte-exactly after removal
  and advanced head. Existing grammar/refusal/concurrency/late-replay/capture/
  lost-response cases are retained.
- Added `tests/native/r14-d8-support/acceptance_fault.t.cpp`, reusing the existing
  native acceptance fixture setup and scoped VFS/child-process harness pattern.
  Real `complete_acceptance` receives SQLITE_FULL at every actual selected VFS
  write (38 journal positions and 15 main-database positions), then an unfaulted
  end-of-enumeration call succeeds. Each failure checks all seven table counts,
  verified head/revision/event/request state and same-key retry/replay with no
  duplicate rows. Five finite process-death sections bracket first journal
  write/sync, first database write/sync and returned completion.
- Minimal `src/engine/CMakeLists.txt` wiring adds the new test source to the
  existing test-only `planar_ledger_format_tests`; default all remains unchanged.
  No new runtime hook or product callback exists.
- Added an R14 acceptance evidence table and scoped documentation links. The
  coordinator owns the round record. `freeze.py`, `gate.py` and `run.py` are
  verification/evidence helpers only.

## Explicit crash-recovery limit

Before first journal write and first journal sync, the main database is exactly
its original bytes but a non-hot journal remains. Ordinary opening and same-key
retry refuse `recovery_required`; `recover_quiescent` refuses `corrupt` with
`Recovery requires a valid hot journal header`. Refusals preserve every scratch
file's bytes; no sidecar is manually deleted and raw SQLite is not used to open
these originals. No supported automated recovery/retry is demonstrated for these
two boundaries. This follows the conservative ledger-contract initialization/
failure section (lines 494–499) and R12-C explicit recovery boundary (lines 43–49).

At real database write/sync interruption, explicit recognized-hot-journal recovery
returns the old whole acceptance. After returned completion, the whole committed
acceptance is readable and exact saved bytes replay. Each supported retry leaves
one revision, one request and two events including initialization. These are
native VFS/process proofs, not public CLI disk-full/crash injection or hardware
power-loss evidence. The independent reviewer must assess this contract-conformant
safety boundary rather than infer universal recovery.

## Final verification

- Debug full: **402/402, zero failures/skips**, 193.69 seconds.
- Release full: **402/402, zero failures/skips**, 100.07 seconds.
- Both final inventories preserve all **400** independently reviewed D7-B names
  and add exactly the two D8 native tests. JUnit and LastTest were archived before
  inventory calls. New focused native tests and public CLI workflow passed before
  the full gates; final full runs contain the final source content.
- Both configurations: pinned formatting/Doxygen for 95 active source files;
  explicit pinned formatting for the new external test source; 12 hand byte /
  20 upstream / 41 canonical-domain vectors and ten independent record vectors;
  all seven historical variant families and fixed-export exact-source harness,
  six D6 variants, C01 and full D7 probe/original/separator/assembly audits.
- Final debug `debug-no-work-repeat.log` and release `release-no-work.log` show
  genuine no-work all+planar_tests builds. The first debug check rebuilt after a
  transient logging-string edit was reversed byte-identically during the active
  full run. The coordinator prohibited further active-run input edits. Exact
  substitutions, restored mtime/hash and reconstructed intermediate-hash limit
  are disclosed in `transient-noop-edit.json`; the rebuilt object retains the
  original string and excludes the transient string. The timestamp-only rebuild
  did not change final tested source content; no extra full behavioral rerun is
  claimed. `gate.py debug` failed only that first no-work assertion; its full
  tests, lint and audits passed. `gate.py release` completed successfully.

`commands.json` records direct argv, UTC start/end and status for every recorded
command. Retained failed/zero-match attempts are described in `failed-attempts.md`;
none is counted as success. Test stdout can contain repeated earlier process
lines due to inherited fork stdio buffering; exactly five sections are covered.

## Freeze and preservation

`baseline.json` is the full contemporaneous repository-file hash inventory before
implementation; `starting-head.txt` and `starting-status.txt` preserve the existing
checkout. The three initially relevant existing source files were copied before
editing. Documentation baseline copies were reconstructed after append-only edits
and verified byte-for-byte against those contemporaneous hashes; this distinction
is explicit in `baseline-snapshot-note.json`. New files have no prior snapshot.

`candidate.json` uses repository-relative paths and hashes every owned final input;
`candidate/` preserves those exact bytes and `owned.diff` compares them against
verified baseline bytes. `protection.json` checks every preexisting inventoried
file and allows only assigned changes plus the coordinator-owned D8 round record.
All prior evidence and goldens remain unchanged. `artifacts.json` hashes both
product binaries, native acceptance test executables, changed-policy CLI probes,
compile databases and CMake caches. `evidence-sha256.json` is evidence-directory-
relative and excludes itself. `verification.json` records counts, inventory
preservation, final no-work paths and limitations.

Verification is macOS ARM64 / pinned LLVM 23.1.4 only. Linux, universal recovery,
hardware power loss, external-profile X, R15+ execution and RA01 authoring remain
open. No approval is claimed. Debug and release build directories are released
idle for fresh independent review.
