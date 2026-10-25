# V5-02-X06-B2 independent review

Disposition: **request-changes**. Three required changes, **X06B2-R1**,
**X06B2-R2** and **X06B2-R3**, block approval. R1 also touches a diagnostic
contract that the candidate's own test calls "different by design". If the
corrective search decides the existing load-family diagnostic cannot be kept,
that is a product question for the operator, not a coder choice.

The extraction is faithful. `load_capture` now delegates its embedded open,
routing, capture and sealed hand-off to the new `acceptance::source_capture`. The
error mapping is one-to-one. The unchanged `load.t.cpp` cases and the load CLI
acceptance test still pass. The bound factory uses the existing native mechanisms
correctly: `add_root` with the original root receipt, an explicit first
`read` with the original declaration receipt, and bound `add_input` for the
registry. Native `read_file` then discovers through the root, requires
`same_selection` and verifies the actual opened leaf. No capture or load
interface file needed a change, and none changed.

The gaps are in what the factory requires and in what the tests and evidence
show. The architecture says the bound factory requires all three receipts from
the same original admission. The candidate accepts a registry receipt from an
unrelated admission, and it accepts a moved-from registry receipt (R2). Criterion
8 (advisory X06A4C01-N1) is not met. The missing-declaration test never asserts
the admission diagnostic. That diagnostic is not the existing load-family one
(R1). The red-first log cannot be tied to any code state, and the report does not
mention it (R3).

I reviewed in the shared checkout on `dev/planar-v2-vernacular`, HEAD
`bcfe1ad7ed94a85985f1189deba3b12cc851e7bd`, against the uncommitted candidate. I
wrote only this file and new files under
`tests/native/v5-02-x06-b2-review-evidence/`. I did not edit, revert or reformat
any product, test or author-evidence file. I did not commit, install, publish or
merge anything, and I used no old Planar tool, queue, skill, role or installed
binary. Builds and ctest in `build/rewrite-debug` ran one at a time. The probes
in R1 and R2 ran in a separate copy of the source tree, with its own configure
and build tree under the session scratch directory. The gated build tree was not
used for them.

## Exact candidate

| Identity | SHA-256 |
| --- | --- |
| `git diff bcfe1ad7` (2 tracked files), at review start and end | `5ccff2eb0b6773dcb61abc4daeff650032c6f44500ed75856c07738fea3b7dbf` |
| `src/engine/CMakeLists.txt` | `74721d41994951236a70388730e7e30dcca424559f382b6765c653f2fc1adfca` |
| `src/engine/acceptance/load.cpp` | `caf038172725875e88e26800733467aa26cc1e06bd1286e11579c41bc25412be` |
| `src/engine/acceptance/source_capture.cppm` (untracked) | `f965c1624fae01ea0b47c7bfa0687b98271b369f32ad77f9d1a596f0ae9146b4` |
| `src/engine/acceptance/source_capture.cpp` (untracked) | `8486186347efb6e9aad0d5955f2f36628d1aac8bba25e57403930bfd6b74e6aa` |
| `src/engine/acceptance/source_capture.t.cpp` (untracked) | `b8a608753b418b9cc0056407af0b6234c47e86d7175a7248d60b35b382a670d8` |
| `docs/vernacular/rounds/V5-02-X06-B2.md` (untracked) | `90f92f55da2f2c91507b5f366d30dab2b5e7ba39a729a488383b6ba8952a804e` |

These match the author's `raw/candidate-identity.txt`. `git status` shows no
other change. `load.cppm`, `load.t.cpp` and every `capture/*` file are
byte-identical to HEAD. The author's evidence directory is gitignored
(`.gitignore:37`), like every earlier evidence directory. See
[`candidate.txt`](../../../tests/native/v5-02-x06-b2-review-evidence/candidate.txt).

## Required change: X06B2-R1 — Keep and assert the existing load-family diagnostic for a missing declaration

Criterion 8 and advisory X06A4C01-N1 require two things. A missing selected
declaration must return the existing load-family diagnostic, and a test must
assert that diagnostic, not only when the refusal happens. X-CAP says the same:
"Missing supplied paths retain existing family capture errors". The architecture
matrix (row "Known factory vs opaque parse") puts this refusal at initial
admission, before registration.

The candidate test `source capture vanished declaration reports admitted change
beside embedded missing` (source_capture.t.cpp:325–350) does three things:

- It admits a missing `missing.lua` through `admit_file` and checks only
  `REQUIRE_FALSE(absent)`. It does not assert the diagnostic.
