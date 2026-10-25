# V5-02-X06-B1-C01 independent review

Disposition: **approve**. The combined B1+C01 candidate closes both required
changes of the [B1 review](V5-02-X06-B1-review.md). **X06B1-R1** and **X06B1-R2**
are closed. This review raises no new required change. Its advisory notes concern
how the evidence is packaged; they do not affect the product or the facts the
evidence establishes.

C01 changes exactly one product-adjacent file. `src/engine/CMakeLists.txt` gains
four `CMAKE_CONFIGURE_DEPENDS` lines and nothing else. Every other B1 source,
test, generator and fixture is byte-identical to what the B1 review approved on
substance. In a scratch copy of the tree, editing any of the four ledger files
re-runs CMake and regenerates every exact-source clone of that file. With the B1
`CMakeLists.txt` restored, the same edit regenerates nothing. That reproduces the
B1 failure and shows the fix closes it.

The evidence package now exists under `tests/native/v5-02-x06-b1-c01-evidence/`,
and the docs folder is gone. The relocated B1 logs match the hashes the B1 review
recorded. The author's gate claims match what I re-ran: 928/928 in full Debug
ctest, all 918 X06-A3 names preserved plus the 10 new ones, lint over 133
sources, and a full build followed by a no-work second build. The cycle manifest
also matches my independent audit.

I reviewed in the shared checkout on `dev/planar-v2-vernacular`, HEAD
`6c30ecdbc63af1fae690774108eb7d37cd4ffbf8`, against the uncommitted candidate. I
wrote only this file and new files under
`tests/native/v5-02-x06-b1-c01-evidence/review/`. I did not edit, revert or
reformat any product, test, process or author-evidence file, and I did not
commit, install, publish or merge anything. I used no old Planar tool, queue,
skill, role or installed binary. Builds and ctest in `build/rewrite-debug` ran
one at a time. The regeneration probe and the target graph each used their own
configure tree in the session scratch directory. The gated tree's `build.ninja`
and `CMakeCache.txt` modification times were unchanged by them.

## Exact candidate

| Identity | SHA-256 |
| --- | --- |
| `git diff` (20 modified tracked files), at review start and end | `e0b6aeca1b33a31d7a4874e69d61e36ac0622c9af7e4abee2999ba49dbba1be0` |
| `src/engine/CMakeLists.txt` with the four C01 lines removed | `1d439dfd6290a54b566eb44e1a4e8d95516ab73e3b84fad58f695904d38e2622` (equals the B1-reviewed file) |
| B1 `git diff` at the B1 review | `4ebf3fa3391fe3a26d4848aa688ee9ba6a2d9c85b7639bb07a5a478d7895de1a` |

[`candidate.json`](../../../tests/native/v5-02-x06-b1-c01-evidence/review/candidate.json)
lists every modified and untracked file hash.

Compared with the B1 review's `candidate.json`, the changes are:

- **`src/engine/CMakeLists.txt`**: the four lines below, verified by the hash
  above.
- **`docs/vernacular/roadmap.md`**: now modified. This is a coordinator status
  update.
- **`docs/vernacular/rounds/V5-02-X06-B1.md`**: changed. The coordinator appended
  the B1 review outcome.
- **The four `docs/vernacular/rounds/V5-02-X06-B1-evidence/*.log` files**:
  removed from the docs folder.

The other 18 tracked files and all 10 untracked B1 sources and process files
match the B1 review's hashes exactly. These include `storage*`,
`acceptance.{cppm,cpp}`, `intent*`, `storage_types.cppm`, all four generators
and the variant tests. `src/engine/ledger/storage.cpp` and `storage.t.cpp` are
unchanged, which is consistent with the decision not to restore N1.

## X06B1-R1 — closed

`src/engine/CMakeLists.txt` lines 195–202 now read:

```cmake
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/format.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/format.cppm"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/storage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/storage.cppm"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/acceptance.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/acceptance.cppm"
  "${r12c2_support}/generate.py")
```

**Generator inputs are fully covered.** Each generator's file loop reads one of
these sets:

- r12c2: `format.{cppm,cpp}`.
- r13b1: `storage.{cppm,cpp}` and `format.{cppm,cpp}`.
- r14c3b2 and x03-b2-b-c01: storage, format and `acceptance.{cppm,cpp}`.

The union of those sets is exactly the six files now listed. Each generator's
own `generate.py` was already a configure dependency, and none of them reads
`storage_types.cppm`.

**The regeneration rule lists all six files.** In the regenerated
`build/rewrite-debug/build.ninja`, the `build build.ninja … : RERUN_CMAKE` rule
at line 19879 lists all six ledger files. I confirmed this read-only with
`ninja -t query build.ninja`; see
[`regeneration-inputs-query.log`](../../../tests/native/v5-02-x06-b1-c01-evidence/review/regeneration-inputs-query.log).
The author's `regen-inputs.txt` records the same rule before and after the edit:
the four paths appear only after it, with 186 tokens before and 190 after.

