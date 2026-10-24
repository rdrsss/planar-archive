# X04-A2 native profile capture candidate

Coder candidate awaiting independent review; no self-approval. Shared checkout
`dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No Planar tools/skills/queue,
installed binary, operator data, commit, install, publication or merge was used.
Coordinator owns `docs/vernacular/rounds/V5-02-X04-A2.md`; its concurrent recorded
brief/disposition updates are explicitly accounted for in the ownership audit.

## Delivered boundary and ordering

`planar.engine.capture.profile` provides move-only `profile_capture::open` from
an explicit absolute selector, `snapshot`, staged `admit_root`/`admit_file`/
`admit_target`, metadata-only `verify_file`, and repeated `verify`. A separate
`profile_error` carries the existing native diagnostic and only logical `profile`
or one-based `profile.modules[N]`. The immutable closure retains original profile
bytes/mode under logical `profile.json`, and raw module bytes/modes in exact list
order. Host filenames can contain spaces/Unicode; module paths stay portable.

Open captures the trusted profile, parses and validates the entire module list,
then metadata-selects **every** listed module before reading **any** module. Only
complete capture/sealing returns an owner with staged supplied-input methods.
This is the closure-first order required by X-CAP. No supplied content is consumed
by admission or descriptor verification. The original A1 admission and native
acceptance/content receipt owners both remain alive for repeated seals and move
transfer. Failure poisons the profile owner; moved sources refuse; self-move
preserves ownership. Previously returned immutable bytes remain unchanged even
when their owner later refuses. Unrelated sibling files are never enumerated.

No Lua, external registration, CLI flag, provenance/interpretation codec or ledger
acceptance is wired here. Known shipped slots and opaque late slots can use these
native seams; their actual X05/X07 phase integration remains future work.

## Parser and byte limits

The profile-specific Glaze adapter retains decoded duplicate keys and validated
raw number views. Glaze owns JSON, numeric grammar, UTF-8, escapes and surrogate
handling. Its strict `validate_json` pass handles the inherited pair-range
reader's permissive separator behavior. Node callbacks bound depth 4/nodes 132;
Glaze's `try_emplace_back` hook checks local array 64 and remaining node budget
before constructing elements or growing containers. Schema rejects unknown or
duplicate keys, wrong containers, nulls, fractional/exponent versions, trailing
bytes, comments, BOM and invalid/non-NUL UTF-8. Any valid integer version except
exact 1 is unsupported_version, including integers larger than INT32/INT64/UINT64
and a 65000-digit token; canonical/int32 JSON codecs are unchanged.

The pinned JSON string reader does **not** implement the documented runtime
`max_string_length` context field (BEVE/CBOR use it). The approved workaround
reads a library raw token without allocation, bounds its body to 6144 bytes, then
uses the ordinary Glaze string_t decoder with a fixed 6152-byte scratch object.
Every resize/short-string assign checks capacity before writing. Glaze's raw
intermediate length plus its 8-byte padding fits this fixed storage; unpadded
raw token views use the library's checked tail path. Decoded length <=1024 is
checked before constructing retained strings. Proof of 6144: each decoded byte
requires at most 6 raw JSON bytes (`\u00XX`); surrogate pairs are 12 raw bytes for
4 decoded UTF-8 bytes and therefore smaller expansion. Exact escaped 1024-byte
paths, 1024/1025-byte keys, surrogate pairs and invalid escapes exercise the real
library path. No escape/JSON/number grammar was implemented by hand.

Portable spelling reuses `valid_path` (1024 bytes/128-byte segments), with exact
`.lua` suffix, reserved top segments and decoded duplicate/case-folded prefix
aliases checked before module selection. Physical aliases, symlinks and ancestor
changes use A1 admission. All module bytes must be non-NUL UTF-8 source, and
initial ESC bytecode is refused before publication.

`native_capture_session::read_bounded` binds the already opened no-follow
file descriptor to A1 metadata before checking descriptor size against caller
per-read and remaining aggregate ceilings, and before allocating/reading bytes.
Profile 65536/65537, module 1 MiB/+1 and aggregate 8 MiB/+1 tests observe no read of the
refused inode. Cached reads apply the new bound too. Bounded receipts retain the
fact that rereads must not exceed the original charged size; both final sealing
and acceptance verification reject growth before allocation/read. Old read
entry points retain their old cache behavior and inherited quotas unless their
receipt is explicitly upgraded by a later bounded read.

These are local profile/module bounds. Existing A1 metadata and native content
ceilings remain separate. Sharing 64 MiB/10000 across a complete later invocation
with untrusted services is **not** proven or implemented here. Ceilings are not
RSS, descriptor-capacity or hostile-writer guarantees.

## Verification and retained failures

Authoritative source is frozen in `passes/11-final-docs/candidate-source/` and
its `source-freeze.json`. No source changed during its commands. `commands.json`
records exact commands, exits, UTC starts and durations.

- Pass09 debug `all planar_tests` builds; inventory 488 preserves all 451 A1 names
  and adds 37 profile cases. Focused profile/capture/path/acceptance:162/162 pass,
  zero failures/errors/skips. This is not a full 488-test execution claim.
- Pass09 canonical, record-v1 and request-v2 vectors, four inherited runtime/CLI
  audits and five generated storage/export exact forward/reverse audits pass.
- Pass09/10 formatting passes but Doxygen fails on undocumented internal adapter
  declarations/members. Pass10 also repeats 162/162 successfully; both complete
  raw outputs and freezes remain, with no overwrite of earlier successful tests.
- Pass11 corrects documentation only. `comment-only.diff` and token identity proof
  preserve all C++ string/character tokens and discard only comments/whitespace,
  binding the final source to pass09's passing tests and audits. Other previously
  frozen sources are byte-identical. Pass11 debug build, equal 488-name inventory,
  pinned formatting+Doxygen 104 active sources and final no-work build all pass.
  Build identities bind the final binary, tests, observer and changed objects.

Initial attempts01/02 are compilation failures (adapter name qualification,
custom quoted-key recognition and short-string assign requirements). Attempt01
has no pre-command source hash freeze and is excluded from acceptance evidence.
Attempt03's 37-name broad regex records 2 fixture failures: incorrect expected
logical fields, an absolute long-path ofstream beyond macOS path length, and
path generator underflow. The fixture now uses descriptor-relative final writes;
those corrections did not change product behavior. Attempts04/05 pass 29 profile
cases. Attempt08 passes all expanded 37 profile cases. XML initially emitted under
the build directory for attempts03–07 is copied here without alteration; final
runners use absolute output paths.

A real new-seam regression is retained: attempt06's exact-one-test run grows a
bounded raw module during the final verification seam and observes content reads
above its original quota before refusal. Attempt07 passes the same test after the
bounded-receipt fix. `growth-regression-before-native-exact.cpp` and
`growth-regression-before-profile.t.cpp` reconstruct the pre-fix sources and match
the attempt06 frozen hashes exactly. Initial incomplete reconstruction files are
retained as explicitly mismatched exploratory evidence. Final 37-case coverage
also checks the retained acceptance reread and cached-receipt upgrade.

The unchanged A1 observer is linked narrowly to profile tests, with an explicit
watched module positive control before metadata-order refusal tests, profile
read positive control, and supplied observation read positive control. Assertions
prove zero watched read attempts on full-list/metadata selection refusals, byte
quota refusals, changed descriptor binding and metadata-only admission/checks.
This is positive-controlled read/pread/readv/mmap API observation, not universal
kernel tracing. Linux observer branches remain unexecuted.

## Protection and release

`baseline-filesystem.json` freezes regular-file hashes/modes and symlink targets
before product edits. Only root `.git/`, root `build/` and this new evidence
subtree are excluded. Nested directories with those names stay protected.
Starting status/diff/HEAD/branch and existing owned snapshots are retained.
Final filesystem, ownership audit, candidate manifest and owned delta bind the
released candidate, preserve AGENTS.md as the CLAUDE.md symlink, and permit only
explicit ownership plus the coordinator's round-record delta. Artifact manifests
hash every retained output except themselves. No source/evidence edit follows
ownership release. Only macOS ARM64 debug ran here; full debug/release integration
and independent X04-A review remain required.