- It asserts the embedded open's diagnostic: `io` / `Cannot inspect confined
  input`. That is the existing load-family diagnostic.
- It asserts that a declaration removed after admission refuses with
  `capture_changed` / `Captured external profile changed`, and says in a comment
  that the two "differ by design".

My probe (`probe-cases.t.cpp.inc`, run log `probe-run.log`) asserts the admission
diagnostic. Admitting a missing declaration returns `io` / `Cannot open confined
input` with field `profile` (admission.cpp `io_error`, profile.cpp:131–134). The
existing load-family diagnostic is `io` / `Cannot inspect confined input` with no
profile field (native.cpp:284, rendered by `native_error` in
`src/cmd/planar/response.cpp`). The code matches, but the message and the field
location do not.

**Failure scenario.** The known-load factory in the next row will admit the
selected declaration before registration, as this row's tests model. Once that
factory is public, `planar load --workspace <missing>` run with a profile reports
a different message, and a `profile` field, than the same command run without a
profile. That contradicts X-CAP. Nothing in this round's tests would catch it,
because the only assertion on the admission path is that it fails.

**Required.** Assert the admission-time diagnostic for a missing selected
declaration. Then make the bound path produce the existing load-family
diagnostic: code, message and field. Put the adapter wherever the corrective
search shows it belongs. That may be a small known-load admission helper in
source_capture, or an adaptation the runtime row must carry. If it is the second,
record it as an explicit obligation of that row, with this row's test pinning
the raw admission diagnostic. A missing file and a file that vanishes after
admission may reasonably differ (`io` against `capture_changed`). If the
corrective round keeps that difference, it must be recorded against X-CAP rather
than settled by a test comment. If a contract reading is needed, return it to the
operator.

## Required change: X06B2-R2 — Require all three receipts from the same original admission, at the factory

The architecture's source_capture section (profile-preparation-architecture.md
lines 80–90) states: "Its bound factory requires all three from the same
original profile admission". `open_bound` (source_capture.cpp:143–181) enforces
this for the declaration only, through native `same_selection` against the
root's discovery. The registry receipt is stored unchecked
(`state->registry_receipt.emplace`, line 165) and bound only at `capture`
through `add_input` (line 235). Native bound `add_input` (native.cpp:598–627)
verifies the receipt against the opened leaf, and nothing else.

Probes in a scratch copy of the candidate, with only the probe cases appended to
the test file
([`probe-run.log`](../../../tests/native/v5-02-x06-b2-review-evidence/probe-run.log)):

- **Registry from a second admission.** Root and declaration come from profile
  admission `p1`, the registry from an independent `p2`. `open_bound` accepts,
  `capture` succeeds, `source.verify()` succeeds, and both `p1.verify()` and
  `p2.verify()` stay healthy (`R2 capture accepted=1 verify=1 p1=1 p2=1`).
- **Moved-from registry receipt.** `open_bound` accepts it
  (`R2 moved-from registry open_bound accepted=1`). The candidate's own test
  (lines 137–152) confirms the refusal comes only at `capture`.

**Failure scenario.** The registry is the one selected file whose trusted-alias
check depends on which admission it went through. The contract requires a
registry that aliases a trusted module to be refused. A registry admitted under a
second admission is checked only against that admission's profile closure. The
prepared capture then pairs the first admission's workspace with a registry the
first admission never vetted. Poisoning is also split: a refusal after
construction poisons the registry's admission (the `refuse_bound` lambda), not
the root and declaration admission, and the reverse. A moved-from registry lets
construction succeed even though the round requires refusal "without ... the
registry receipt". Spec and job content is then loaded and evaluated before the
missing authority is noticed.

**Required.** Make the bound factory refuse, before any declaration content read:

- a registry or declaration receipt that is not from the root's original
  admission;
- any moved-from or poisoned receipt.

Every such refusal must poison each supplied admission. If this needs a new
non-allocating "same admission" or "is live" predicate in `capture/admission`,
add it as a minimal forwarding change owned by exactly one package. The round
record already allows this "where search proves" it is needed. Add regression
tests for the registry from a second admission, a moved-from registry at the
factory, and a moved-from root. Show each one failing on the current candidate.

## Required change: X06B2-R3 — Make the red-first evidence verifiable and map the criteria

Criterion 10 requires regression tests to be shown failing before the fix, with
the evidence retained. `red-first.log` shows 3 of 9 cases failing (6
assertions). However:

- It records no command, no build directory and no code identity.
- Its line numbers (190, 118, 129, 145, 213, 231) are three lines off from the
  candidate test (193, 121, 132, 148, 216, 234), so it ran against an earlier
  test file.
- `REPORT.md` does not mention it.

No reviewer can tell which implementation state each failure shows, or whether
the same assertions still exist. The report also does not map the ten acceptance
criteria to tests. Several criteria are shown only partly (see the table below),
and the report should say so.

**Required.** For each red-first regression in the corrective round, record:

- the exact command;
- the pre-fix code identity, or the reversible mutation applied to the current
  candidate;
- the failing assertion and the test-file hash it ran against.

Add a criterion-by-criterion table to the report, with its limits. The new R1
and R2 regressions must be shown red this way.

## Criteria assessment

| # | Criterion | Finding |
| --- | --- | --- |
| 1 | Factory requires three original receipts | **Partial.** The type-level arity is proven by `static_assert` (t.cpp:105–108). A receipt from another admission is caught for the declaration only. A registry from another admission and a moved-from registry are accepted (R2). |
| 2 | Root and declaration metadata-admitted before registration, no content read | **Shown for this row's scope.** The `admission precedes registration` test watches the declaration inode: zero read attempts through admission and `p.verify()`, then reads only inside `open_bound`. No registration exists in this row, so the ordering itself must be proven by the runtime factory row. |
| 3 | Disjoint-directory module/declaration hard link refused before any external chunk | **Shown at admission.** `admit_file` refuses with `unsafe_path` and zero reads. This is existing X04 admission behavior. The test does not reach `source_capture`, and no chunk machinery exists in this row. |
| 4 | Replacement between registration and first read refused | **Met.** A rename-and-copy after admission refuses at `open_bound` with `capture_changed`, zero reads of the new inode, and poisons the admission. |
| 5 | same_selection plus actual-leaf verification, no receipt replacement | **Met.** A receipt for another admitted file or from another admission refuses with "Retained file authority cannot be replaced". The positive path seals and reverifies. |
| 6 | Discovery/seal/move/precommit/poison across real profile admission | **Mostly met.** Moves, move-assignment over an embedded owner, `verify()` and admission poisoning are shown. Post-seal mutation covers the declaration bytes, the registry bytes and the declaration mode, but no file discovered through the root (advisory N2). |
| 7 | Positive distinct-file registry colocation | **Met.** It binds as `input.json`, the content matches, and it is excluded from the workspace files. The trusted-alias registry is refused at admission. |
| 8 | Missing declaration returns and asserts the existing load-family diagnostic | **Not met** (R1). |
| 9 | Existing charges, terminal failure and profile transaction refusal unchanged | **Met by preservation.** No native, capture or ledger file changed, the unchanged load tests pass, and the terminal-after-capture test passes. |
| 10 | Red-first evidence retained | **Not verifiable** (R3). |

## Independent gate and inventory

All commands ran from the checkout root, serially, with unpiped exit codes.

| Command | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target planar_tests` | exit 0, no work. Built bytes are current. |
| `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -R "source capture\|replacement\.load\|load capture"` | exit 0, 27/27 ([`focused-ctest.log`](../../../tests/native/v5-02-x06-b2-review-evidence/focused-ctest.log)) |
| `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | exit 0, **937/937** passed, none failed, skipped or disabled, 353.97 s ([`full-ctest.log`](../../../tests/native/v5-02-x06-b2-review-evidence/full-ctest.log)) |
| Scratch copy plus probes: `planar_acceptance_source_capture_tests "[review]"` | 3/3 probes fail as the findings predict. The 9 candidate cases still pass in that build. |

The inventory checks out. Comparing the author's 937-name `inventory.txt` with
`v5-02-x06-b1-c01-evidence/inventory.txt` (928 names) gives zero removed names and
exactly the nine new `replacement.source capture ...` names
([`inventory-diff.txt`](../../../tests/native/v5-02-x06-b2-review-evidence/inventory-diff.txt)).
The author's raw logs agree. `2-ctest.log` reports 937 passed and no
Failed/Skipped/Not Run/Disabled lines. `3-lint.log` lists all three new source
files among 136 checked. The no-change build logs say "no work to do". I did not
rerun lint. The author's `1-build-tests.log` is itself a no-work build, so the
gate log does not show the compile. My build confirmed the products were
current.

## Advisory notes (non-blocking)

- **X06B2-N1.** `load.cpp` keeps its own copies of
  `at`/`kind`/`keys`/`same`/`same_field`/`parent`/`path`/`identity`/`routes`,
  and `source_capture.cpp` keeps a second set. The architecture says
  correspondence logic "is received" by source_capture. A shared internal helper
  would stop the two `routes` copies drifting before the runtime row reuses them.
  The new module also imports `planar.engine.canonical`, which is outside the
  architecture's "capture and native/value/host-path primitives only" list. That
  is harmless, because load already depended on it, but the architecture text
  should match.
- **X06B2-N2.** Add one post-seal mutation of a file discovered through the root
  (spec, job or reference), so that "retained discovery" is shown directly.
- **X06B2-N3.** `docs/architecture.md` describes the active module graph and does
  not yet list `acceptance.source_capture`. The round record does not own it. The
  coordinator should schedule the update.
- **X06B2-N4.** The `open_bound` comment "Refusals before any receipt is consumed
  poison the supplied originals directly" is accurate. However, after
  `add_root` consumes the root, a refusal poisons only through the registry
  receipt. That is correct only once R2 guarantees a shared admission.

## Limits

This review covers Debug on macOS only. I did not run RelWithDebInfo, Linux or
sanitizers, and I did not rerun lint. The probes ran in a scratch copy of the
candidate, with only test cases appended. They show behavior, not a fix.
Inventory preservation is measured against the untracked B1-C01 inventory file,
as in the author's report. No claim is made about X06 runtime preparation,
transactions, shared budgets or RSS.