**A behavioural probe confirms the fix.** Because I do not own the sources, I ran
the probe in a scratch copy of the candidate tree. I configured the Debug preset,
settled the tree, and confirmed that a plain `ninja build.ninja` did not re-run
CMake. I then appended a distinct comment marker to one file at a time. The
results are in
[`regeneration-probe.json`](../../../tests/native/v5-02-x06-b1-c01-evidence/review/regeneration-probe.json).

- **`storage.cpp` and `storage.cppm`.** Editing either one re-ran CMake. The
  marker appeared in the r13b1, r14c3b2 and x03 clones of that file.
- **`acceptance.cpp` and `acceptance.cppm`.** Editing either one re-ran CMake.
  The marker appeared in the r14c3b2 and x03 clones.
- **Negative control.** I restored the B1 `CMakeLists.txt` (hash `1d439dfd…`),
  reconfigured and settled, then edited `storage.cpp`. CMake did not re-run, and
  no generated clone received the marker. This is the stale-variant scenario
  that X06B1-R1 described.

## X06B1-R2 — closed

**Relocation.** `docs/vernacular/rounds/V5-02-X06-B1-evidence/` no longer exists.
The four B1 logs now live under `raw/` as `b1-original-{build,ctest,lint,nochange}.log`.
Their SHA-256 values equal the hashes the B1 review recorded for the docs-folder
copies (see
[`artifact-manifest.json`](../../../tests/native/v5-02-x06-b1-c01-evidence/review/artifact-manifest.json)).
The `tests/native/*-evidence/` gitignore rule covers `raw/`. The redaction
`check` passes, with exit 0, on the files that are meant to be committed:
`REPORT.md`, `dependency-cycle-manifest.md`, the round record and
`src/engine/CMakeLists.txt`.

**Report.** `REPORT.md` lists each gate command with its unpiped exit code and
matched count. The raw `N.exit` files hold 0 for each of them. The report also
honestly notes that step 4 was itself a no-work build because the targets were
already current. The candidate's compile is recorded separately: `raw/build.log`
has the CMake re-run and 533 build steps after the edit, and `raw/gate-build-full.log`
shows a full build that did one dyndep step, followed by
`raw/gate-nochange.log` reporting no work.

**Gate (criteria 3 and 7), re-run independently.**

