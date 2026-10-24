# V5-02-B1 coder candidate

This is implementation and verification evidence, not independent approval.
Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Existing rewrite changes were
preserved. The coordinator owns the round record. No Planar tooling, installed
binary, operator data, installation, commit, release build, publication or merge
was used.

The candidate contains the native retained invocation-local command VM, exact
catalog validation and same-bundle callable references, shipped four Lua handlers,
and finite typed native service interface. Test providers supply capture/storage
facts; real CLI integration remains B2. Opaque host slots, ordered services,
retained native errors/facts, frozen globals/libraries, cumulative native transfer
charges, terminal caught quotas and postpublication export state are enforced.
Terminal invocation closes Lua and releases references/handles/private intent.
The fresh fixed-export API remains available and has no registration capability.

`candidate-files.json` hashes all 11 owned product/test/support/reference files;
`candidate/` captures them. `baseline/` contains seven pre-existing owned files
before the first edit; the other four were new. `owned.diff` is reconstructed
against those baseline bytes, including additions. Its SHA256 is
`295a89f4ff4c4952a2875634fc91e8710fc0780a3768be6b5a278cb7c4078f7a`.
`baseline-manifest.json` captured 5,650 existing files under source, tests,
vendor, fixtures, scripts, CMake and docs before edits, excluding this new evidence
and coordinator-owned current round. The audit checks 5,643 unchanged protected
files after excluding the seven owned existing files; historical evidence,
fixtures, v5 and existing goldens remain byte-identical. Root instruction files
were outside that manifest; the AGENTS symlink remains intact and no root file
was edited by this coder.

## Verification

Direct commands were serialized within `build/rewrite-debug`; the final build
completed before the final CTest run. Final JUnit and LastTest were archived before
inventory commands. No product inputs changed afterward.

- `cmake --preset debug`, followed by incremental
  `cmake --build build/rewrite-debug --target all planar_tests`.
  Final implementation/interface/bundle builds are in `build-combined.log`;
  final added regression compilation is `build-combined-test.log`.
- `ctest --test-dir build/rewrite-debug -R
  'replacement.(command|runtime|loader|provenance|interpretation|acceptance prepared|cli$|init_cli$|history_cli$|export_cli$)'
  --output-on-failure --output-junit <absolute evidence>/verified.xml`:
  83 passed, zero failures/skips, including all 17 new cases and all four actual
  CLI suites. `verified.log`, `verified.xml`, `verified-LastTest.log` are the final
  raw records. This is focused verification, not a claim that all 354 tests ran.
- The independent cross-phase conversion-budget regression passed separately:
  `combined.log`, `combined.xml`, `combined-LastTest.log`. It discovers a successful
  execution boundary with ordinary registration, then repeats identical transfers
  with larger valid catalog help. Registration alone succeeds; execution with
  ordinary registration succeeds; combined registration/execution fails
  `native_bytes`. Resetting the native counters would make both executions start
  equally and fail the regression's required refusal.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  pinned formatting and Doxygen passed over 87 active sources. See
  `lint-verified.log`. The documented Doxygen signal-only retry succeeded on its
  second attempt; no warning was suppressed.
- `python3 tests/vectors/generate.py --check`: 12 hand byte vectors, 20 upstream
  hashes, 41 canonical/domain vectors passed (`vectors.log`).
- The final no-change `cmake --build build/rewrite-debug --target all planar_tests`
  ran zero steps (`no-work.log`).
- `python3 tests/native/v5-02-b1-evidence/audit.py`: passed (`audit.json`). It
  reverses all 18 exact runtime/interface variants, verifies their exact embedded
  assembly/original bytes and logical command-module owner, checks the original
  runtime test variant, candidate/protection hashes and unchanged historical input
  coverage. All 337 old CTest names remain; 17 new names bring registration to 354.

Intermediate build, smoke and test failures remain preserved. Initial smoke caught
counting reserved C stack capacity as live slots. The retained bound now checks
actual live top and Lua frame slots, and native multi-result string/UTF-8 helpers
are bounded before stack growth. The real render test caught redundant tagging
of an already typed sorted list; the shipped handler now uses unchanged normalized
routing. Initial lint found missing declaration documentation; final lint passed.

The interpretation assertion independently reconstructs authored original files,
ordered assembly and canonical manifest, computes the complete provenance tree
digest, and changes only expected `stdlib_digest` in the historical decoded fixture.
All other expected values/canonical bytes remain exact; old canonical files still
serve historical decoding tests. The actual CLI render goldens were not modified.

## Limits and handoff

This macOS Apple Silicon debug result does not establish Linux, release, CLI
retained-VM integration, workspace naming, load, real storage adapters or external
profiles. B2 must compose the provider into the actual family handlers and retain
native publication/stdout uncertainty policy. Native write-lock release is a
provider contract here; no real storage transaction runs in these tests.

Debug build ownership is released after this evidence is frozen and reported.
Independent review must check the exact candidate before B2. No self-approval is
claimed. `artifact-manifest.json` hashes all evidence artifacts except itself.
