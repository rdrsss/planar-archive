# X06-A2 candidate evidence

Candidate awaiting fresh independent review; this report does not approve it.
Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
The exact 14-file source/reference snapshot is `candidate.json`; `candidate.diff`
reconstructs changes from the frozen preedit copies. Work used only the assigned
serialized Debug directory. No Planar operational tools, agent delegation,
installed binary, release/install-release command, commit, installation,
publication, merge or operator-data access occurred. No others' edits were reverted.

## Scope and deterministic order

The new pure `history/recorded_interpretation` module is a prerequisite; actual
ledger integrity/history/replay/export dispatch and acceptance are unchanged.
There is no ledger-format parameter or dependency on `ledger.format` or
`history.tree`. Fixed interpretation readers apply independently of schema1/2;
writer restrictions are separate. The native link proof records pure transitive
`ledger_types` vocabulary without storage execution, Lua/runtime or native capture.

`classify_recorded_interpretation` first preflights **both** encoded byte lengths
at 16 MiB, then canonical-decodes interpretation followed by separate versions,
with depth256 and four-million-node limits. Presence of `extensions`, or
`provenance`/`registration` in either embedded or separate versions, chooses the
effective family. Presence checks safely ignore non-record values. Numeric
version alone never selects effective: a future v1-shaped envelope remains on
the original decoder. This hint verifies neither schema nor provenance and is
never an admission token. Wrong-kind markers still select effective bounds but
cannot grant authority.

`verify_recorded_interpretation` repeats classification internally, releasing its
classification values before provenance work. It accepts complete actual raw
files plus expected workspace, never a supplied digest/profile/prepared aggregate.
Its order is:

1. Both size preflights, interpretation canonical decode, separate-V canonical
   decode and family classification.
2. Fixed family file count, then shared tree bounds/topology/digest computation
   (embedded16 MiB/4096, effective32 MiB/8192). No per-file view allocation occurs
   before count admission. Tree resource failures remain resource; invalid topology
   refuses before manifest or interpretation semantics.
3. Independent provenance verification. Embedded uses unchanged `verify_stdlib`,
   retaining its typed unsupported-manifest behavior. Effective first checks its
   canonical manifest with 1 MiB/depth16/24000-node limits, exact eight outer
   keys/kinds, at most64 exact module entries with portable external paths and
   normalized modes, and exact catalog container keys/kinds. Malformed preflight
   fields precede unsupported manifest version/scheme. This is only a bounded
   envelope preflight; it does not reinterpret future catalogs or descriptors.
   Known manifests immediately use unchanged `verify_effective_stdlib` for full
   membership/profile/module/embedded/catalog/owner correspondence. Other effective
   structural failures map to invalid_tree; resource stays distinct.
4. Original fixed v1 decoder, or the approved A1 mandatory-rawtree v2 decoder.
   Existing fixed syntax, B/S/E structure, separate-V equality and unsupported
   ordering remain in those decoders. A1 independently verifies the raw files
   again and checks catalog, digest and selected-shape identities.
5. Exact recorded workspace ID and recorded raw-tree digest equality against
   expected workspace and independently recomputed provenance. This applies even
   when the fixed decoder returns unsupported interpretation.

Malformed fixed syntax or separate-V mismatch precedes interpretation unsupported;
provenance refusal precedes either. Unsupported interpretation with wrong workspace
or matching forged digest claims against different raw files refuses correspondence.
Unknown manifest refuses entirely; no exportable or semantic result escapes.
Supported provenance with well-formed unknown interpretation returns a distinct
`unsupported_interpretation` status and **no semantic payload**. After the fixed
unsupported check, bounded canonical values are decoded again only to inspect
already syntax-checked workspace and digest; those values die locally. Later A3
may use that status for raw source export, but A2 wires no reader/export path.
Unknown extra fields remain closed through the original fixed structure checks.

Supported results own canonical values and exact bytes, sever input aliases and
retain no raw-file views. Repeated bounded decoding, provenance copies and A1 raw
verification are deliberate, with no new shared-budget or RSS guarantee. Ordinary
results do not prove historical semantic honesty, callable code, same-VM selection
or acceptance authority. All new public operations catch native bad_alloc and
preserve canonical/lower typed resource failures.

## Preserved regression and scoped prerequisite correction

The original v1 decoder collapsed failure of its embedded-versions canonical
encode into malformed. The first 15-case run preserved in `first-tests.xml`
records **18 failed allocation assertions**, with all other functional cases
passing. `first-test-source.json` and three source copies freeze that candidate.
`diagnostic-tests.xml` records the same18 failures with the first at enclosing
embedded verify allocation1883/1904; `diagnostic-test.cpp` and its hash are retained.

