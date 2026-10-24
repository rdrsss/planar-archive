# X06-A3 candidate evidence

Fresh candidate awaiting independent review. This report does not approve it.
Shared checkout `dev/planar-v2-vernacular`, starting/current HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
Coordinator owns the A3 round. Coder ownership is the13 files frozen in
`candidate.json`, this support/evidence subtree and exclusive serialized Debug.
No delegation, Planar operational tools/skills/roles/queue, installed binary,
release/install-release invocation, commit, installation, publication, merge or
operator-data operation occurred. No others' edits were reverted.

## Implemented boundary

`ledger/format.cpp` changes only `verify_content`, `verify_history` and one
import. Source and registry reconstruction/validation retain their original
order. Interpretation and separate-V records are loaded through the existing
bounded codec. The approved pure classifier chooses embedded16 MiB/4096 files
or effective32 MiB/8192 files; actual recorded stdlib directories are reconstructed
through `tree_core`. The approved helper independently repeats classification,
verifies raw provenance, fixed syntax, workspace and recomputed digest. This also
applies to safely unsupported interpretation. Verification returns only the
helper's repeated verified family to the event verifier.

Accepted events retain that family alongside active references. Existing noops
already require exact active references and reuse the family, without rebuilding
the same trees. Existing chronology, references, revision actors/timestamps,
request actor/expected-head/event/result correspondence and schema/codec checks
retain their ordering. Only after those checks, request_v1 and request_v2 without
profile require embedded; request_v2 with profile requires effective. Selector
presence is private intent, and its host path is never compared to portable
`profile/input.json`. Both accepted and noop mismatches refuse corruption.

`history/read.cpp` reconstructs actual stdlib bytes via existing
`history/tree::read_tree` on the original pinned `format_file` snapshot. It calls
the same helper with recorded workspace, and projects only supported owned decoded
payload. Unsupported semantics returns existing `unsupported_interpretation` with
no semantic fallback. The existing export implementation remains byte-for-byte
unchanged; known supported provenance permits exact raw source export even when
interpretation is safely unsupported. Unknown effective manifests refuse existing
format `unsupported_state`; old embedded unknown-manifest behavior remains
corruption. Ordinary malformed/resource/tree/correspondence failures map to existing
corruption; typed underlying storage failures retain their original vocabulary.
No new public diagnostic enum or SQLite-full conflation is introduced.

CMake adds only required helper dependencies for production and exact generated
format variants, history-read's existing tree/helper edges, and one test-vector
macro. The actual compiler-produced CXX.dd closure is preserved in
`no-lua-proof.json`; history.read/ledger.format reach no Lua, runtime, live profile
or closure-guard module. The existing native capture/writer dependency and pure
profile codec remain. Native undefined-symbol output has no Lua symbols.
Historical interpretation executes no current or old Lua/profile/runtime bytes.

A1/A2 pure modules, prepared acceptance, the complete transaction implementation,
load, runtime, export, schemas, CLI, init and exact external-profile contract are
unchanged. Existing exact saved profile replay now becomes readable through
verified history. Fresh profile acceptance still refuses before CAS/candidate
inspection. No same-VM preparation or new acceptance authority follows.

## SQL fixtures and actual regression

Tests extend the existing owned colocated SQL-only fixture builder. They do not
add a conversion tool or publish through a newly enabled profile writer. Both
embedded schemas retain their existing controls; profile history uses format2
with native codec2 request framing and directly published relational rows.
No fabricated valid format1/profile-history claim is made.

`red-tests.xml` preserves one actual failed success regression against the
unchanged pre-fix readers: a valid effective-v2 recorded history refused at
`open_existing`. `red-source.json` and exact copied sources bind that run; both
reader hashes equal the original full baseline. The first fixture compile attempt
incorrectly tried placing request_v2 in the codec1 variant; `red-build.log`
preserves its compiler failure. It was corrected through existing
`intern_request_v2` before the actual red run (`red-build2.log`). The same success
regression then passes after integration (`first-tests.xml`).

New integration cases cover:

- Effective-v2 recorded history, accepted revisions, noop references and all four
  history selectors; exact source export remains source-only.
- Original scratch source/registry/profile/runtime byte staging followed by removal
  before reads/replay, with no dependence on original filesystem contents.
- Unknown interpretation with supported raw tree: page/state/request and source
  export succeed, revision projection returns typed unsupported interpretation.
