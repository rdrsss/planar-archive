# R13-C2 interpretation and integrated R13 evidence

This is coder evidence, pending independent review. Work used the shared
`dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No Planar tooling, installed binary,
operator database, commit, installation, publication or merge was used.
Coordinator round records are outside coder ownership.

## Candidate and contracts

The new native interpretation module composes internally prepared embedded
provenance with the fixed trusted `interpret_bundle` export. The export composes
existing validation/normalization under `not_captured`, preserving complete
outputs and sorting both job lists. Native code owns the exact envelope/versions
map, canonical byte ceilings and historical fixed-format verification. Historical
decoding never invokes Lua or compares historical code identity to current code.
Registry provenance remains C1 preparation from the same captured bytes/mode.

`candidate.json` pins thirteen repository-relative product/reference/fixture paths;
exact snapshots use those paths beneath `candidate/`. Its identity hashes compact
sorted-key UTF-8 JSON of the `files` map, with comma/colon separators and no LF:
`37ad152cae2e7601a85761a6f23f9f8cc6213beafa9451ade67af05dbbba25e8`.
`baseline/` stores the five existing owned files. C1 candidate copies provided
four exact baselines; bootstrap was reconstructed by reversing the single added
export line and verified against the pre-edit SHA256. `candidate.diff` includes
textual changes; the manifest pins binary canonical fixtures too.

Full captured expected structures were reviewed in `golden-review.txt` and
`golden-review.md` before fixture freeze. JSON fixtures came through Glaze,
canonical fixtures through the existing canonical codec. The temporary capture
branch was removed from final tests. Exact whole-value comparisons against
existing normalize/validate exports independently check field completeness.
The ninth embedded Lua file deliberately changes stdlib assembly/provenance;
existing render/work/CLI expectations remain checked unchanged.

## Focused verification and honest intermediate failures

Executed direct serialized builds and tests, with all stdout/stderr retained:

- `cmake --preset debug`, then `cmake --build build/rewrite-debug --target all planar_tests`.
- Final `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)' --output-on-failure --output-junit r13c2-focused-final.xml`: 109 cases passed, zero skips.
- `ctest --test-dir build/rewrite-debug -R 'replacement.cli' --output-on-failure --output-junit r13c2-cli.xml`: one acceptance case passed, zero skips (247 source-built invocations).
- `python3 tests/vectors/generate.py --check`: 12 hand byte vectors, 20 upstream hashes, 41 canonical/domain vectors.
- `python3 tests/vectors/records/generate.py --check`: 10 independent record vectors.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: formatter/Doxygen passed 50 active sources. Doxygen's documented diagnostic-free SIGBUS occurred on the first attempt and its existing helper succeeded on attempt two; raw log retained.
- Python syntax compilation without cache writes and `bash -n scripts/cpp-lint-doxygen.sh` passed.

`build-initial.log` retains a compile failure from an unused copied test helper;
removing that helper corrected it. `interpretation-initial.log`/`initial.xml`
retain two test-authoring failures: golden files had not yet been captured, and
raw registry check-input order was compared directly against normalized set
order. The complete normalized registry comparison now sorts only that input
set before equality, while the original exact byte provenance stays separately
checked. No product behavior was changed to accommodate either test mistake.
The 9-case `interpretation.log`, first 108-case `focused.log` and final 109-case
`focused-final.log` are preserved with their exact JUnit reports. The final
additional case verifies absent optional notes/rationale without manufacturing
fields. Final tests also separate native runtime/resource failures from trusted
schema/ownership/unsupported diagnostics.

The saved-fixture record roundtrip initializes a scratch ledger, interns both
canonical values, commits/reopens and decodes complete bytes without any Lua or
capture call. Historical extension/different-digest checks do not rerun modern
schema policy. Inclusive/excessive 16 MiB, map/list and signed-32 boundaries are
covered; no successful empty selection or skipped test is counted as evidence.

## Protection and path bases

`baseline.json` is the pre-edit Git-visible file hash map; `starting-status.txt`
preserves the existing large rewrite. `audit.py` supplements that baseline with
C1's inherited full protected map, so ignored prior files are covered. The initial
`audit-freeze.log` checked only Git-visible inputs; `audit-expanded.log` added the
inherited map. Final audit additionally protects previous round records, excluding
only the coordinator's current R13/C/C2 records. Every `.DS_Store` path is
explicitly excluded as OS metadata. Exact C1 coder/reviewer evidence manifests
(47/34 entries) are also verified, and no prior evidence was modified.

All protection/candidate paths are repository-relative. Generation manifests
are exact configured output; fresh temporary generation is compared byte for
byte with each configured variant. R13-B1 requires both digest substitutions,
reversibility and no equality mutation. R12-C2 requires complete reversibility.
`evidence.json` maps paths relative to this directory, excluding itself and OS
`.DS_Store` metadata only; it includes raw logs, snapshots, scripts and manifests.

## Frozen integrated milestone gates

After focused checks, final sources and all thirteen candidate files stayed
unchanged through these sequential complete gates:

- `make test-all`: 226 registered cases passed, zero skips; formatting/Doxygen
  passed 50 active sources; final no-change build ran zero steps.
- `make test-all PRESET=release`: 226 registered cases passed, zero skips;
  formatting/Doxygen passed 50 active sources; final no-change build ran zero steps.

Both raw full logs retain every test status and the checker's nonzero-count/
zero-skip confirmation. `audit-final.log` verifies 11,863 protected inputs,
including prior round records, all exact prior C1 coder/reviewer evidence and
both debug/release exact reversible generated variants. Candidate identity is
unchanged. Separate final serialized `cmake --build build/rewrite-{debug,release}
--target all planar_tests` invocations each print only `ninja: no work to do.`

These gates ran on this macOS arm64 host with the pinned toolchain. Linux,
future R14 acceptance/history activation and execution of historical Lua policy
are not claimed. Both build directories are released for independent review.
No coder self-approval is implied.
