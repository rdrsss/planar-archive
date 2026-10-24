# R13-B1 coder evidence

Candidate for independent review; this is not an approval. Shared checkout
`dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The pre-existing breaking rewrite is
preserved. No Planar tools, installed binary/operator database, commit, installation,
publication, tree builder, CLI or acceptance implementation was used.

The product API adds bounded typed snapshot reads and independently owned
`BEGIN IMMEDIATE` writers. All four kinds intern in one explicit transaction;
exact encoded equality includes local directory child IDs. Every failed operation,
failed commit and abandonment invalidates/rolls back the complete writer. Full
streaming validation checks stored metadata/bytes/hash, ordered edges, older
references and exact digest buckets, while keeping revisions/requests empty and
initial event/current state exact. Existing format, publication, recovery, schema
and primitive protection remain in place.

Final commands and results:

- `cmake --preset debug`: configured successfully; see `configure-final.log`.
- `cmake --build --preset debug --target all planar_tests`: succeeded; final
  restored protected variant build is `protected-build.log`.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record)' --output-on-failure --output-junit <repo>/tests/native/r13b1-evidence/focused.xml`:
  **76 matched, 76 passed, zero skips** in `focused-final.log` and `focused.xml`.
- `python3 tests/vectors/generate.py --check`: 12 hand byte vectors, 20 upstream
  hashes, 41 canonical/domain vectors; `vectors.log`.
- `python3 tests/vectors/records/generate.py --check`: 10 independent vectors;
  `record-vectors.log`.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  pinned formatter and Doxygen passed across 40 active source files;
  `lint-final.log`. Supplementary identical formatter/Doxygen configuration
  over `digest.cppm` and `collision.t.cpp` passed; `helper-lint.log`.
- `python3 tests/native/r13b1-support/audit.py`: 1,834 protected source/vendor/
  vector/prior-evidence files unchanged; fresh temporary regeneration of both
  R12-C2 and R13-B1 equals configured bytes/manifests, both digest call sites
  are redirected, focused count/zero-skips and restored mutation state checked;
  `audits.log`.
- Repeating `cmake --build --preset debug --target all planar_tests` performed
  zero steps (`ninja: no work to do.`); `no-work.log`.

Forced collisions use only verification modules. `generate.py` renames the format
module/namespace and redirects its two digest calls to a helper that imports the
original record type/encoder and forces a distinct constant for each kind. Its
manifest proves exact reversible source transformation. The normal generated
variant exercises all four kinds, partial-prefix rollback, local-ID-only directory
differences and ambiguous exact-plus-unequal buckets in both row orders.

Mutation RED was produced by:

1. `python3 tests/native/r13b1-support/generate.py src/engine/ledger build/rewrite-debug/src/engine/r13b1-generated --mutate-equality`
2. `cmake --build --preset debug --target planar_record_format_test_tests`
3. `build/rewrite-debug/src/engine/planar_record_format_test_tests '[collision]'`

This verification-only mutation removes the interning exact-byte check, preserving
the validator and every other product byte. `mutation-generation.json` records
that additional reversible substitution. `mutation-red.log` has exit 42: 3 cases,
1 passed, 2 failed, 17 failed assertions; all four kind iterations and the separate
local-child-ID case expose the missing check. Some Catch2 diagnostics contain raw
synthetic digest bytes, so the log must be read as bytes or with replacement
UTF-8 decoding. Product source was never deliberately broken. Regenerating without
`--mutate-equality`, rebuilding, and running the final focused selection produced
GREEN. `generation.json` and `r13b1-generation.json` are the restored variant;
`r12c2-generation.json` also audits the existing publication-fault variant.

Intermediate evidence is retained accurately: `build-tests.log` failed because a
new test compared `expected<string, record_error>` objects whose error lacks
operator==; the test now compares successful byte values. `focused-first.log`
passed the product cases but failed the three collision cases because their new
scratch helper used macOS's symlinked `/tmp` without canonicalizing it. The helper
now canonicalizes its root, matching existing foundation tests. The new standalone
metadata case passed in `metadata.log` before inclusion in the final 76-case run.
`audits-initial.log` records an audit-script UTF-8 decoding error on the synthetic
mutation log after the protected-source/generation/JUnit checks passed; the final
audit reads that log as bytes. These are harness/evidence issues, not product
regressions or a deliberately broken product baseline.

`before/` and `status-before.txt` freeze the initial owned files/pre-existing state.
`candidate/`, `candidate.json` and `candidate.diff` freeze the exact reviewable
source and baseline-relative changes. `evidence.json` hashes evidence artifacts
(excluding itself). The coordinator owns round records and review disposition.
This is focused macOS debug evidence; complete R13 integrated debug/release gates,
complete tree reconstruction, acceptance and Linux runtime verification remain
outside B1.
