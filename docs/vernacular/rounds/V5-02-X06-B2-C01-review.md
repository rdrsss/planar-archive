# V5-02-X06-B2-C01 independent review (combined B2+C01)

Disposition: **request-changes**. One required change, **X06B2C01-R1**, blocks
approval. It is a documentation and evidence defect, not a product defect.

The code side of the correction is sound. The bound factory now refuses, before
any consumption or content read, a root, declaration or registry receipt that is
moved-from, poisoned or not from one original admission, and it poisons every
supplied receipt, so poisoning reaches the shared admission. The two new
`capture/admission` predicates are non-allocating, filesystem-free and touch no
selection, verification or native code. The missing-declaration diagnostic is
pinned raw at admission and adapted exactly to the existing load-family
diagnostic. The N2 post-seal mutation of a root-discovered file and the
registry/declaration hard-link refusal are tested. My independent gate passes
(see below).

What fails is the evidence the corrective brief made an explicit acceptance
criterion. `REPORT.md` contains no criterion-to-test map, no red-first account
and no X-CAP record for the removed-after-admission case. The round record says
it does, and a test comment points to it. The raw red-first files exist and are
mostly verifiable, but the report the brief requires was never written, or was
overwritten by the gate report.

I reviewed in the shared checkout on `dev/planar-v2-vernacular`, HEAD
`bcfe1ad7ed94a85985f1189deba3b12cc851e7bd`, against the uncommitted combined
candidate. I wrote only this file and new files under
`tests/native/v5-02-x06-b2-c01-evidence/review/`. I did not edit, revert or
reformat any product, test, round or author-evidence file. I did not commit,
install, publish or merge anything, and used no old Planar tool, queue, skill,
role or installed binary. Builds and ctest in `build/rewrite-debug` ran one at a
time, with no other build process running.

## Exact candidate

| Identity | SHA-256 |
| --- | --- |
| `git diff` (tracked files, 6 files), at review start and end | `5049380855a8c991b53622bfb95a966321b57988ef58ca7ec0c05a66429167ad` |
| `src/engine/CMakeLists.txt` | `74721d41994951236a70388730e7e30dcca424559f382b6765c653f2fc1adfca` |
| `src/engine/acceptance/load.cpp` | `caf038172725875e88e26800733467aa26cc1e06bd1286e11579c41bc25412be` |
| `src/engine/acceptance/source_capture.cppm` (untracked) | `58865a5a328b0d157cc9bd0f6621a0601487e10feea2b3126cd8f353b2e2791e` |
| `src/engine/acceptance/source_capture.cpp` (untracked) | `db0e6dfdfd7e6f03adfce25c87eaf15e71f8c5ddf15cc7de4dbadc0396545cc9` |
| `src/engine/acceptance/source_capture.t.cpp` (untracked) | `3924ecc0c0ff9aed37a858d7d8d78908dbba961c2accc0c59aedfb2a1934e182` |
| `src/engine/capture/admission.cppm` | `b289112661a17394d083f3b853c4f5bf4c96efac3bc0dc46ec7897acfb34cf2f` |
| `src/engine/capture/admission.cpp` | `7886d7407df2679a90b234da46b01e7f7f711fa6c3e3c418eb208b26c77bd109` |
| `src/engine/capture/admission.t.cpp` | `75ffa7bed2bebc3f1c1d8f1141cfb812971447509f03d55b08805e1ef52b9ef1` |
| `docs/vernacular/rounds/V5-02-X06-B2.md` (untracked) | `1c684c32b85b374cc1a3f7e6b4aa034001fc9fda04bfcf7cd4b9e9ead8dc83b3` |
| `tests/native/v5-02-x06-b2-c01-evidence/REPORT.md` (gitignored) | `cb1674887b68c7ef251289cd04717f10a4169d58fe3fae67cfb1805dc2f2ca4e` |

The source hashes match the author's `raw/candidate-identity.txt`. `load.cpp` and
`CMakeLists.txt` are byte-identical to the B2 review's table, so C01 did not touch
them. `git status` shows only the expected files plus the coordinator's
`roadmap.md` status edit and the B2 round/review records. See
[`candidate.txt`](../../../tests/native/v5-02-x06-b2-c01-evidence/review/candidate.txt).

## Required change: X06B2C01-R1 — Write the report the brief requires

