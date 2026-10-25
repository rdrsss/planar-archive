# V5-02-X06-B1 independent review

Disposition: **request-changes**. Two required changes, **X06B1-R1** and
**X06B1-R2**, block approval of this candidate. Neither needs a product choice
from the operator.

The product extraction itself is faithful. `ledger.storage`, `ledger.acceptance`,
`ledger.storage_types` and `acceptance.intent` match the architecture's "Native
completion dependency extraction" row. The target graph has no cycle, all four
generated variants build and reverse exactly, all 918 earlier test names are
still registered, and the full Debug suite of 928 tests passes.

The first blocking problem is a build-graph regression. Editing the new storage
or acceptance sources no longer regenerates the three exact-source test variants
that copy them (R1). The second is the evidence package. It is missing the
agreed report and dependency manifest. Its focused gate did not run most of the
variant tests this repair targeted, and it reported a failed no-work check that
was really the first full build (R2).

The review ran in the shared checkout on `dev/planar-v2-vernacular`, HEAD
`6c30ecdbc63af1fae690774108eb7d37cd4ffbf8`, against the uncommitted candidate. I
wrote only this file and new files under
`tests/native/v5-02-x06-b1-evidence/review/`. I did not edit, revert or reformat
any product, test, process or author-evidence file. I used no old Planar tool,
queue, skill, role or installed binary. I did not commit, install, publish or
merge anything. I ran builds and ctest one at a time in `build/rewrite-debug`. A
separate configure-only tree under the session scratch directory produced the
target graph.

## Exact candidate

| Identity | SHA-256 |
| --- | --- |
| `git diff` (19 modified tracked files), at the start and end of review | `4ebf3fa3391fe3a26d4848aa688ee9ba6a2d9c85b7639bb07a5a478d7895de1a` |
| `git diff` without `docs/vernacular/orchestration.md` | `7c99eae601135d90ad6cdbfae0f29c10c450596173d9c644b2372e2a6a9fbd25` |
| `src/engine/ledger/storage_types.cppm` | `9bdfa2d88910bb57238d1553ee1286b81fea5ae15a24f4dcefb81c566b2fe42a` |
| `src/engine/ledger/storage.cppm` | `7fafed46b7a1058110ea2b1b9ab1f73246b02a21e9d83e6491e75f0f225310d3` |
| `src/engine/ledger/storage.cpp` | `211eb2e9c2ae6bf3169c5b02fbf4653ed161454cb88789b3c84b112bd3203943` |
| `src/engine/ledger/storage.t.cpp` | `7a21276baf66ced8ce9de11544c3dcb5c2b8c6d2fc52153a59d2f61dd9b5efac` |
| `src/engine/ledger/acceptance.cppm` | `54ae7b90abc366143d5e30f7029a14b33d8036f788d3fa5f4ccbd43097423327` |
| `src/engine/ledger/acceptance.cpp` | `ab2fc8c7940e6503d3b2fd2dcc51caa92882ffd37dbbc0cef2afe2750bdf6704` |
| `src/engine/acceptance/intent.cppm` | `acfca679ad1a6927e77d6865cf84813e0be7097dbc0e69ed73cc7c9658bbacf5` |
| `src/engine/acceptance/intent.cpp` | `3b3b63ba6e0bfb31d5a5668697f878af09b9e417cdb829af4258433243949560` |
| `src/engine/acceptance/intent.t.cpp` | `b644ef7cde6f18834bfb91d2ea300252afd74699a8590279b136363089719803` |
| `docs/vernacular/rounds/V5-02-X06-B1.md` | `afb5609f09d0c146d62f7acda30ebd6f4692956220dcc18bf30740ba16680b7b` |
| `scripts/redact-local-paths.py` (process file) | `83d5041ac80aebbe112ad38d7621d04987db718745751200897979008d9cb1d8` |