- Unknown effective manifest: read-only/read-write opens refuse unsupported state
  without modifying bytes/journals; export creates no destination.
- Actual raw-tree tampering, workspace/digest mismatch (including unsupported
  semantics), catalog and selected-definition forgery. Supplemental modes cover
  separate-V mismatch, malformed interpretation and guidance resource excess.
- Embedded/profile selector mismatches in both directions, for accepted and noop
  events; request-v1/effective mismatch remains corruption in format2.
- Valid effective closure with4096 embedded files and4100 complete files, proving
  actual reader dispatch selects wider fixed effective bounds. This does not claim
  impossible simultaneous component maxima or valid8192-file/32 MiB provenance.
- Saved accepted and noop profile requests replay with moved-from preparation after
  original files are removed, including identity-bound completion receipts and
  exact saved result bytes. Fresh profile ID still refuses `unsupported_state`,
  with unchanged database bytes. Existing old embedded replay tests rerun.

The first large-fixture attempt iterated raw lexical shipped order, which changed
catalog owner ordinals; `bounds-tests.xml` preserves its fixture-preparation
failure. Preserving original manifest order corrects the fixture and
`bounds-tests2.xml` passes. This was no product correction. Existing blanket
profile-history refusal control now expects corruption for mismatched embedded
provenance, retaining its name and substantive refusal assertions.

## Executed verification

| Check | Result |
| --- | --- |
| Serialized `cmake --build --preset debug --target all planar_tests` | Pass, including existing source/runtime/CLI variants; raw build logs retained. |
| Exact absolute CTest focused list and JUnit output | **711/711**, zero failures/errors/skips/disabled; **260.98 seconds**. |
| Eight added cases in that focused run | **6,001 assertions**. |
| Final supplemental affected case | **1/1**, **970 assertions**, zero failures/errors/skips/disabled. |
| Inventory versus original A2 | **918 names**, every **910** preserved, exactly8 additions. |
| Pinned formatting/Doxygen on final sources | Pass over **124 active source files**. |
| Four unchanged independent vector generators with `--check`, no bytecode writing | Canonical/hash, records, effective provenance and v2 pass. |
| Source/scope, compiled module graph, native symbols and local links | Pass; **519** local Markdown links resolve. |
| Final serialized `all planar_tests` build | `ninja: no work to do.` |

`focused-source.json` and the copied test file freeze the711-case run. After it,
three already implemented negative fixture branches were noticed absent from its
mode list. The only subsequent test change adds those three mode strings,
recorded exactly in `supplement-test.diff`. The single affected test was rebuilt
and rerun under CTest on final sources. Product and CMake hashes remain identical
to the711-case run; formatting/Doxygen and no-work were rerun on final sources.
There was no redundant broad gate for report/support changes.

The support audit initially referenced a nonexistent old export filename, then
used an overbroad substring that also matched pure profile_codec, then omitted
URL-decoding historical Markdown links. These checker issues were corrected;
preserved graph/link failure logs distinguish them from product failures.
Final checks inspect the real source and compiler graph without weakening the
no-Lua or historical-body requirements. No private source compilation was used.

## Frozen protection and limits

One original full-content baseline and one final full-content inventory cover the
whole checkout, including root dotfiles, symlink identities/targets, prior A1/A2
author/reviewer evidence and all build trees. Only `.git`, this A3 support/evidence
subtree and the exact coordinator `V5-02-X06-A3.md` are exempt. Candidate files and
Debug outputs are the only allowed changes. Protected files/links retain exact
hash/stat identity; directory membership and mode/device/inode remain exact,
with ordinary ancestor directory timestamp changes recorded separately.
Preedit copies match the original baseline. `protection-result.json` records the
comparison; `artifact-manifest.json` binds candidate, report, raw evidence,
protection and support files without a hash cycle. AGENTS.md remains the original
symlink to CLAUDE.md; compile_commands.json retains its original symlink identity.

No full918-case configuration gate, release/Linux gate, profile preparation or
fresh profile transactions, whole-invocation shared budget, RSS guarantee, public
profile CLI or mandatory external-profile completion is claimed. Latest complete
approved macOS gate remains C5's876/configuration. Public init creates format1;
later X06, X07, X08, execution/RA01 and Linux remain open. Fresh independent
review is required.

Debug and all coder-owned files are released; no build/test process remains.
