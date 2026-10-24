# R13-A independent review

Disposition: **approve**. No blocking or requested-change findings in the
assigned pure-codec scope. This approves the exact candidate below, not later
history/storage/acceptance behavior or the complete R13 milestone.

## Candidate, authority and scope

Fresh reviewer `/root/r13a_reviewer` independently reviewed shared
`dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The checkout contains the intentional
uncommitted rewrite. Only this report and
`tests/native/r13a-review-evidence/` were authored; no product fixes, unrelated
reverts, Planar tools, installed binaries, operator data, commits, installation,
publication or merge. `build/rewrite-debug` was exclusively held for direct
serialized verification and is released when this report is delivered.

Read the active repository instructions, implementation plan, roadmap, v4 design,
ledger contract, R13/R13-A briefs and current architecture/testing references.
Reviewed all twelve frozen candidate files, the six-file baseline/diff, original
failing tests and raw logs, independent vector construction and protection audit.
The coordinator's recorded decision that only `canonical_v1` is currently
supported resolves the codec-ID semantics; request normalization and operation
acceptance remain external policy.

The repository-relative twelve-file candidate manifest is
`tests/native/r13a-evidence/candidate-sha256.json`, SHA256
`64215ea0423fa4ff1a3de29e644095b1a17ca6e82820f515405f098b0be5fa3d`.
The folder-relative forty-file coder evidence manifest is
`tests/native/r13a-evidence/evidence-sha256.json`, SHA256
`5d3958e545ebb726685a893bf29984d5130608557a565a7c4ec26b3e3b28eb34`.
All entries and candidate snapshots match before and after review. All twenty
protected files still match their recorded before/after identities. The original
encoder class and complete `canonical_encode` function remain byte-identical to
baseline. Independent audit source/results are in the review evidence directory.

## Technical assessment

- `src/engine/history/record.cpp:8` fixes the 64 MiB whole-record / 16 MiB payload
  ceilings, positive signed-64 IDs and exact thirteen-byte magic plus version.
  Encoding/decoding cover all four normative big-endian frames. Exact modes,
  binary file payloads, local child metadata, portable segment grammar,
  sorting/uniqueness/case aliases, exact request strings and canonical head tokens
  are validated. Lengths and subtraction guards bound owned copies; compact
  malicious claims cannot request their declared buffers or reserve an inventory.
- `src/engine/history/record.cpp:120` uses one framing implementation for stored
  and portable bytes. Only directory child IDs are conditionally omitted
  (`:136`). `:260` calls pinned raw seed-zero XXH3 and canonical sixteen-byte
  conversion, preserving existing render-domain recipes. The record module has
  no database, filesystem, Lua or runtime dependency; CMake preserves the leaf
  boundary and excludes test executables from default `all`.
- `src/engine/canonical/canonical.cpp:106` reconstructs immutable values with
  distinct empty containers and arbitrary binary strings. Keys require valid
  UTF-8 without NUL and strict unsigned-byte order/uniqueness (`:179`). Integer
  bit patterns must encode the existing signed-32 domain. Private/unknown tags,
  trailing bytes, truncations, impossible counts and out-of-range integers refuse.
  Counts are checked against remaining nodes and minimum physical input before
  traversal (`:160`), without attacker-count reserve. Depth, aggregate nodes and
  encoded bytes are clamped at the public boundary (`:211`) and failures unwind
  owned partial trees into typed diagnostics.
- `src/engine/history/record.cpp:71` validates codec-ID text and refuses a valid
  unknown codec before canonical payload interpretation. Malformed framing or
  resource violations can refuse earlier. Unknown record versions/kinds use
  `invalid_record`; unsupported payload codecs and resource limits remain
  distinguishable typed failures. No JSON grammar or serializer was introduced.
- `tests/vectors/records/generate.py` independently frames bytes using Python
  `struct`, hand-transcribed canonical payloads and a separately compiled pinned
  upstream hash reference. Numeric high/low output checks canonical digest order
  independently of the product conversion helper. The ten fixed vectors include
  binary/CRLF/executable bytes, empty directories/map/list, signed minimum,
  exact request arguments and local child-ID versus portable digest behavior.
  Product tests exhaust proper-prefix truncations and exercise exact payload and
  whole-record ceilings. Documentation accurately limits these claims to local
  framing and validation.

The original two test failures are legitimate fixture-offset errors. Independently
compiling the preserved original test source against the final unchanged product
objects reproduces exactly its original failures at lines 61 and 153: 7 cases,
5 passed / 2 failed; 800 assertions, 798 passed / 2 failed. The corrected candidate
passes. Static framing confirms first/second child-ID offsets 37/73; the original
36/77 deletions removed adjacent metadata/digest bytes, and its attempted zero-ID
mutation changed an already-zero byte. The corrections retain the intended
checks; they do not relax product validation.

## Independent verification

Raw logs, probe sources, exact compile/link commands and a reproduction README
are retained in `tests/native/r13a-review-evidence/`.

| Executed check | Result |
| --- | --- |
| `cmake --preset debug` | Exit 0; existing experimental import-std warning only. |
| `cmake --build build/rewrite-debug --target all planar_tests` | Exit 0. |
| `ctest --test-dir build/rewrite-debug -R 'replacement.(canonical\|hash\|record)' --output-on-failure` | 19 passes: 7 canonical, 3 hash, 9 record; zero failures/skips. |
| Record vector generator `--check` | Ten exact record/digest fixtures pass. |
| Existing vector generator `--check` | 12 hand-byte, 20 upstream-hash and 41 canonical/domain vectors pass. |
| Independent native adversarial probe | 9,813 assertions pass; 2,000 generated nested canonical trees, 5,000 malformed byte inputs; unknown tag/version/kind domains, malformed and extreme UTF-8 keys, exact binary strings, aggregate nodes and large compact claims; no skips. |
| Independent native allocation probe | Nine probes pass, including compact claims and exact 16/64 MiB boundaries; no claimed-count reserve. |
| `ctest --test-dir build/rewrite-debug -R '^replacement.cli$' --output-on-failure` | One acceptance case passes, existing 247 CLI invocations; zero skips. |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | Formatting and Doxygen pass across 40 active files. |
| Final `cmake --build build/rewrite-debug --target all planar_tests` | Exit 0; exactly `ninja: no work to do.` |
| Final identity audit | All candidate/evidence/baseline/protected identities match the initial audit. |

The allocation probe observes ordinary host C++ requests, not process RSS or
SQLite/C allocations. Compact malformed claims request at most 112 bytes per
allocation / 176 total for canonical decoding and zero for record decoding.
A 16 MiB file uses one 16,777,232-byte request; a 16 MiB value uses at most that
single request size / 33,554,560 total including its validated canonical tree;
a whole 64 MiB request uses 67,108,800 total bytes. Reviewer harness mistakes
(payload-plus-one versus libc++ allocation rounding, raw binary diagnostic
UTF-8 decoding, and Catch2's configured failure exit 42) are explicitly labeled
with preserved initial source/logs; none was a product failure or required a
candidate change.

The independent evidence manifest contains 28 entries, SHA256
`696d72cce0772076616d70713224d8a3060b7d47b44cffb652e89e38e172530c`.
The report is outside that manifest and receives a separate delivery hash.

## Limits and continuation

This is executed macOS Apple Silicon debug evidence. The 182-case registry was
not run in full here; complete integrated debug/release gates remain the R13
milestone. Linux execution is not established. Contextual graph/reference/tree
validation, collision-safe interning, rollback, complete provenance and accepted
history remain R13-B/C and R14. No approval of those future surfaces is implied.
No product choice remains unresolved within R13-A's reviewed scope.