| Command | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target planar_tests` | exit 0, no work |
| `cmake --build build/rewrite-debug` (full) | exit 0, no work |
| `cmake --build build/rewrite-debug` (separately recorded second build) | exit 0, `ninja: no work to do` |
| `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j6` | exit 0, **928/928 passed**. There are 928 `Passed` lines. The only line matching skipped, not run, disabled or failed is a test name. |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, formatting and Doxygen checked 133 active sources |

**Inventory (criterion 4).** I compared `ctest --show-only=json-v1` against
`tests/native/v5-02-x06-a3-evidence/inventory.json`, which has 918 unique names
and is name-identical to the A3 review inventory. The result is 928 unique
names, 0 missing, 10 added and 0 disabled. The 10 added are four
`replacement.intent` and six `replacement.storage` cases, the same list the
author reports. The author's sorted `inventory.txt` equals my list. See
[`inventory-preservation.json`](../../../tests/native/v5-02-x06-b1-c01-evidence/review/inventory-preservation.json).

**Dependency cycles (criterion 5).** I ran my own configure-only `--graphviz` in
scratch and wrote my own checker,
[`graph-audit.py`](../../../tests/native/v5-02-x06-b1-c01-evidence/review/graph-audit.py).
The graph has 254 nodes and 1321 edges. Results are in
[`target-graph-audit.json`](../../../tests/native/v5-02-x06-b1-c01-evidence/review/target-graph-audit.json).

- There is no strongly connected component with more than one node. All 41
  self-loops are `*_tests` targets.
- All 21 forbidden-reachability checks are false. They cover:
  - storage and storage_types, which must not reach format, acceptance,
    prepared, intent or runtime;
  - format, which must not reach acceptance, prepared, intent or runtime;
  - acceptance, which must not reach runtime or format;
  - intent, which must not reach storage, format, acceptance, prepared or
    runtime.
- The `planar_runtime` node exists, so the runtime checks are not vacuous.
- Running my checker on the author's `raw/target-graph.dot` gives a
  byte-identical result.
- The author's `dependency-cycle-manifest.md` direct-dependency lists agree with
  mine.

**Citations (criterion 6).** Runtime and capture are unchanged from HEAD
(`git diff --quiet HEAD -- src/engine/runtime src/engine/capture` exits 0).
Reading the current source:

- `src/engine/runtime/runtime.cpp`, paired admission. Line 4806 opens the
  workspace branch, 4807 is `admit_root("extension.workspace.root", …)` and 4809
  is `admit_file(key, path, admission_file_role::declaration)`. Lines 4810–4811
  begin the separate root/base-root branch. The exact span is 4806–4809, and
  the B1 review's 4807–4811 reading over-runs by two lines. The author's
  `citations-verified.txt` says the same.
- `src/engine/runtime/runtime.cpp`, retained bound reads. Line 5071 is
  `session.add_root("workspace", …, retain_root("extension.workspace.root"))`
  and line 5076 is `session.read("workspace", file, retain_file("extension.workspace"))`.
  The reading is exact.
- `src/engine/capture/native.cpp`, `same_selection`. Line 217 checks the
  pending-authority replacement and line 251 checks root discovery in the bound
  read. Both fall inside the 217–251 reading.

## Advisories N1 and N2 (criterion 8)

**X06B1-N1 is not restored and is recorded as an accepted difference.** The round
record's Result section states this. I agree it is not a one-line restore. At
HEAD only the inline acceptance query used the argument-less `storage()` (generic
`io`) for prepare and bind failures. The shared `lookup_request` in `storage.cpp`
(lines 1264–1284) also serves `read_snapshot::read_request` and the format
reader. HEAD's `format_file::read_request` already mapped those failures through
`storage(error)`. Restoring the exact acceptance mapping would therefore need a
parameter or a duplicate query. A single-line change would instead alter the
existing read paths. The difference only matters for a prepare or bind SQLite
failure under the held write lock after locked `verify`, and accepting it is
sound.

**X06B1-N2 is recorded as deferred** in the round record. The `ledger.storage`
export surface, including the `native` helpers, is unchanged. This item carries
forward to the round that settles initialization and recovery ownership.

## Scope (criterion 9)

There is no profile writer anywhere in `src`. A search for
`complete_profile_acceptance`, `prepared_profile` or `profile_writer` finds
nothing. `src/stdlib`, `src/engine/runtime` and `src/cmd/planar/handlers` are
unchanged from HEAD. Public init still calls `initialize_new`, which calls
`initialize_selected(…, ledger_format::v1)` (`format.cpp` 322–323). C01 adds no
behaviour change.

## Advisory notes

- **X06B1C01-N1 (report completeness).** `REPORT.md` does not itself contain
  four items: the regeneration-rule excerpt, the confirmed citation lines, the
  N1 and N2 decisions, or links to the sidecar files. The facts are present in
  `regen-inputs.txt` and `citations-verified.txt`, and the decisions are in the
  round record. Under `orchestration.md`, however, only `REPORT.md`/`README.md`
  and top-level `.md`/`.json` manifests are committed, so those `.txt` sidecars
  will stay local. This review records each of these facts in committed form.
  Future reports should fold such facts into `REPORT.md` or a `.json` manifest.
- **X06B1C01-N2 (two gate passes in `raw/`).** `raw/` holds two complete gate
  passes: the `gate-*` set at about 20:12 and the numbered `1`–`5` set at about
  20:15. `REPORT.md` describes only the numbered set. Both passes show 928/928
  and exit 0. Labelling or pruning the extra pass would make the record easier
  to audit.
- **X06B1C01-N3 (artifact hashes).** The author package has no committed hash
  manifest for its local raw artifacts, which `orchestration.md` expects.
  [`artifact-manifest.json`](../../../tests/native/v5-02-x06-b1-c01-evidence/review/artifact-manifest.json)
  records the SHA-256 and size of every author artifact as reviewed.

## Commit guidance

Once the coordinator confirms the candidate is still current, the commit should
cover the B1 candidate files, C01's `src/engine/CMakeLists.txt`, the round
records, this review, the C01 `REPORT.md` and `dependency-cycle-manifest.md`, and
the review's small `.json` manifests: `candidate.json`, `commands.json`,
`artifact-manifest.json`, `inventory-preservation.json`,
`regeneration-probe.json` and `target-graph-audit.json`. All six are under 100
KB and contain no absolute paths. `review/inventory.json` (about 736 KB, with
local paths), the logs, `graph-audit.py` and `regen-probe/` stay local. The
current-candidate check is the `git diff` hash above plus the untracked hashes
in `candidate.json`. Run the redaction `redact` and then `check` steps on the
staged set as required.

## Limits

This is macOS Debug evidence only. No RelWithDebInfo, release, Linux or
integrated X06 gate is claimed. The latest complete approved gate is still
X05-C5's 876 per configuration, and the registered inventory becomes 928 with
this candidate. The regeneration probe ran in a scratch copy of the tree that
left out bulky evidence folders, not in `build/rewrite-debug`. The gated
evidence for X06B1-R1 is the regeneration rule. I did not rebuild the candidate
from a clean tree: every build in this review was a no-work build over targets
the author's run had already compiled. The fresh 928-test run exercised those
binaries.
