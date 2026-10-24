# R13-C1 implementation evidence

Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Coordinator owns round records.
No Planar tools, installed binary, operator database, commits, installation,
publication or merge. All storage fixtures use canonical scratch roots.

## Candidate and verification

Candidate adds shared pure tree construction/digest and bounded owned registry/
stdlib preparation, generated ordered original views and historical manifest
verification. Product runtime `source()` retains exactly the pre-change assembly.
Existing Lua/runtime/CLI/codec/storage/vendor and prior evidence are protected.
Prepared registry contains exact `input.json` bytes/mode. Stdlib includes exact
originals and two explicit synthetic files; complete bounds include both. Native
manifest decoding executes no Lua. Metadata count/path/separator bounds precede
bounded real canonical encoding; total encoded size precedes payload copying.
No handwritten JSON/canonical serializer or additional path grammar is used.

Commands ran directly on macOS Apple Silicon debug:

| Command | Raw evidence | Result |
| --- | --- | --- |
| `cmake --preset debug` | configure.log | Exit 0. |
| `cmake --build --preset debug --target all planar_tests` | build.log, build-2.log, build-final.log, build-roundtrip.log | Exit 0; incremental refinements preserved. |
| `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger\|canonical\|hash\|record\|tree\|provenance)' --output-on-failure --output-junit tests/native/r13c1-evidence/focused-final.xml` | focused.log/.xml, focused-final.log/.xml | Initial and final runs 99/99 passed, zero failures/skips; final includes scratch-ledger provenance roundtrip. |
| `python3 tests/vectors/generate.py --check` | vectors.log | Exit 0; 12 hand-byte, 20 upstream-hash, 41 canonical/domain vectors. |
| `python3 tests/vectors/records/generate.py --check` | record-vectors.log | Exit 0; ten independent record vectors. |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | lint.log, lint-final.log | Exit 0; pinned formatter and Doxygen, 47 active files, zero warnings. |
| Supplementary pinned formatter and Doxygen | helper-lint.log | Exit 0; two verification-only helper files, zero warnings. |
| `ctest --test-dir build/rewrite-debug -R '^replacement.cli$' --output-on-failure --output-junit tests/native/r13c1-evidence/cli.xml` | cli.log/.xml | Exit 0; one acceptance case, 247 fresh-source-binary invocations. |
| `cmake --build --preset debug --target all planar_tests` after completed tests | no-work.log | Exit 0; `ninja: no work to do.` |
| `python3 tests/native/r13c1-evidence/audit.py` | audit.log | Exit 0; unchanged protected files, exact fresh reversible C2/B1 variants, frozen candidate and report counts. |

Six new cases cover exact binary registry bytes/modes and its byte boundary;
all eight authored/generated stdlib source comparisons plus legacy concatenation;
scratch-ledger intern/commit/reconstruct/verify of exact files; source/order/
separator/boundary identities; unsafe/reserved/duplicate/aliased paths; wrong
modes, missing/extra/duplicate files, malformed/unsupported manifests; 4094 originals
plus two synthetic files; exactly 16 MiB combined payload success and next-original-byte
refusal. Existing B2 success helpers additionally compare pure/interned digests
across profiles and displaced local IDs. Existing rollback, collisions and all
B2 limits remain selected and unchanged in meaning.

## Harness corrections and limits

Initial `audit-initial.py`/`audit-initial.log` correctly checked protection and
variants, then failed because it looked for JUnit reports relative to repository
root. CTest resolves relative `--output-junit` under its build directory.
Corrected `audit.py` copies those exact report bytes into this evidence folder
before validating. This was evidence plumbing, not a product test failure.

An early no-change build/audit invocation overlapped the final tree test: the
build did zero work. Its log and audit are preserved as `no-work-overlap.log`
and `audit-before-final-serialization.log`. The authoritative `no-work.log` and
`audit.log` were rerun sequentially after the CTest session returned exit 0.

No product test or build failed. Full integrated R13 debug/release and Linux
execution remain outside this focused slice. The current registry has 216 cases;
only the 99 focused cases and one acceptance case were run here. This evidence
makes no self-approval claim. Debug is released on delivery.

## Path bases and immutable identities

`protected.json` maps repository-relative pre-round input paths to SHA256,
excluding owned files, build/Git internals, coordinator round records and this
new evidence folder. It includes all prior evidence and protected product files.
`baseline/` preserves the seven existing owned files before this round.

`candidate.json` maps ten repository-relative files to SHA256. Exact snapshots
use those paths below `candidate/`. Candidate identity hashes the compact
sorted-key UTF-8 `files` map with comma/colon separators and no LF.
`candidate.diff` compares exact baselines (empty for new files) using repository-
relative diff labels. `registry.json` is fresh CTest inventory.

`r12c2-generation.json` and `r13b1-generation.json` are exact configured manifests;
fresh temporary regeneration matches source/generated bytes and both digest
substitutions without equality mutation. `evidence.json` maps paths relative to
this evidence folder to SHA256 and excludes itself only. It includes scripts,
logs, snapshots and manifests. This README belongs to that manifest.
