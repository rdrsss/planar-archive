# N03-B integration candidate evidence

Candidate awaits independent review. Shared checkout `dev/planar-v2-vernacular`,
starting HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`; all pre-existing
rewrite changes preserved. No Planar tools, installed/legacy executable,
database, delegation, commit, installation or publication. Builds serialized.

## Implementation and library ownership

The approved `planar.engine.json_codec` is unchanged. Trusted runtime links it
and exposes only `engine.json_decode`/`engine.json_encode` to the trusted state.
Declaration states retain their constructor-only environment. Lua `json_parse`
is a type/diagnostic adapter: logical file, `$`, one-based library byte offset,
`syntax` to `schema`, `input_bytes` to the existing `json_bytes` name. Generic
schema and observation policy remain Lua. Decode/encode failures return ordinary
Lua diagnostic tables, independent of opaque native hash-error tokens.

Codec/conversion work and all C++ RAII temporaries finish before allocating Lua
results or raising Lua errors. State owns the pending structural result while
protected pushes can longjmp. Caught native diagnostic construction failures
leave the catch frame before `luaL_error`; no exception destructor is skipped.
Repeated decode validation and encode conversion retain cumulative native
conversion accounting. Native codec allocations remain separate from the Lua
allocator and this accounting: writer scratch at most **96 MiB + 1 KiB**, plus
bounded parsed/converted structures and caller-owned input. This is not total RSS
or a 16 MiB working-set claim. No allocation-failure injection/RSS measurement.

The old native `src/engine/json/` module/tests/target are deleted. CLI success,
usage, native/schema errors and checked stdout-write errors construct ordinary
values and use Glaze encoding. No handwritten output literal/fallback remains;
if allocation/encoding cannot produce a diagnostic, status 4 returns without
fabricated JSON or recursive serialization attempts. `json.lua` no longer has a
JSON scanner; `presentation.lua` calls the same native encoder for descriptors
and checkpoint accounting. Its byte-to-hex helper is solely visible prose
control escaping, followed by dynamic fence selection. UTF-8 validation and
sorting helpers are generic policy, not JSON grammar/escaping. The static empty
observations JSON document is input parsed by Glaze. No codec/trusted-render
filesystem access was added. Binary canonical_v1 and hash modules are unchanged.

## Recorded failures and corrections

`tests-before-expectations.log` preserves the first full 117-test run: 114 passed,
3 failed. Two were intentional Glaze output changes: current outer wire goldens
and the old checkpoint assumption that every newline costs six encoded bytes.
The third exposed a product adapter bug: non-string `json_parse` input returned
`internal`. The thin Lua adapter now retains the existing `schema` refusal.
The new checkpoint regression measures actual Glaze output at 8192/8193 bytes,
including 2048 newline characters, without changing the 2048-byte field policy.

`gate-debug-initial.log` preserves a subsequent harness failure while all 118
native cases passed. The new CLI test incorrectly expected a JSON offset for
invalid raw UTF-8. `cli-error-probe.log` confirms capture rejects it before JSON;
the test now checks that earlier boundary. Syntax/duplicate/numeric refusals
through the JSON bridge still require logical file, `$`, and positive offset.
`diff-check.log` records an extra CMake EOF blank line; it was removed without
changing target semantics, then both directories reconfigured and returned no
work on a second build (`final-*-reconfigure.log`, `final-*-no-change.log`).

## Executed verification

- `cmake --preset debug`, then direct `cmake --build build/rewrite-debug --target
  all planar_tests`: passes; development build transcripts retained.
- `build/rewrite-debug/src/engine/planar_runtime_tests '[codec]'`: **3 cases /
  56 assertions pass**, including the library-computed checkpoint case.
- `make test-all PRESET=debug` and then `make test-all PRESET=release`: **119/119
  registered tests each, zero skips**. Each has 118 native cases / **48,229
  assertions**, plus one acceptance case with **158 actual CLI invocations**,
  60 host-path checks, six exact native goldens, independent Python JSON/hash
  oracles, fresh processes, closed stdout and checked I/O failures. Formatting
  and Doxygen pass over 25 active C++ sources; no-change builds do no work.
  Raw logs: `gate-debug.log`, `gate-release.log`, `ctest-debug-detail.log`,
  `ctest-release-detail.log`; counts are in `test-counts.json`.
- Both built `planar_json_codec_tests '[codec]'`: unchanged **8 cases / 115
  assertions pass** (`codec-debug.log`, `codec-release.log`).
- `python3 tests/acceptance/render_evaluation.py /absolute/built/planar NEW_DIR`
  for debug and release: each **19 actual CLI invocations, six exact goldens,
  11 identity comparisons, independent hashes, zero skips**, inputs/HOME stable.
  Raw outputs and commands are in `evaluation-debug/` and `evaluation-release/`;
  outer logs are `evaluation-debug.log` and `evaluation-release.log`.
- `git diff --exit-code HEAD -- vendor/glaze`: passes (`vendor-exact.log`).
  `approved-codec-check.json` verifies all three codec sources, dependency pin
  and notices against the N03-A-C01 approved manifest; no codec edits.
- `preserved-inputs.json` verifies all 20 original declaration/registry/source/
  observation input modes and hashes against frozen R10 evidence. The original
  R03 authored expectations/symbolic data were not edited.
- `historical-evidence-integrity.json` verifies frozen R10 (166 entries), N03-A
  (20), N03-A-C01 (37) and corrective review (19) evidence manifests; all match.
  `historical-evidence-sha256.txt` inventories all 824 retained prior evidence
  files. `source-audit.json` records deleted paths, removed custom-code markers
  and exact codec/binary-identity source hashes.

## Exact output changes

`native-before/` preserves every previous current native golden and its README.
`golden-identity-diff.json` records exact old/new hashes and byte counts for all
six envelopes. All parsed envelopes are **identical**, including W, unit outputs,
worker role outputs, observations, Markdown and render identity. Wire bytes
shrink because Glaze uses short escapes. The fixture descriptors need no C0
escape change. General descriptors containing controls use Glaze spelling and
can legitimately change Markdown/render identity; identity domains are unchanged.
`evaluation-historical-identity-diff.json` compares all 19 debug and 19 release
current packets with frozen R10: parsed packets and identities are identical,
with 11 mutation comparisons each. Frozen R10 bytes remain untouched.

## Freeze and limitations

`candidate-files.txt`, `candidate-sha256.txt`, `candidate/` and
`deleted-paths.txt` identify the 25 changed retained files and three deletions.
`evidence-sha256.txt` freezes the evidence separately, excluding itself.
Coordinator-owned N03/evaluation-status records are outside this candidate.
No approval is claimed. Verification is macOS Apple Silicon debug/release;
Linux, sanitizers, native allocator fault injection, RSS and concurrency were not
executed. No additional broad test run is needed absent new changes/findings.