[`candidate.json`](../../../tests/native/v5-02-x06-b1-evidence/review/candidate.json)
lists every modified and untracked file hash. The dispatch lists
`tests/native/r13b1-support/collision.t.cpp` as a candidate file, but it is
unchanged from HEAD. The orchestration document and the redaction script are
process files and were not reviewed as product scope.

## Required change: X06B1-R1 — Regenerate the variants when their new sources change

Before this round, the r13b1, r14c3b2 and x03-b2-b-c01 generators read only
`ledger/format.cppm` and `ledger/format.cpp`. Those two files are in the
directory's `CMAKE_CONFIGURE_DEPENDS`, which the r12c2 block in
`src/engine/CMakeLists.txt` sets around lines 195–198. Any edit to them re-ran
configure, so every exact-source variant was regenerated from current bytes.

The candidate generators now also read `storage.cppm`, `storage.cpp`,
`acceptance.cppm` and `acceptance.cpp`. The r13b1 generator uses the two storage
files. The r14c3b2 and x03 generators use all four. None of these files was
added to `CMAKE_CONFIGURE_DEPENDS`. The `build build.ninja` regeneration rule in
`build/rewrite-debug` lists only `format.cpp` and `format.cppm` among ledger
sources; see
[`regeneration-inputs.txt`](../../../tests/native/v5-02-x06-b1-evidence/review/regeneration-inputs.txt).

**Failure scenario.** A later round edits `storage.cpp`, for example the interning
or digest-bucket code. The real storage library rebuilds, but the generated
storage clones in `r13b1-generated`, `r14c3b2-generated` and
`v5-02-x03-b2-b-c01-generated` keep their old bytes until something else triggers
a configure. The collision, atomic and metadata-continuity tests then pass or fail
against stale code while claiming to be exact-source variants. That breaks the
round's own criterion that generated variants keep their original intent and
reverse exactly from the current source.

The X06 profile-transaction rounds will edit `ledger/acceptance.cpp` and probably
the storage files. Those are the edits this mechanism exists to track.

**Required.** Add `ledger/storage.cppm`, `ledger/storage.cpp`,
`ledger/acceptance.cppm` and `ledger/acceptance.cpp` to the configure dependencies
of the variant generators that read them. Then show from the regenerated
`build.ninja` that they are regeneration inputs. Covering every input in the
generators' file loops is the safest form. No product source changes.

## Required change: X06B1-R2 — Complete the evidence the round agreed

The dispatch and the repair record require `tests/native/v5-02-x06-b1-evidence/`
to hold a `REPORT.md` plus a dependency-cycle audit manifest. Only raw logs
exist there. The latest gate logs were written instead to the untracked
`docs/vernacular/rounds/V5-02-X06-B1-evidence/`, and they contain machine-local
absolute paths. Specific gaps:

1. **No report and no dependency-cycle manifest.** Agreed verification item 5 has
   no artifact.
2. **The focused regex missed most of the repaired variants.** The pattern
   `ledger|acceptance|prepared|load|snapshot|history|record_digest|publication|storage|intent`
   matched 259 tests. It selected 0 of the 5 `planar_atomic_format_test_tests`
   cases (the r14c3b2 variant) and 1 of the 3
   `planar_metadata_format_test_tests` cases (the x03 variant). It also selected
   0 of 6 `planar_source_export_tests` cases, 1 of 4 export-fault cases and 72 of
   90 `planar_ledger_format_tests` cases. The gate summary therefore does not
   show that the variants this repair targeted still pass.
3. **The 918-name preservation was claimed but not shown.** The ctest log only
   proves 259 passing tests.
4. **The no-change result misreads the run.** The gate's plain build relinked
   `bin/planar` because the build before it targeted only `planar_tests`. It was
   the first full build, not a second one. No zero-work second build was
   recorded.
5. **Criterion 5 was not reported.** The search's re-anchoring of the
   architecture's "around" line citations (advisory X06A4C01-N2) is not recorded.

**Required.** The corrective round's gate must put a `REPORT.md` and a
dependency manifest under `tests/native/v5-02-x06-b1-evidence/`. The report must
cover:

