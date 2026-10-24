# R13-A codec candidate evidence

Shared checkout: `<repo>`,
`dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional uncommitted rewrite
was preserved. No Planar tools, installed binary, operator data, commits,
installation, publication or merge were used. This is coder evidence for an
independent reviewer, not approval.

## Exact candidate

`candidate-sha256.json` lists all twelve owned candidate files, with exact copies
under `candidate/`. Manifest SHA256:
`64215ea0423fa4ff1a3de29e644095b1a17ca6e82820f515405f098b0be5fa3d`.
`baseline/` retains the six pre-existing owned files before edits;
`owned-changes.patch` compares those files, rather than claiming the large
pre-existing Git rewrite as this round's work. Newly created source/fixtures are
fully retained in the candidate snapshot. `baseline-sha256.json` and
`protection-audit.json` establish no change to twenty captured protected files
(hash, ledger, trusted Lua and dependency definitions). The existing
`canonical_encode` function is byte-identical to baseline; its unchanged digest
is also recorded. The encoder implementation preceding it is unchanged except
for the inserted decoder class after the original encoder class.

The new pure `planar.engine.history.record` module has no ledger, runtime or
filesystem imports. It uses the private pinned xxHash target directly, preserving
render hash recipes. Directory IDs are included in stored bytes and omitted
precisely from portable digest preimages. `canonical_v1` is the sole supported
value payload codec; a valid unknown codec identifier is `unsupported_codec`,
with no payload interpretation. Malformed identifier text is `invalid_record`.
This supported-codec distinction follows coordinator direction during the round.

## Executed checks

Commands ran serially per `build/rewrite-debug`; logs preserve complete output.
The formatter was the pinned compiler sibling
`/opt/homebrew/opt/llvm/bin/clang-format`, applied only to six owned C++ files.

| Command | Raw log | Result |
| --- | --- | --- |
| `cmake --preset debug` | `configure.log` | Exit 0 |
| `cmake --build build/rewrite-debug --target all planar_tests` | `build-initial.log` | Exit 0 |
| `ctest --test-dir build/rewrite-debug -R 'replacement.(canonical\|hash\|record)' --output-on-failure` | `focused-initial.log` | Exit 8; two test-fixture offset mistakes described below |
| same build command | `build.log` | Exit 0 |
| same focused CTest command | `focused.log` | Exit 0; 17 cases, zero failures/skips |
| same build command | `build-final.log` | Exit 0 |
| same focused CTest command | `focused-final.log` | Exit 0; 19 cases, zero failures/skips |
| same build command | `build-codec-policy.log` | Exit 0 |
| same focused CTest command | `focused-codec-policy.log` | Exit 0; final 19 cases, zero failures/skips |
| `ctest --test-dir build/rewrite-debug -R '^replacement.cli$' --output-on-failure` | `cli.log` | Exit 0; one Python acceptance case, zero failures/skips |
| `python3 tests/vectors/generate.py --check` | `render-vectors.log` | Exit 0; 12 hand bytes, 20 upstream hashes, 41 canonical/domain vectors |
| `python3 tests/vectors/records/generate.py --check` | `record-vectors.log` | Exit 0; ten independent record framing/digest vectors |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | `lint.log`, final `lint-final.log` | Exit 0; formatting/Doxygen over 40 active source files |
| same build command | `no-work.log` | Exit 0; exactly `ninja: no work to do.` |
| `ctest --test-dir build/rewrite-debug -N` | `test-registry.log` | Exit 0; 182 registered cases |

The focused nineteen comprise seven canonical, three hash and nine record cases.
The registry increased from 170 to 182 (three new canonical and nine record).
The full registry was enumerated, not executed in this subround. The renderer
acceptance case covers its existing 247 real CLI invocations and goldens.

## Initial failures and correction

The initial `record.t.cpp` is preserved as `initial-record-test.cpp`.
The independent framing oracle and product codec agreed; two assertions used
incorrect literal offsets. `PLANAR-RECORD` has thirteen ASCII bytes, followed by
one version byte. The first child ID starts at byte offset 37, the second at 73.
The original preimage test erased offsets 36 and 77. The corruption test also
mutated one already-zero ID byte rather than setting the ID to zero. Corrected
byte positions preserve the intended omission and ID-corruption checks. These
were fixture mistakes, not a codec regression; no product refusal was relaxed.
Their raw failure output remains intact.

## Bounds and limits

Independent fixed vectors and exhaustive proper-prefix truncations cover all four
record kinds. Tests also cover noncanonical integers, raw binary canonical strings,
UTF-8 map keys, empty map/list distinction, private tags, key ordering/duplicates,
depth/node/byte limits, malformed lengths/counts, directory aliases/order/IDs,
unsupported codecs, request text/head tokens, exact 16 MiB file/value payloads
and the 64 MiB whole-record ceiling. The giant-length rejection inputs contain
only headers; they do not allocate the claimed payloads.

This subround establishes local framing and pure codec behavior. Contextual tree
reference integrity, interning, forced collisions/rollback, provenance assembly,
accepted history and load semantics remain later R13/R14 work. Full integrated
debug/release gates remain the R13 milestone. Executed host: macOS Apple Silicon;
Linux execution is not established. There was no new failing product regression
requiring a corrective round in this candidate.
