# N03-B independent integration review

Disposition: **approve**. The integrated candidate completes the operator's
Glaze restoration request: all active product JSON grammar, unescaping and
serialization are delegated to the pinned library. No remaining handwritten
product JSON parser, string serializer or emergency JSON fallback was found.

Reviewed shared `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product/candidate and prior evidence
were read-only; reviewer ownership was this report and
`tests/native/n03b-review-evidence/`. No unrelated changes were reverted. No
Planar tools, installed/legacy binary, database, commit, installation,
publication, delegation or concurrent builder was used.

## Assessment

- The approved codec remains unchanged. The adapter invokes Glaze structural
  parsing, complete strict validation and writing; native checks enforce value,
  duplicate, UTF-8/NUL and resource policy. The old three-file native JSON module
  and its target/tests are removed. Both Lua handwritten JSON implementations
  are gone: `json.lua:54` adapts type/diagnostic policy around
  `engine.json_decode`; `presentation.lua:6` delegates descriptors and checkpoint
  sizing to `engine.json_encode`. Generic merge sorting, UTF-8 validation,
  prose-only control display and dynamic Markdown fences are legitimate Lua
  policy. The static empty-observations JSON document is parsed input.
- `runtime.cpp:388` completes conversion/codec work before Lua return allocation;
  pending native values belong to state. At `:417`, all C++ RAII locals and
  caught exceptions have unwound before `push` or `luaL_error` can longjmp.
  Existing conversion anchoring stays protected. Diagnostics at `:372` are
  ordinary records, independent of opaque hash-error tokens. Syntax maps to
  `schema`, raw-byte limits to `json_bytes`, and offsets to one-based bytes;
  Lua adds the logical file and `$`. Declaration environments have no binding.
- Successful decode validation and encode conversion retain cumulative native
  accounting. Independent repeated-call probes reach `native_bytes` on both
  paths. At that cumulative boundary final result conversion can itself refuse
  with a top-level typed resource diagnostic; it does not reset accounting.
  Per-call codec storage remains outside Lua allocator/accounting. Documentation
  accurately limits writer scratch to 96 MiB + 1 KiB plus bounded structural
  copies and caller-owned input; this is no total RSS guarantee.
- CLI success, usage/schema/native failures and checked stdout failure all use
  structured values through `codec_encode` (`bootstrap.cpp:54`, `:196`, `:221`).
  Serialization failure and outer allocation failure return status 4 without
  fabricated JSON (`:57`, `:278`, `:284`). The CLI reference explicitly documents
  this exception. There is no manual control-spelling normalization in product
  output. No filesystem capability was added to the codec/trusted renderer.
- Glaze short escapes and uppercase control hexadecimal are explicit current
  transport/descriptor contracts. The revised checkpoint test constructs actual
  8192/8193-byte library output while retaining 2048-byte field limits
  (`schema.t.cpp:2076`). It replaces the obsolete six-bytes-per-newline assumption
  without relaxing the 8192-byte object bound. Binary canonical encoding and
  hash domains are unchanged. General descriptors containing controls can change
  Markdown/render bytes under the documented spelling change; no claim of
  universal render-identity preservation is made.

No blocking finding remains. Coordinator-owned closing records can point to the
current N03-B evaluation while preserving the original R10 guide/evidence as the
historical evaluation package.

## Exact identity and preservation

`identity-before.json` and `identity-after.json` match. They independently verify
all **25 candidate files**, matching snapshots, all **236 frozen N03-B evidence
files**, and the **824-file historical evidence inventory**. The three deleted
old JSON paths remain absent. Candidate manifest SHA256:
`28efeb946c429af57490264cc1f95f4ebaef08a56b538c96e89d65db2f7579db`.
N03-B evidence manifest SHA256:
`274ad6bb470c0897eaea7de0d0b2db7ff0410ef5adb017c2ceae901006160022`.

All three approved codec sources, dependency pin and notices match the approved
N03-A-C01 values. All **663 Glaze files** remain HEAD-exact. Frozen R10 (166),
N03-A (20), N03-A-C01 (37) and corrective-review (19) manifest entries still
match. The N03-A-C01 engine CMake file is intentionally superseded by the current
integration candidate, not treated as an unchanged codec source.

## Independent verification

Reviewer-owned sources, exact command arrays, logs and audits are retained under
`tests/native/n03b-review-evidence/`:

- `run.py` compiled and ran a public API probe against the current production
  runtime plus fixed test runtime: **262 checks, zero failures**. It exercises
  malformed separators and trailing input; numeric/null/duplicate/NUL/Unicode
  refusals; signed integer bounds, empty map/array and surrogate-pair acceptance;
  ordinary diagnostic structure, logical files and positive offsets; raw-byte
  and depth ceilings; hash-error recovery; library spelling and declaration
  codec denial.
- `custom-run.py` compiled an exact copy of current bridge implementation, with
  only its embedded-source import redirected to reviewer-owned fixed exports.
  **Four scenarios pass**: 1000 iterations mixing caught opaque hash errors,
  malformed JSON, valid decode/encode and cyclic encode refusals; wrong-type/
  extra-argument/function refusals followed by valid decode; repeated successful
  50,000-value decode; and repeated encode of that decoded value. Both repeated
  paths reach cumulative `resource_limit/native_bytes`. This exercises the exact
  native implementation without editing the candidate or introducing runtime
  source injection into the product. `artifact-cleanup.json` verifies the copy
  and records executable/object/PCM hashes before owned binary artifacts were
  removed.
- `cli-probe.py`: **180 checks / 23 CLI processes, zero failures**. Malformed
  registry inputs retain logical file, `$`, schema code and positive offset.
  An unknown field containing newline/tab/CR/backspace/form-feed/U+001F proves
  Glaze short escapes and uppercase `\u001F` on the actual error-output path.
  Closed stdout yields status 4 and an encoded I/O diagnostic. Scratch HOME and
  database path are isolated and no database appears.
- Focused current runtime codec tests: **3 cases / 56 assertions pass**, zero
  skips. Current codec tests: **8 cases / 115 assertions pass**, zero skips.
  `cmake --build build/rewrite-debug --target all planar_tests`: **zero steps**,
  `ninja: no work to do.` No full gate was needlessly repeated.
- `evidence-audit.py` independently parses the raw debug/release detail logs:
  **118 native cases / 48,229 assertions** each, plus acceptance; both full gate
  logs report **119/119, zero skips**, 25-file formatting/Doxygen and no-work
  build. It independently compares all **19 current parsed packets per build**
  to frozen R10: all equal, including identities/Markdown. All **20 original
  fixture inputs** retain exact modes and hashes. All six current parsed native
  goldens equal their preserved previous envelopes. All **21 local Markdown
  links** in candidate documents resolve.

The first custom helper compile used an incorrect reviewer-written Lua include
path and failed before executing; its command/log are preserved as
`initial-custom-bridge-compile*`. Correcting that harness path to the committed
`vendor/lua/0b10/src` compiled successfully without candidate changes. Existing
coder red evidence for the output/checkpoint spelling changes and actual
non-string parser regression was inspected and remains preserved.

## Limits and approval boundary

Independent executable probes ran on macOS Apple Silicon debug; release full
execution and evaluations were inspected from frozen raw evidence. No Linux,
sanitizer, native allocation-failure injection, RSS measurement or concurrency
claim is added. Source analysis establishes the Lua longjmp ownership boundary;
normal/refusal/resource probes support it but do not replace allocator fault
injection. No end-to-end 16 MiB CLI envelope-boundary success is claimed.

This approves the exact integrated JSON replacement, not persistence or an
operator evaluation outcome. R10 operator packet assessment remains pending.