- the executed commands, unpiped exit codes and matched counts;
- a test selection that includes every variant and new binary, or the full Debug
  inventory;
- an inventory diff against the 918 approved X06-A3 names;
- a full build followed by a separately recorded zero-work build;
- the re-anchored citation lines. My readings, for reference:
  `src/engine/runtime/runtime.cpp` 4807–4811 for paired admission and 5071–5076
  for retained bound reads; `src/engine/capture/native.cpp` around 217–251 for
  `same_selection` in the bound read.

Decide whether the duplicate logs under `docs/vernacular/rounds/` are removed or
pass through the redaction step. They should not be committed as they are.

The independent results below show what that evidence should establish. They do
not replace the author package.

## Source assessment against the architecture row

**Module ownership and imports.** `storage_types.cppm` holds the eight relocated
inert definitions with unchanged namespace and layout. It also holds the
`format_error`, `format_error_code`, `record_reference` and `stored_record`
aliases, and imports only `ledger.types` and `ledger.history_codec`.

`storage.cppm` imports `storage_types`, `sqlite` and `history.tree_core`.
`storage.cpp` adds the codec, verifier and canonical modules. Neither imports
runtime, prepared, `ledger.acceptance` or `ledger.format`.

`format.cppm` re-exports `storage_types`, imports storage without re-exporting
it, and no longer imports prepared or acceptance. No compatibility re-export
exists. `acceptance.cppm` imports `storage_types`, `completion`, `prepared` and
storage, but not runtime or format. `intent.cppm` imports only value, record and
`request_v2`, and prepared re-exports it. The command's invocation and the tests
add their imports explicitly.

**Behavior preservation.** I compared the HEAD `format.cpp` (2238 lines),
normalized for the `format_state`→`storage_state` and
`format_read_guard`→`storage_read_guard` renames, with the new `storage.cpp`,
`format.cpp` and `acceptance.cpp`.

- Verification, decoding, history verification, interning, open/read, locked
  entry and initialization/publication/recovery code moved byte for byte, apart
  from namespace placement and `identical_content`→`identical_content_rows`.
- `begin_write` reproduces `begin_records` exactly: read-only verification, held
  identity, snapshot release, sidecar checks, `BEGIN IMMEDIATE`, locked
  `verify`, and identity plus complete metadata equality.
- The completion algorithm keeps its order exactly: receipt reuse, prelude
  identity, replay before candidate inspection, format/codec family, the explicit
  `Unsupported profile acceptance` refusal, head CAS, workspace, the five
  interned roots plus body, previous revision, noop, row append,
  `verify_before_commit`, `candidate.verify()`, then commit with
  confirmed-noncommit evidence and receipt phases.
- `append_acceptance_rows` keeps the same single timestamp, allocator,
  revision/event/request/current-state SQL, encoding order and refusal texts.

**Entry and precommit checks stay separate.** `begin_write` is the entry check.
`verify_before_commit(site)` repeats held-file identity, full `verify` and
complete metadata equality including format. Its site argument selects only the
original record-writer or acceptance message. `record_writer::commit` and the
acceptance path each call it before `commit`. The x03 metadata variant keeps
distinct `begin`, `writer_commit` and `acceptance_commit` injection points. All
three of its tests pass.

**No handle or callback escape.** `write_transaction` exposes only the finite
typed methods from the architecture, plus `read_record`, which the existing
`record_writer` facade needs. It has no accessor for `connection`, statement,
callback or caller SQL. Every failing operation releases the shared state
through `transact`, so later calls refuse with `Unavailable record transaction`.
Commit and abort release the state too. A request miss does not poison the
transaction. The new storage tests cover each of these cases.

The `native` helper namespace exports `execute`/`scalar(connection&, sql)` for
format's initialization and recovery code. A caller cannot obtain a
transaction's connection through it (see N2).

