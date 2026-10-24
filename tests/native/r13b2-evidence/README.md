# R13-B2 implementation evidence

Candidate for independent review, not approval. Shared checkout
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
The intentional rewrite, prior evidence, schema, SQLite primitive, record codec,
CLI/Lua and vendors are preserved. No Planar tools, operator state, installed
binary, commit, install, publication or merge was used. Debug builds ran directly,
serialized; the directory is released on delivery.

The new `planar.engine.history.tree` module accepts sealed one-root logical
path/payload views and normalized numeric modes. `build_tree` returns an exact
directory reference in the caller-owned writer without committing. Both write
and pending-tree read operations abort the entire writer on input, limit or
storage refusal, and an RAII guard also aborts during exceptions. Earlier pending
records cannot commit after refusal. The only ledger behavior added is explicit
idempotent `record_writer::abort() noexcept`, which releases existing RAII state.
Tree errors retain original typed ledger errors.

Profiles are fixed, independently selected source/provenance bounds. Root counts
as one directory at depth zero; files count toward depth 64. Provenance uses the
induced `1 + 4096 * 63` directory maximum, not source's 8192. Construction stores
local names in a bounded trie and creates only file ancestors plus root.
Reconstruction checks root kind/digest, disallows value/request children and
non-root empty directories, checks expanded occurrence budgets before enqueueing,
and retains only one decoded record at a time. All returned files and payloads
are owned and sorted. No fresh filesystem capture, timestamps or Lua replay is
part of this boundary.

## Commands and results

All commands run from repository root, with exit zero unless noted.

- `cmake --build --preset debug --target all planar_tests`:
  `build-initial.log`, `build-final.log`, `build-final-expanded.log` and
  `build-final-storage-prefix.log`. All succeeded. Generated C2/B1 exact-source
  variants were rebuilt after the narrow abort API addition.
- `build/rewrite-debug/src/engine/planar_history_tree_tests`:
  initial `tree-initial.log`, 13 cases / 671 assertions passed. This preceded
  local-name scratch reduction and strengthened tests, which the final focused
  run below exercises. No product defect was being corrected; this is a new
  API/feature, so there is no before-fix regression claim.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record|tree)' --output-on-failure --output-junit <repo>/tests/native/r13b2-evidence/focused.xml`:
  final `focused.log` / `focused.xml`, **93/93 passed, zero failures/skips**,
  35.54 seconds. This is 79 retained cases + 13 tree + one ledger abort case.
  The 13 final tree cases passed 691 assertions. `focused-initial.*` preserves
  the earlier 93-case run before test strengthening.
- `python3 tests/vectors/generate.py --check`: `vectors.log`, 12 hand byte,
  20 upstream hash and 41 canonical/domain vectors verified.
- `python3 tests/vectors/records/generate.py --check`: `record-vectors.log`,
  10 independent record vectors verified.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  `lint.log` and `lint-final.log`, pinned formatter and Doxygen, **44 active
  source files**, zero warnings.
- `python3 tests/native/r13b1c01-evidence/helper-lint.py`: `helper-lint.log`,
  unchanged pinned formatter/Doxygen check of both B1 helper C++ files.
- `cmake --build --preset debug --target all planar_tests`: `no-work.log`,
  exactly `ninja: no work to do.`
- `python3 tests/native/r13b2-evidence/audit.py`: `audits-final.log`, all **3746
  protected input files** unchanged; 50 original B1, 16 B1 review, 48 B1-C01
  and 18 B1-C01 review evidence entries unchanged. Fresh reversible C2/B1
  regeneration exactly matches configured bytes/manifests. B1 still redirects
  both digest call sites with no equality mutation. Candidate snapshots, 93
  focused results and zero-work build all verified.
- The initial audit harness incorrectly treated C2's top-level manifest mapping
  as B1's `files`-wrapped schema: exit 1 `KeyError: 'files'`. Exact failed script
  and output are retained in `audit-initial.py` / `audits-initial.log`. The local
  harness now checks both schemas explicitly; no product or prior evidence was
  changed to resolve this. `audits.log` retains the first successful rerun.
- `ctest --test-dir build/rewrite-debug --show-only=json-v1`: `registry.json`,
  **210 registered cases**, not a claim of running the complete registry.
- Scoped `git diff --check` passed. New-file whitespace is also covered by the
  pinned formatter and snapshot review.

Tests cover binary/CRLF/empty/executable files, mixed-case byte sorting, empty
root, reuse, portable digest despite displaced database IDs, rename/delete/
same-length edits, old-tree reconstruction and owned lifetime. Exact/excessive
bytes, files, source directories, depth and path length are checked for both
profiles where applicable. Shared subtrees expand to the full provenance
maximum of **258,049 directories and 4096 files**, and expanded payloads obey
exact source/provenance byte limits. Inputs reject duplicates, case aliases,
file/directory conflicts, unsafe grammar and unnormalized modes. Reconstruction
refuses wrong root kinds/digests/IDs, value/request descendants and empty
non-root directories. Writer failure paths verify no pending prefix can commit.

The typed storage regression seeds an independently encoded valid file at ID
`9223372036854775806`. Tree insertion has room to insert its novel file at the
maximum ID; inserting the parent directory then fails with the original
`unsupported_state`. The writer refuses commit, a fresh verified reader sees
only the seed, and the maximum-ID file is absent. This proves rollback after a
tree inserted a prefix without adding a product hash override. Existing all-kind
forced-collision and actual SQLite I/O-fault cases remain in the focused suite.

## Snapshot and manifest path bases

`candidate.json` maps nine repository-relative owned paths to SHA256. Exact
snapshots use those same paths under `candidate/`. Its identity is SHA256 of
the compact sorted-key UTF-8 JSON `files` mapping with separators `,`/`:` and
no newline. `candidate.diff` compares against the six exact pre-round files
under `before/`, with new files compared against empty, using repository-relative
labels. `protected-before.json` maps unchanged repository-relative input paths
to SHA256. Round records are coordinator owned and excluded from that baseline.

`evidence.json` maps paths relative to this evidence directory to SHA256 and
excludes only itself. It includes scripts, logs, baseline and candidate snapshots,
registry, manifests and diff. No manifest asserts independent approval.

Executed platform is macOS Apple Silicon debug. Release, Linux, full integrated
R13 gates, acceptance/provenance assembly and CLI wiring remain later work.
Exception safety is enforced by the abort guard; no allocation-failure injector
was added. Existing lower-level I/O fault tests remain active rather than adding
another fault seam to the tree implementation.
