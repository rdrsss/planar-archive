# X05-A3-C02 — Original module-seal locations

Candidate pending fresh independent combined A3+C01+C02 review. This correction
addresses C01-review R4 without changing refusal policy, public runtime API,
shipped Lua, CMake, vendor, pure codecs or historical evidence. The shared checkout
is `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`; all pre-existing rewrite changes are
intentional and retained. Authority and ownership are in the coordinator's
[C02 round](../../../docs/vernacular/rounds/V5-02-X05-A3-C02.md).

## Correction

`closure_guard::audit_roots_detailed` returns the existing failure category plus
an optional zero-based position in the original root span. Selection occurs
immediately before the existing `audit.graph(root, 1, true)` call, under the same
single inspector, identity/prototype memo, traversal order and graph-depth rules.
The enum-only API delegates to that traversal. Preflight failures have no root;
known-root refusals and caught native allocation failures retain the selected
position. Both typed and unknown exceptions restore the original Lua top; unknown
exceptions still propagate. Cumulative PMR accounting remains unchanged.

Only module sealing opts into location mapping. Its retained D/F root pairs map
to the original per-kind catalog index, counting earlier modules and the five
shipped commands. Location formatting is inside native diagnostic/bad_alloc
handling and finishes before allocating Lua calls; only trivial mapping state
crosses such calls. Before root selection and before identity-set union the
location is the active `profile.modules[N]`. Each actual freezer root uses the
same original-entry mapping. Initial embedded/environment and private repeated
seals do not acquire unrelated external-entry locations. The existing all-root
audit-before-any-freeze ordering, terminal failure state and atomic successful
identity-set publication remain intact.

The module-seal callback now checks terminal state before changing location or
pushing roots. Development `dev-locations-2` intentionally retried that private
callback and exposed a test-only reset from the first freeze field to the module
boundary before terminal refusal. Its complete source/output remain preserved.
`dev-locations-3` used `seal_new_roots` to isolate terminal refusal; the final
candidate and `dev-locations-4` restore the stronger actual module-seal retry,
which now preserves the first failure field. The public constructor discards the
failed owner and does not expose a retry API.

## Regression and controlled-path evidence

`red-writer-first.log` and `red-native-first.log` run the immutable review's
unchanged-production ordinary-runtime binaries before edits. Both exit1,
reproducing shape1 being attributed to valid shape2 or valid projection1. Exact
commands are in `red-commands.json`; the original review sources/binaries remain
immutable.

The new ordinary-runtime matrix tests one unsafe retained writer or direct
native callable among otherwise valid registrations. Its 36 combinations cover
early/middle/late original positions, all three descriptor kinds including
commands, interleaved kinds, and a second module with prior entries of every
kind. Every assertion requires the existing exact code, redacted message and
one-based original field. New guard cases cover selected roots, enum-wrapper
equivalence, pre-root absence, bad_alloc classification, unknown exception
restoration, and shared memo/report semantics. Existing active-frame tests now
exercise the detailed path, including preflight resource refusal without a root;
old depth/memo tests still exercise the wrapper.

`locations.py` makes reversible private production copies. The pre-root case
removes the native catalog only after ordinary environment sealing, causing the
real guard metadata preflight to refuse before selecting a root. The partial
freeze case injects Lua error immediately after an actual metatable installation
on shape1; a valid projection follows. The union case injects Lua error after an
actual insertion into the private identity-set union. These are explicit
controlled faults, not natural exhaustion. Each checks exact diagnostic location,
unchanged published identity-set identity, terminal same-owner module-seal retry
and zero tracked live Lua allocation after destruction. A success control runs
the same instrumented production copy without faults. Transformations include
source/candidate hashes, exact substitutions and verified reverse equality.

A2's retained repeated-root runner is copied into new support, with only its
output destination, the added private seal_roots parameter, and its outdated
create error injection adapted to the current implementation. It throws the same
native diagnostic inside create_impl rather than returning an old incompatible
expected type. Historical generators remain untouched. The retained A3/C01
production, reachable-native, corrective and budget runners likewise write only
to C02 evidence. Their individual reversible deltas remain in each output root.

Development failures remain: `dev-locations-1` was an overly broad text anchor;
`dev-locations-2` exposed the private retry field reset described above;
`dev-repeat-1` failed compilation because its inherited A2 fixture returned the
obsolete diagnostic expected type. `dev-repeat-2` and `dev-locations-4` pass.
`dev-repeat-1` private compilation began before `dev-build-2` completion was
confirmed; that development scheduling mistake is recorded, not claimed as a
serialized gate. Final gate commands await completion sequentially.

## Verification and identities

Final results, immutable candidate identities and release checks are recorded
below after the serialized gate. No full 577-test run, release/RelWithDebInfo or
Linux gate is claimed. Latest full gate remains 520 per configuration. Independent
combined approval, public profile CLI, B/C semantics/adapters, X06 storage/history
interpretation, caller orchestration, mandatory external-profile gate, cutover,
execution/RA01 and Linux remain open. Public init creates format1.

`final-1` preserved all passing focused/probe/vector/audit output but stopped at
pinned formatting. Two continuation lines in the added test needed a second
formatter pass. `formatting-correction.json` proves before/after line equality
when only leading indentation is removed, and records the passing dry-run
idempotence check. `final-2` refreezes that correction and reruns the entire scope
gate; no failed run is presented as a clean gate.

Manifest schemas: `final-2/source-freeze.json`, `source-after.json` and
`source-release.json` map repository-relative regular-file paths to `{sha256,
mode}` and symlinks to `{symlink,mode}`. Directory entries are not recorded.
They include the complete source entry set except root `.git`, root `build`, and
this C02 evidence root. All historical evidence, shipped Lua/vendor and the
AGENTS.md symlink are therefore covered. `final-2/owned.diff` compares owned
candidate source with C01 starting copies; `scope-audit.json` lists allowed
changes. `final-2/build-identities.json` maps build-relative object, PCM, archive,
selected executable/configuration paths to SHA256. `artifact-manifest.json` maps
all evidence files except itself and finalization.json to SHA256.
`finalization.json` binds report, manifest, candidate/diff and release identities;
its explicit exclusions avoid recursive hashing. No post-release writes follow
finalization.

The serialized final-2 gate passes all 21 commands. Debug inventory
is 577 unique names preserving all 573 prior names; focused execution is
204/204 with zero failures/errors/skips/disabled cases. The new matrix has 36
ordinary-runtime combinations. The repeated-root suite passes ten scenarios plus
its deliberate red control. Production and reachable-native suites each pass
eight scenarios plus their red control; retained corrective and high/exact/short
budget probes pass. Four vector checks, four historical exact-source audits,
A0 forward/reverse/exact-shipped checks, pinned formatting/Doxygen for 117 active
sources and a no-work all+planar_tests build pass. The two immutable R4 detector
sources are recompiled against the candidate in ordinary-green: both now exit0.

Source freeze, after-gate and release compare 20,440 entries with zero
mismatches; 931 build identities also match at release. Exact identities:

- Source freeze/release: `8beb5181756f4fc31f603c3eceda489f835b736c91b076abbc6ffd6bebfeac46`.
- Owned candidate diff: `a749522841f12e0e5eb90b99023807eefe18ebcc3055dba47d5f5615033cbece`.
- Build identities: `8fb391b2dd86317d7e80a801962334126752f9d098479ab47c5400950766da8c`.

Artifact/report hashes and artifact count are in finalization.json. Serialized
build ownership is released on completion of final hashing; no further source,
build or evidence edits follow this release. This is a candidate, not self-approval.