A direct old-decoder regression was then added and run against the still unchanged
original lower source: `direct-red-tests.xml` records **18 failed assertions**, first
allocation880/900. The direct red source/test and identities are preserved. The
original lower SHA256 is
`08c995a8b62ee2c4f26a42d4c4d5f685572563db023f3f3d5ced73c226682497`,
checked against the original baseline before its one-time preedit copy.

The coordinator authorized exactly one prerequisite adapter in
`interpretation_codec.cpp`: failed embedded-V canonical encoding now forwards
through existing `codec_failure`, before the unchanged exact mismatch check.
Ordinary malformed/equality/unsupported precedence, supported canonical bytes and public API
remain unchanged. The source audit checks the complete file against that exact
single replacement. No heuristic retry or best-effort decoder fallback was added.
Final exhaustive top-level and direct regression sweeps pass. The direct lower
API retains original raw bad_alloc sites; enclosing new APIs convert them. These
controlled one-shot sweeps make no natural host-exhaustion claim.

The initial preedit-copy attempt lacked its destination directory and failed
before mutation; the running build was allowed to finish before copying/applying
the adapter. `precorrection-all-build.log` preserves that pre-correction full
build. No build input was edited while any build was active.

## Executed verification

| Check | Result |
| --- | --- |
| `cmake --build --preset debug --target planar_history_recorded_interpretation_tests` and later `all planar_tests` | Pass. Final combined build in `final-build.log`. |
| CTest with exact absolute `focused-names.txt`, JUnit output | **106/106**, zero failures/errors/skips/disabled; **62.00 seconds**. |
| New helper cases within that gate | **18 cases / 14,193 assertions**. |
| JSON inventory versus A1 | **910 names**, all **892** preserved, exactly18 additions. |
| Pinned formatter/Doxygen via `scripts/rewrite-check.py lint` | Pass over **124 active source files**. |
| Four existing vector generators, each `--check` with bytecode writing disabled | Canonical/hash, record, effective provenance and A1 v2 vectors pass unchanged. |
| Source/native graph and undefined-symbol audit | Pass; no Lua/runtime/storage/native-capture execution dependency. Actual native link command is retained. |
| Candidate Markdown local links | **517** resolve. |
| Final serialized `all planar_tests` build | `ninja: no work to do.` |

New tests cover original v1 supported/future/malformed controls; both supported
families; future numeric versions versus markers; wrong-kind/missing/extra markers;
unknown interpretation with changed valid raw module bytes or missing raw members;
workspace/digest/versions mismatch; unknown embedded/effective manifests and paired
failure precedence; future extra fields; ownership after destroying inputs;
allocation refusal and direct old-codec regression. Existing focused coverage
retains canonical/record/tree/catalog/provenance/v1/v2 vectors and structural tests.

Valid embedded provenance reaches the **exact16 MiB aggregate** and **4096 files**
with successful decode; one extra byte refuses. Separate synthetic topology
controls reach embedded16/effective32 MiB and4096/8192 file outer boundaries before
missing-manifest refusal, and exceed each ceiling by one. These latter controls
are deliberately invalid provenance, not claims of valid maximum effective input.
Effective component ceilings permit at most4162 valid complete files
(4096 embedded +64 external + profile + effective manifest); its16 MiB embedded,
8 MiB external,64 KiB profile and1 MiB manifest ceilings also prevent valid full
32 MiB aggregate input. The four-million canonical-node bound is inherited from
explicit fixed limits and the focused canonical suite; no new simultaneous-maxima
or whole-process-memory claim is made.

## Frozen protection and release

Full-content baseline/final inventories include every prior evidence file, root
dotfile, symlink identity/target and all build trees. Only `.git`, this A2 evidence/
support subtree and the **exact coordinator A2 round** are exempt. The protection
checker allows only the14 candidate files and Debug outputs to change, compares
all protected file/link stat identities plus content, checks directory membership
and original mode/device/inode, and records ordinary ancestor directory timestamps
separately. Prior A1 author/review evidence and release/install-release are protected.
Preedit copies match the original baseline. Candidate hashes are rechecked after
the final full hash; `protection-result.json` and `artifact-manifest.json` bind the
candidate, report, raw evidence, protection and support scripts without a hash cycle.

No full910-case configuration gate, release/Linux, actual reader dispatch, profile
preparation/transactions/shared accounting, public CLI or mandatory external-profile
completion is claimed. Latest complete approved macOS gate remains C5's876 per
configuration. Public init creates format1. A3, later X06, X07, X08, execution/RA01
and Linux remain open. Fresh independent review is required.

Debug and all coder-owned files are released; no build/test process remains.
