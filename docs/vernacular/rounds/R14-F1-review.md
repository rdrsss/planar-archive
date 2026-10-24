# R14-F1 independent native export review

Disposition: **approve**. No blocking findings or unresolved product choices.
Reviewed by fresh `/root/r14f1_reviewer` on the shared
`dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The existing dirty rewrite was
intentional and retained. Product files were read-only throughout this review;
writes were confined to this report and `tests/native/r14f1-review-evidence/`.

## Exact candidate and evidence

- The nine-file candidate manifest
  `tests/native/r14f1-evidence/candidate-files.json` has SHA256
  `f287ddaa8809b93d65b87fd066253a2fed1819bdc27d43b2a35c089eaa4dd12d`.
  All nine current files matched independently at entry and finalization.
- The final 45-entry implementation evidence manifest
  `tests/native/r14f1-evidence/evidence-sha256.json` has SHA256
  `d2d092ad7969fb258c09e5627211321bfa30f1fea3acb565928bd4f539319774`.
  Every folder-relative entry matched. This supersedes the earlier evidence
  manifest identity following the README-only spacing cleanup.
- Independent inventory verification retained all 4992 protected files, the
  `AGENTS.md -> CLAUDE.md` symlink and all 317 previous registered test names.
  There are 327 current tests, including ten export cases. Each of the three
  older generated variants and the new export variant matched fresh generation
  byte-for-byte and reported exact reversal. Reviewer additions are outside
  that implementation-owned candidate.

## Contract assessment

`source_export.cpp:263` obtains one verified read-only ledger owner; the retained
transaction begins at `ledger/format.cpp:1121`. Revision lookup, source-record
lookup and complete source-profile tree reconstruction use that owner before
any target-side creation. `source_export.cpp:150` builds all ancestor/file
receipts before materialization. Unsupported interpretation is irrelevant to
this source read. Source-only membership, empty root, byte/mode fidelity and old/
change/revert revision behavior have actual native coverage.

`source_export.cpp:56` traverses original parent components with no-follow
relative opens, rejects invalid final spellings/NUL, and holds the real parent.
The existing-entry check is followed by a true platform no-replace operation at
`:178`; there is no overwrite fallback. The macOS SDK declaration/RENAME_EXCL
constant is recorded in implementation evidence. Linux uses guarded renameat2
syscall support and returns an I/O refusal if unavailable.

`source_export.cpp:191` creates only exclusive generated staging and validated
source entries, uses exclusive/no-follow file opens, handles short writes and
EINTR, rejects zero progress, applies final file modes before file sync, then
sets/syncs directories in reverse creation order. Traversal retains descriptors
by depth rather than tree size. The full-profile test actually exports 4096
files/8191 directories under RLIMIT_NOFILE 128; a separate depth-64 export runs
under that same ceiling. Existing tree tests own excessive-profile refusals.

`source_export.cpp:118` checks stage/ancestor/entry identities, removes only
identified owned entries, and retains replaced or unrecognized entries.
Cleanup is nonallocating/noexcept and is also invoked by RAII. Allocation needed
for the result, location token and entry plan precedes staging; a post-creation
exception enters cleanup. After rename, `:316` marks publication immediately;
parent-sync/close/exception failure cannot enter target cleanup. Close errors on
required materialization/sync handles refuse success. The native API preserves
storage categories and exposes only the generated basename for incomplete
cleanup, with no source contents or caller paths in new messages.

The injected matrix really checks prepublication absence/cleanup, complete
postpublication preservation, empty arriving-target preservation, replaced
file/directory/root and unowned-entry retention. Termination uses subprocess
`_exit` immediately before rename and at post-rename parent sync, then verifies
absent-or-complete targets and nonadoption of leftovers. It is not a hardware
power-loss experiment. The unsafe-rename mutation genuinely fails the empty
arrival test (exit42); the restored exact generated variant passes it. The full
327-test gate preceded only a test/helper strengthening; candidate comparison
confirms unchanged product/module/CMake/docs. Final strengthened export tests and
active/helper lint were rerun, so repeating the full gate was unnecessary.

## Independent verification

Executed serially while holding the debug build directory:

1. Manifest and protected inventory checks; fresh exact/reversible generation of
   all four variants; registered-test inventory comparison. Results recorded in
   `manifest-verification.json` and `independent-audit.json`.
2. `ctest --test-dir build/rewrite-debug -R 'replacement.source export'
   --output-on-failure --output-junit <review-evidence>/export-suite.xml`:
   ten matched, ten passed, zero skips, including full breadth/depth/fault cases.
3. `python3 tests/native/r14f1-review-evidence/build-probe.py`, then the compiled
   reviewer harness with filter `'[reviewer],[reviewer-umask]'`: three matched
   cases, 72 assertions, zero failures/skips. The harness links the unchanged
   production module using its existing pinned compilation/link configuration.
   It independently checks embedded NUL, symlink/.., actual-directory/..,
   malformed ledger before target creation, and 100 success plus 100 existing-
   target calls under RLIMIT_NOFILE 48 without descriptor exhaustion.
4. `cmake --build build/rewrite-debug --target all planar_tests`:
   `ninja: no work to do.`

Raw implementation gate evidence was inspected: 327 passes/zero skips, all three
CLI suites, 84-source formatting/Doxygen, unchanged render/record vectors and a
no-work build. No redundant full gate was run by the reviewer.

## Permission observation and limits

The independent harness also verifies exact final modes under umask0077 and0177.
Umask0777 removes initial owner access from mkdir: the stage cannot be reopened.
The operation honestly returns cleanup_incomplete, leaves no target and carries
only its exact generated `.planar-export-<32 lowercase hex>` basename; one empty
mode0000 stage remains. The test restores its mode only after observing and
asserting that state, to dispose of its own scratch fixture. Removing that known
empty root without reopening could reduce leftovers, but conservative refusal is
not a blocker: the contract permits explicit incomplete cleanup and does not
require successful export despite denied directory access. Product code does
not alter the process-global umask.

This approval covers native F1 only. It does not claim an export CLI, encoded
response/stdout failure recovery, load membership policy, release verification,
Linux compilation/execution, hostile concurrent directory-owner safety or hardware
power-loss durability. Tests ran on macOS ARM64. Actual allocator exhaustion was
not induced; a std::bad_alloc at a materialization write boundary exercises
post-creation exception cleanup. All tests use scratch ledgers; no legacy tools,
operator state, installed binary, installation, commit, publication or merge
was used. Debug ownership is released to the coordinator after this report.