**Generated variants.** I re-ran all four generators into scratch directories.
Each passes its declared-substitution and exact-reversal checks, and its output is
byte-identical to the current `build/rewrite-debug` generated trees. The
`--mutate-equality` (r13b1) and `--mutate-final-receipt` (r14c3b2) modes still
apply exactly one change, now in `storage.cpp` and `acceptance.cpp`.

- r12c2 still redirects all of `fsync`/`linkat`/`unlinkat`. All of those calls
  remain in `format.cpp`; `storage.cpp` and `acceptance.cpp` contain none.
- r13b1 and r14c3b2 still force both digest sites, which moved together into
  `storage.cpp`.
- No variant file imports the real storage, format or acceptance module.
- The changes to the variant test files touch only imports.

**No profile writer or public change.** The source tree has no
`complete_profile_acceptance`, profile writer or `prepared_profile`. Runtime,
handlers, history, capture and shipped Lua are unchanged. Public init still
calls `initialize_new`, which creates format1.

## Independent verification

All logs are under
[`tests/native/v5-02-x06-b1-evidence/review/`](../../../tests/native/v5-02-x06-b1-evidence/review/).
Each command was run unpiped, with its exit code read directly.

| Check | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target planar_tests`, then a plain build, then both again | exit 0 every time; every run printed `ninja: no work to do` |
| `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j6` (full inventory) | exit 0; **928/928 passed**, 0 failed, not run, skipped or disabled |
| Inventory compared with `v5-02-x06-a3-review-evidence/inventory.json` | 918 prior names, 0 missing; 10 added (6 storage and 4 intent tests); 0 disabled |
| Named variant and new-binary cases in the full log | r12c2 fault 6, r13b1 collision 3, r14c3b2 atomic 5, x03 metadata 3, storage 6, intent 4; all passed |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0; formatting and Doxygen checked 133 active sources; `storage.cppm` `descriptor::fd` is now documented |
| CMake target graph (`--graphviz`, configure-only scratch tree) | 254 targets, 1321 edges; the only cycles are the existing self-edges from `catch_discover_tests` on every `*_tests` target; no multi-target cycle |
| Forbidden reachability | storage reaches none of runtime, prepared, acceptance or format; format reaches no acceptance, prepared or runtime; acceptance and the variant acceptance clones reach no runtime; runtime does not reach `planar_ledger_acceptance` |

Test-only links are limited to test executables:
`planar_ledger_storage_tests`→format and
`planar_ledger_format_tests`→acceptance/prepared. No library depends on them.

## Advisory notes

- **X06B1-N1.** The locked replay lookup now uses the reader's `lookup_request`.
  The original inline query mapped prepare and bind failures to the generic
  `io` refusal and did no key check. The new path rechecks the key, which request
  intent has already enforced, and classifies prepare and bind SQLite errors
  through `storage(error)`. For valid intents this differs only on prepare or
  bind failures under the held write lock, where `SQLITE_BUSY`/`SQLITE_CORRUPT`
  are practically unreachable. Record this as an accepted difference or restore
  the exact mapping.
- **X06B1-N2.** `ledger.storage` exports `ledger.sqlite` and the `native`
  helpers, which include generic `execute`/`scalar` over a caller-owned
  connection. HEAD format exported `ledger.sqlite` as well, and these helpers
  cannot reach a transaction's connection. A later round could narrow this
  surface once initialization and recovery ownership settles.
- **X06B1-N3.** The gate's first plain-build log recorded only dyndep
  regeneration and a relink of `bin/planar`. This review confirmed zero work
  afterwards. Future gates should run a full build first so the second build can
  show zero work.

## Limits

This is focused macOS Debug evidence only. No RelWithDebInfo, full release,
Linux or X06 integrated gate is claimed. I did not demonstrate the stale-variant
failure (R1) by editing `storage.cpp`, because I do not own that file. The
evidence is the regeneration rule in `build.ninja` plus the generators' input
lists. The latest complete approved gate remains X05-C5's 876 per configuration.