The C01 brief (R3) requires that "REPORT.md maps all ten acceptance criteria to
tests, with limits", that each red-first regression records the exact command,
the pre-fix code identity and the failing assertion with matching line numbers,
and (R1) that keeping `capture_changed` for a declaration removed after
admission "is recorded against X-CAP in the report, not in a test comment".

`tests/native/v5-02-x06-b2-c01-evidence/REPORT.md` (2694 bytes, written at
21:35, after the coder's red-line file at 21:26) is a gate report only:
candidate identity, five gate commands, inventory and limits. It contains:

- no criterion-to-test table (no file in the evidence directory contains the
  word "criteri");
- no red-first section: it never mentions `red-snapshot-A`, `red-snapshot-B`,
  `B-red.log`, `A-red.log` or `red-line-verify.txt`, and does not say which
  regressions were red and which were guards;
- no X-CAP record for the removed-after-admission case.

Yet the round record states that REPORT.md holds "exact commands, snapshot
hashes, verified failing lines, and the criteria table", and
`source_capture.t.cpp:466` says the post-admission vanish is "recorded against
X-CAP in REPORT.md". Both references are dangling. A later reviewer or the
coordinator cannot rely on the record without reconstructing the evidence from
raw files, which is exactly what R3 of the B2 review asked to avoid.

The raw files themselves are good enough to build the report from (see the
evidence check below), with one gap: `A-red.log` and `A-green.log` start at
"Test project" and carry no command line, unlike `B-red.log`. The admission
predicate red run therefore has hashes and line numbers but no recorded command.

**Required.** Add to the C01 REPORT.md (keeping the gate section):

1. a red-first section that, for each regression, gives the command, the hashed
   snapshot, the failing lines, and whether it was red or a passing guard; include
   the exact command for the A (admission predicate) run;
2. a table mapping all ten B2 criteria plus the C01 additions (R1 diagnostic, R2
   construction refusal, N2, hard-link) to named tests, with limits;
3. the X-CAP record for the removed-after-admission `capture_changed` outcome
   (my reading is below; the author should state theirs);
4. a correction of line 147 in the A red log (see the evidence check), which was
   an assertion later inverted rather than a genuine red.

No product or test bytes need to change. If the round record or test comment is
reworded instead, that is also acceptable, as long as the brief's report content
exists somewhere the record points to.

## Check of each prior finding and advisory

### X06B2-R1 (missing declaration diagnostic) — closed in code

`source capture vanished declaration reports admitted change beside embedded missing`
(t.cpp:423–480):

- pins the raw admission diagnostic exactly: code `io`, message
  `Cannot open confined input`, field `profile` (439–441);
- pins the existing load-family diagnostic from the embedded open: `io`,
  `Cannot inspect confined input`, empty limit, zero maximum (428–432);
- asserts that the exported adapter `known_declaration_error` turns the first
  into the second, field by field (442–452), and passes other codes through
  unchanged (454–464).

The adapter (source_capture.cpp:103–107) drops the profile-level field because
it takes only `profile_error::detail`; load then maps `source_error_code::capture`
to `load_error_code::capture` with the detail moved unchanged (load.cpp:14–34),
so the field behaviour equals the embedded path. Nothing calls the adapter yet:
the obligation for the known-load runtime row is recorded in the round record
("The known-load row must apply `known_declaration_error`"). That is the option
the B2 review allowed. See advisory N1 below on its breadth.

On X-CAP: "Missing supplied paths retain existing family capture errors" governs
a path that is missing when it is supplied. A declaration that existed at
admission and vanished before the first read is a changed receipt, and X-CAP
says "a changed receipt refuses `capture_changed`". I therefore read the
`capture_changed` outcome (t.cpp:465–479) as conforming. The report must still
record that (R1 above).

### X06B2-R2 (single original admission) — closed

`open_bound` (source_capture.cpp:148–191) checks `live()` on all three receipts
and `same_admission` of root with declaration and with registry (160–163) before
`validate_paths`, before the root is consumed and before any read. Every refusal
at that stage poisons all three supplied receipts (154–159), so both admissions
are poisoned when they differ. After consumption, refusals poison through the
retained registry receipt, which construction has now proven shares the root's
admission; this also resolves advisory N4.

Tests:

| Case | Test (t.cpp lines) | Asserts |
| --- | --- | --- |
| Registry from a second admission | 156–177 | refused, `capture_changed`, zero declaration reads, `p1` and `p2` poisoned |
| Moved-from registry | 137–151 | refused, zero reads, admission poisoned |
| Moved-from root | 178–192 | refused, zero reads, admission poisoned |
| Poisoned registry / root | 193–218 | refused, zero reads, admission poisoned |
| Moved-from / poisoned declaration | 113–136 | refused, admission poisoned |
| Declaration from a second admission | 305–327 | refused, zero reads, both poisoned |

The predicates in `capture/admission` compare only the shared state pointer and
its `failed` flag (admission.cpp:484–501). The new admission test
(admission.t.cpp:86–162) covers same/different admissions, repeated receipts,
moved-from handles and poisoning by either the admission or a receipt, and states
that poisoning changes liveness and not identity. The predicates are documented
as non-allocating and filesystem-free; the class is thread-confined, so reading
the plain `failed` flag is safe.

### X06B2-R3 (red-first evidence) — raw evidence verifiable; report missing (R1 above)

I re-hashed every snapshot and diffed it against the current files:

- `red-snapshot-B` (hashes `db5feedb…`, `4f4c4290…`, `83aedf8f…`) equals the B2
  baseline `source_capture.cpp` plus a three-line stub adapter, with no R2 check.
  Its `.t.cpp` differs from the current test only by two lines of clang-format
  alignment (225–226). `B-red.log` records the build and ctest commands (ctest
  exit 8) and fails at t.cpp:146, 171, 323, 445 and 450; each line holds the same
  assertion in the current file. The moved-from root, poisoned receipts,
  hard-link and N2 cases passed at red; the round record says so.
- `red-snapshot-A` (`72520ca6…`) is HEAD `admission.cpp` with the six predicates
  stubbed to `true`. `A-red.log` fails 17 assertions in admission.t.cpp. One of
  them, line 147 `CHECK_FALSE(admission_root->same_admission(*admission_file))`,
  was inverted to `CHECK` in the final test (148–149), so it was a wrong
  expectation, not a red regression. The A logs lack a command line.

The moved-from root case could not be red: the B2 candidate already refused it at
`add_root` and poisoned the shared admission through the registry receipt. That
is acceptable as a guard, but the report should say so.

### X06B2-N2 — closed

`source capture retained flow survives moves and detects post-seal mutation`
case 3 (t.cpp:376–378) appends to `spec.lua`, which the test first proves is in
the sealed workspace closure (352–356); `verify()` then refuses `capture_changed`
and the admission is poisoned.

### Hard-link question — resolved

`source capture refuses a registry hard-linked to the declaration as path_denied`
(t.cpp:221–238) hard-links `workspace/registry.json` to `workspace.lua`. Admission
accepts both distinct paths; `open_bound` succeeds; `capture` refuses with
`path_denied` / `Hard-link input alias` from native `add_input`, nothing is
sealed and the admission is poisoned. The brief accepted refusal at admission or
at capture. The registry is JSON input, not a chunk, so refusal at capture
precedes any use of its bytes. Admission itself does not refuse aliases between
two untrusted supplied files; the existing admission hard-link case covers trusted
modules only. That is a limit worth stating in the report, not a defect against
the cited contracts.

### N1 and N3 — recorded

N1 (duplicate route/correspondence helpers in `load.cpp` and `source_capture.cpp`,
and the `planar.engine.canonical` import) is recorded and not fixed. N3
(`docs/architecture.md`) remains the coordinator's, after approval.

## Criteria assessment (combined B2+C01)

| # | Criterion | Finding |
| --- | --- | --- |
| 1 | Factory requires three original receipts | **Met.** Arity by `static_assert` (t.cpp:105–108); moved-from, poisoned and foreign receipts refused at construction (R2 table). |
| 2 | Root and declaration admitted before registration, no content read | **Met for this row's scope.** Zero read attempts through admission and `p.verify()` (240–252). Ordering against real registration belongs to the runtime row. |
| 3 | Disjoint-directory module/declaration hard link refused before any chunk | **Met at admission** (254–267), existing X04 behaviour; no chunk machinery in this row. |
| 4 | Replacement between registration and first read refused | **Met** (269–283). |
| 5 | same_selection plus actual-leaf verification, no replacement | **Met** (285–340). |
| 6 | Discovery/seal/move/precommit/poison retained | **Met**, now including a root-discovered file (342–386). |
| 7 | Positive distinct-file registry colocation | **Met** (388–408); trusted-alias registry refused (409–420). |
| 8 | Missing declaration returns and asserts the load-family diagnostic | **Met** as pinned raw diagnostic plus asserted adapter; wiring is a recorded runtime-row obligation. |
| 9 | Charges, terminal failure and transaction refusal unchanged | **Met.** No native, ledger or charging code changed; the admission additions are const predicates; unchanged load tests and the terminal test pass. |
| 10 | Red-first evidence retained | **Raw evidence met; report not met** (X06B2C01-R1). |

## Independent gate and inventory

All commands ran from the checkout root, serially, with unpiped exit codes
recorded in `review/*.exit`.

| Command | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target planar_tests` | exit 0, `ninja: no work to do` (products current with the hashed sources) |
| `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | exit 0, **940/940** passed, no Failed/Not Run/Disabled/Skipped lines, 332.00 s ([`2-ctest.log`](../../../tests/native/v5-02-x06-b2-c01-evidence/review/2-ctest.log)) |
| `ctest --test-dir build/rewrite-debug -N` | exit 0, 940 registered names ([`inventory.txt`](../../../tests/native/v5-02-x06-b2-c01-evidence/review/inventory.txt)) |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, "Formatting and Doxygen checked 136 active source files", including all seven candidate C++ files; no Doxygen retry in this run |
| `cmake --build build/rewrite-debug`, twice | exit 0 both times, `ninja: no work to do` |

Inventory: my 940-name list is byte-identical to the author's. Against the B2
candidate's 937 names it removes none and adds exactly three
(`capture admission receipts report same admission and liveness`,
`source capture refuses a registry hard-linked to the declaration as path_denied`,
`source capture refuses receipts outside one original admission at construction`).
Against the committed B1-C01 inventory (928) it removes none and adds twelve. All
twelve source-capture/admission-predicate cases passed in the full run. The
`git diff` and every candidate file hash were unchanged at the end of the gate.

The first `planar_tests` build did no work, so this gate does not itself show the
compile; the author's `raw/1-build-tests.log` shows the 129-step build.

## Advisory notes (non-blocking)

- **X06B2C01-N1 — The adapter keys on a message, not on absence.** Admission
  emits `io` / `Cannot open confined input` for every non-ELOOP/ENOTDIR failure
  of `fstatat`, `openat` or `fstat` (admission.cpp:49–51, 179–196). On macOS the
  metadata open uses `O_EVTONLY`, which fails with EACCES on a mode-000 file (I
  confirmed the syscall behaviour with a scratch probe,
  `review/o-evtonly-probe.txt`). So on macOS an unreadable selected declaration
  would also be rewritten to `Cannot inspect confined input`, while the embedded
  path reports `Cannot open confined input` for the same file (native.cpp:61–63).
  On Linux `O_PATH` succeeds and the later read reports the existing message. This
  is reasoned from source plus the syscall probe, not run through the product. The
  cppm comment presents the pair as meaning "missing". The runtime row should
  either distinguish absence at admission or document the macOS difference.
- **X06B2C01-N2 — No role check on supplied receipts.** The factory does not check
  that the declaration receipt was admitted with the declaration role or the
  registry receipt with the registry role. Existing mechanisms limit the effect
  (the declaration must be discovered through the admitted root, and the
  declaration role is the stricter one), so this is not a defect, but the runtime
  row may want an explicit role predicate.
- **X06B2C01-N3** — The construction refusal reports `capture_changed` /
  "Captured external profile changed" for moved-from, poisoned and foreign
  receipts alike. That is consistent with the existing native refusals of a
  poisoned receipt, but a later diagnostic reviewer may prefer `invalid_state` for
  a moved-from handle, which is a caller bug rather than a changed capture.

## Limits

Debug on macOS only. I did not run RelWithDebInfo, Linux or sanitizers. I did not
build a probe tree; the R2 and R1 behaviour is shown by the candidate's own tests,
whose red states I verified from the retained snapshots and logs rather than by
re-running them. Advisory N1 is reasoned from source plus a syscall probe.
Inventory preservation is measured against the untracked B2 and B1-C01 inventory
files. No claim is made about X06 runtime preparation, known-load admission
timing, transactions, shared budgets or RSS.
