# R14-B1 independent review

Disposition: **approve**. No findings requiring changes.

Reviewed the exact six-file candidate in the shared checkout. Product files and
prior evidence were read-only throughout this review. Reviewer writes are this
report and `tests/native/r14b1-review-evidence/`; no commits, installation,
publication, operator data, legacy Planar tools or queue were used. The debug
build directory was exclusively assigned to this reviewer and is now released.

## Frozen subject and evidence

Candidate embedded identity:
`6c7c61e8bb02b5ca25dc123bc52bdb92ecdbb98575779fc02976ade6afa5fc6d`.
The six live file hashes, frozen candidate bytes and manifest identity agree.
`tests/native/r14b1-evidence/candidate.json` file SHA256 is
`df2f75b1c1a5b4e1426c85b2947a331b2d927b94796933971d6de3b3945fd958`.

Coder evidence embedded identity:
`9ad4d2a8ac0e53552bb09b51508275eb835219e6839ca8eb1eb6c9d0533ff256`.
All 43 listed evidence file hashes and the embedded identity agree.
`tests/native/r14b1-evidence/evidence.json` file SHA256 is
`6e7f6906f13a03332f114c0671965e7eea217a9aa2bd844434fbaa15edda2fa5`.
Embedded identities hash the canonical file-hash mappings; they are distinct from
manifest file hashes. These manifests and all 12,213 protected file hashes were
checked again after verification. No candidate or protected file changed.
Coordinator planning/status exclusions remain those declared in the coder audit.

The review evidence manifest at
`tests/native/r14b1-review-evidence/evidence.json` freezes this report and the
reviewer sources, commands, results and raw logs. `initial-audit.json` records the
independent manifest/protection/report audit. Earlier failed authoring/build/lint
attempts remain preserved and are accurately distinguished from final results.

## Contract and implementation assessment

The applicable authority is CLAUDE.md, implementation/roadmap, the v4
accepted-history design, ledger-contract.md and R14-B/B1. This review covers pure
v1 DTO/codec boundaries only; B2 retains all SQL and cross-row obligations.

- `history_codec.cppm:7` exposes only inert enums, strings, optional signed-64
  heads and named record references. Version and load kind are codec constants.
  Error values cannot carry input, host paths or payload content.
- `history_codec.cpp:32` enforces initialization with absent heads/references,
  acceptance with a nonempty newer result head and positive references, and
  no-op with the same nonempty heads and complete references. Numeric record-ID
  gaps remain allowed; no unapproved consecutive-ID rule was introduced.
- `history_codec.cpp:46` validates ledger identity/request grammar and result
  parent positivity/ordering. A no-op parent belongs to the returned revision;
  it is not forced to the unchanged operation head. Agreement with the actual
  revision row is correctly left to B2.
- `history_codec.cpp:78` validates canonical positive decimal-string IDs through
  `from_chars` with signed-64 overflow and complete-consumption checks. The
  optional `none` token cannot substitute for a required reference.
- `history_codec.cpp:92` checks the inclusive one-MiB byte bound before parsing
  and uses the existing bounded Glaze adapter. `history_codec.cpp:133` and `:186`
  require fixed fields/types; `:164` and `:196` compare typed library re-encoding
  against every input byte. The existing Glaze adapter preserves and rejects
  duplicate keys and owns grammar, escaping, UTF-8 and serialization. New code
  adds no product JSON grammar or escaping implementation.
- Exact initialization bytes match both the independent literal/oracle and the
  unchanged `format.cpp:220` initializer composition. Accepted/change/no-op/
  revert/max-ID wires and meaningful typed round trips are pinned by the new
  colocated tests. Fixed v1 field bounds cannot yield a valid near-one-MiB
  payload; exact-limit malformed and over-limit resource refusals test the
  boundary without padding extensions.
- The candidate CMake change adds only the new pure module/tests. Its exact
  link command contains codec/json/value and Catch2/std leaves, with no
  SQLite, format, capture, runtime, Lua or history algorithms. The test target
  remains outside default `all`. Architecture/testing references describe the
  actual boundary and remaining integration limits.
- Protection hashes confirm no format/schema/Lua/CLI/runtime/vendor changes.
  The unchanged `format.cpp:507` refusal of populated revision/request rows
  remains present. This approval enables no accepted-history read or writer.

## Independent verification

All commands ran directly in the current checkout, serialized where using the
debug build directory. Commands and raw outputs are in the review evidence.

| Check | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target all planar_tests` | Passed; zero Ninja steps |
| `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger\|canonical\|hash\|record\|tree\|provenance\|interpretation)' --output-on-failure --output-junit ../../tests/native/r14b1-review-evidence/focused.xml` | 133/133 passed, zero failures/skips; all 122 baseline names preserved plus 11 new codec cases |
| `ctest --test-dir build/rewrite-debug -R '^replacement.cli$' -V --output-junit ../../tests/native/r14b1-review-evidence/cli.xml` | 1/1 passed; 247 real CLI processes, six goldens, zero skips |
| `python3 tests/native/r14b1-review-evidence/probes.py` | 633 probes passed: 80 exact valid round trips, 541 malformed refusals, 10 unsupported-version refusals, two over-limit resource refusals |
| `python3 tests/vectors/generate.py --check` | 12 hand-byte, 20 upstream hash and 41 canonical/domain vectors verified |
| `python3 tests/vectors/records/generate.py --check` | 10 independent record vectors verified |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | Formatting/Doxygen: 63 active source files, zero warnings |
| Fresh R12-C2/R13-B1 generation into reviewer-owned temporary directories | Every generated source/interface/manifest byte equals configured output; all reverse-identical flags true; no R13-B1 verification mutation |
| Final `all planar_tests` build | Passed; `ninja: no work to do.` |
| Final candidate/evidence/protection audit | Six candidate, 43 coder evidence and 12,213 protected file hashes unchanged; exact pure link unchanged |

The independent probe compiles a separate test-only driver against the exact
built candidate module using the existing BMI and pure link libraries, without
editing product tests or CMake. Python's standard JSON library generates the
expected canonical bytes. Coverage includes every required field and nested
field, wrong types, extra/missing fields, duplicate and escaped-alias keys,
escaped values, separators/comments/trailing bytes/BOM/NUL/invalid UTF-8,
request grammar across all 256 character values, inclusive request length,
maximum/overflow/noncanonical IDs, optional tokens, initial/change/no-op heads,
no-op existing parent, version typing, structural exhaustion and byte ceilings.
Accepted probes must re-encode byte-for-byte; refusals must return the exact typed
error. Sources and commands remain reproducible; generated driver binary/object
were removed after verification.

This is macOS Apple Silicon debug evidence. Release/Linux execution and the
integrated R14 gate remain outstanding. SQL row agreement, chronology, actual
stored parent consistency and deep accepted-history verification remain B2 work.
