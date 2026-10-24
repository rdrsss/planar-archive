# X03-B2-B-C01 implementation handoff

Candidate pending independent review; this report is not approval. Starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, shared checkout
`<repo>`, branch `dev/planar-v2-vernacular`.
The intentional existing rewrite and all historical B2-B author/reviewer evidence
are preserved. No Planar tools/skills/queue, installed binary, operator data,
subagents, commits, install, publication or merge were used.

## Correction

The only product change is `src/engine/ledger/format.cpp`: move the existing
private `same_metadata` definition before its callers and reuse its complete
identity/workspace/creation-time/format comparison at locked `begin_records`,
`record_writer::commit`, and acceptance precommit. Refusal codes, messages,
validation order, replay/CAS, receipts and recovery are unchanged. The already
complete writable-open comparison is retained. No public API, schema, CLI or
format conversion was added.

`src/engine/CMakeLists.txt` wires a verification-only format variant and three
new tests, excluded from default `all` and included in `planar_tests`. The three
new support files live in `../v5-02-x03-b2-b-c01-support/`. Seven status documents
point to the correction round and retain pending-review/integrated-gate status.
The coordinator-owned round file was not edited.

## Controlled regression and limits

These are deterministic unit/integration checks of metadata comparison and
rollback plumbing under controlled verified metadata. They do not reproduce a
real external format change, an outside writer, filesystem timing or SQL
conversion. Actual SQL verification runs before the fixture supplies an otherwise
identical metadata result with the alternate supported format at one selected
boundary. Module/namespace renaming and import/control plumbing isolate the
verification variant. The generator changes no comparison, SQL verification,
transaction or commit code. All seven implementation substitutions and both
interface substitutions are unique; three implementation substitutions supply
metadata at the exact boundaries. Generation manifests record each substitution
and count and prove exact forward/reverse byte reproduction. Frozen red variants
are audited separately against their frozen source; green storage/export variants
are audited against current source.

Each boundary test exercises starting format1 and format2 and unchanged controls.
The locked-entry case refuses with its existing `unsafe_path` message. Writer
precommit refuses after tentative interning and invalidates subsequent commit and
intern calls. Acceptance precommit refuses after tentative history writes, retains
`not_completed` and an empty receipt, and permits same-key fresh retry and exact
saved replay. Refusals compare all seven table counts, original database bytes,
all metadata, initial head and request absence. Control operations really commit.

Before the fix, the frozen tests failed in all three cases, with six precise
missing-refusal assertions (one per boundary/format); controls succeeded. After
the fix the same test, fixture and generator bytes passed. No failed build or
verification attempt is omitted. The expected red run is the only nonzero exit.

## Verification

Exact argv, cwd, start/end times, exit codes and raw output are recorded by
`run.py` in each named `.json`/`.log` pair. CTest JUnit and copied LastTest files
were preserved before inventory. Debug build/test calls were serialized.
Final product/support/status source was frozen before final verification; no
source edits occurred during or after those commands.

| Check | Result |
| --- | --- |
| Pre-fix boundary CTest | 3 cases fail, six missing-refusal assertions, zero errors/skips |
| Debug `all planar_tests` | Pass |
| Corrected boundary CTest | 3/3 pass, zero errors/skips |
| Focused `ledger\|history\|export\|record\|request\|prepared\|acceptance\|publication\|atomic\|metadata continuity` | 209/209 pass, zero errors/skips, 75.10 s |
| Record-v1 / request-v2 independent vectors | 10 + 2 verified |
| Pinned active source formatting / Doxygen | Pass, 98 sources |
| Pinned support C++ formatting | Pass, five files including both new C++ files |
| Exact storage/export variant audits | Five families, ten files, forward/reverse exact |
| CTest inventory | 430 names; every original 427 retained; three added |
| Final debug `all planar_tests` | No work after normal Ninja glob recheck |
| Full source preservation audit | All 15,288 final files and two symlinks match freeze |

Raw focused LastTest establishes, separately for formats1/2, journal SQLITE_FULL
ordinals 0–37 (success 38), database ordinals 0–14 (success 15), all failures with
positive hit counts, and unique process boundaries 0–4. `results.json` retains the
parsed evidence, while the unchanged assertions retain D8 rollback/retry/replay
and conservative non-hot versus explicit hot-journal recovery behavior.

This is macOS ARM64 debug focused verification. No full release/integrated gate,
Linux or hardware power-loss claim is made. The original B2-B candidate and this
correction require fresh independent review before subsequent integrated
verification. X03-B, X06 and the mandatory external-profile gate remain open;
public init still creates format1 and profile acceptance remains unsupported.

## Evidence identities and reconstruction

`baseline.json` inventories 15,285 files and two separately recorded symlinks
before any scoped edit. Exclusions are only root `.git/`, root `build/`, and this
new evidence directory; nested historical build files and all historical evidence
are included. Exact pre-edit copies exist for all changed existing files.
`available-baseline.diff` reconstructs all 12 changed/new source/status members.
`final-source-manifest.json` inventories the entire final non-excluded tree;
`candidate-manifest.json` identifies the 12 scoped members and their exact copies
under `candidate-source/`. `red-source-manifest.json` identifies frozen regression
source, generator, production source and generated variant before red execution.
`final-generated/` archives the final generated variant and its exact report.
All manifest map paths resolve relative to the repository root except the final
artifact manifest, whose members resolve relative to this evidence directory.

| File | SHA256 |
| --- | --- |
| candidate-manifest.json | `6497c9c720369653b60f38b180dae32465ab7468ccdab3685ac10827522490a6` |
| final-source-manifest.json | `717ddb977f5c07563c674d6397d6023bf66b7a65caf6c4be0fe0b80866b2747f` |
| available-baseline.diff | `0277ce09ca89d41206998615e752d13950c070b64034e510417c65f6580b7f04` |
| red-source-manifest.json | `96c31d9d74e5b43191999465c65ce040304e3aa8a44bcd64fb72bb4cae5d2b8e` |

`artifact-manifest.json` freezes every other evidence file and excludes itself.
Reports/manifests were finalized before ownership release. No further writes are
authorized to this coder's completed ownership after handoff.
