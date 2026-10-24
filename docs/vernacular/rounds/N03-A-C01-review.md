# N03-A-C01 independent review

Disposition: **approve** N03-A with the N03-A-C01 correction.

Reviewed the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Candidate and previous evidence were
read-only. Reviewer writes are this report and
`tests/native/n03ac01-review-evidence/`; unrelated changes were preserved.
No Planar tool, installed/legacy executable, database, commit, installation,
publication, delegation, or concurrent builder was used.

## Finding closure and implementation assessment

**N03-A-01 is resolved.** `codec_decode` now calls `glz::validate_json` on the
same complete input bytes after the bounded structural read succeeds and before
conversion (`src/engine/json_codec/codec.cpp:260`, `:269`, `:274`). This rejects
the original trailing/missing object commas and nested variants. The correction
contains no native punctuation parser, manual string serializer, or vendor patch.
The strict grammar belongs to Glaze: its validation options enable validated
skipping and complete trailing-whitespace validation
(`vendor/glaze/a4a7/include/glaze/json/read.hpp:5210`). Its object skipper requires
a key after a comma and an object close after the last value
(`vendor/glaze/a4a7/include/glaze/json/skip.hpp:63`, `:86`, `:98`).

The second pass is bounded by the first pass's 1 MiB raw input, 128 containers,
and 100,000 values; it does not allocate another structural tree. The structural
reader still applies UTF-8, duplicate-key, NUL, numeric and value policy before
conversion. A successful first pass cannot hide further containers in strings
from the strict pass: both passes use Glaze's string/value handling. Library
recursion failures defensively map to `resource_limit/json_depth` using the
effective decoder ceiling (`codec.cpp:261`, `:270`). Strict-pass diagnostics use
the library's zero-based byte count (`:273`).

The raw-byte check remains first. Structural resource/schema refusals retain
their previous precedence; malformed input containing `null`, fractional values
or decoded duplicate keys can therefore receive `schema` before the strict pass.
This does not accept malformed JSON or weaken a resource bound. Independent
probes verified this documented ordering, tightened limits, and hard ceilings.
No additional blocking finding was identified. The earlier N03-A assessment of
the unchanged encoder, dependency wiring and bounded native allocations carries
forward; only `codec.cpp` and `codec.t.cpp` differ from that candidate.

## Exact identity

`identity-before.json` and `identity-after.json` independently verify all six
candidate files and all 37 frozen correction-evidence files against their
manifests. Every entry passed before and after verification, and the two identity
records agree. Candidate manifest SHA256:
`1c1308cb87241e03913bfb0424a129d1af297e511ee35072dc39c4a382fbcbab`.
Correction-evidence manifest SHA256:
`3afc8e2be48fbd7a48d85a3c13fcaf36f5a4a86530d8db730f6070ff941c15ec`.
`git diff --exit-code HEAD -- vendor/glaze` passes before and after; all 663
restored files remain HEAD-exact. The correction diff and prior failing evidence
remain untouched.

## Independent verification

Commands, working directories and raw output are preserved in the reviewer
evidence directory. Executed serially against `build/rewrite-debug`:

- Compiled and ran the new public-API `probe.cpp`: **1,054 checks, 0 failures**.
  It systematically tests missing/trailing object commas across six whitespace
  forms, six scalar/container/string values and three nesting wrappers; valid
  counterparts; other separator/colon/comment/trailing-input failures; Unicode
  byte offsets and escaped punctuation; malformed inputs exactly at 128
  containers, 100,000 values and 1 MiB; next-over and tightened resource limits;
  and prior schema-refusal precedence. `probe.log` is the final result.
- Independently rebuilt and ran the original reviewer public-API probe against
  the corrected library: **243 checks, 0 failures** (`original-probe.log`).
  Its source and original failing evidence were preserved.
- Ran `build/rewrite-debug/src/engine/planar_json_codec_tests '[codec]'`:
  **8 cases / 115 assertions pass**, zero skips (`codec-tests.log`).
- Ran `cmake --build build/rewrite-debug --target all planar_tests`: passes,
  **zero build steps**, `ninja: no work to do.` (`build.log`). This confirms
  the candidate's built module/library graph is current.
- Inspected coder red evidence: all six original malformed-object variants
  failed before the fix. Inspected final build, 28-file pinned formatting/
  Doxygen and no-change logs; they pass. The 121-case listing is registration
  evidence, not a full-suite execution claim.

The first reviewer probe run had three failed checks because its own supposed
1 MiB input was one byte short. `initial-probe.cpp.txt` and `initial-probe.log`
preserve that harness error. The final probe computes padding from the prefix
length and asserts exact construction; it passes without changing the candidate.
Reviewer executable hashes are retained; owned executables/debug bundles were
removed after verification.

## Approval boundary and limitations

This approval permits N03-B integration of the reviewed codec. It does **not**
complete the user's JSON replacement goal: active product consumers still use
the previous custom JSON paths. N03-B must remove the handwritten native/Lua
JSON handling, adapt the bridge/policy/offset contracts, adopt the intentional
Glaze string spelling, update affected references/expectations, and complete its
independent review and integrated product gates.

This focused review did not rerun the other product tests, release/Linux gates,
sanitizers, allocation-failure injection, RSS measurements or concurrency tests.
Native structural and output-buffer allocations remain separate from Lua
allocator metering. These limitations do not claim broader platform or memory
coverage than the executed checks.
