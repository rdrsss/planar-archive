# V5-02-X06-C-C04 gate report

Verdict: PASSED (passed=true). Gate run 2026-10-26 by the gate runner, which edited no source file. This replaces the earlier
report. Logs of this run are raw/build-tests.log and raw/gr2 to raw/gr7 (exit codes in the matching .exit files, or in the
log name for command 1). Earlier logs (raw/1-* to 7-*, build1, g2 to g7) describe earlier tree states and are kept unmodified.

## Candidate identity

HEAD caaf0396ae90a7f7d2d511b9a9a6e185e4362d52, working tree uncommitted.
- sha256 of `git diff HEAD` (tracked only): 618c6ceb5d5ec38e298f78e1a4d61bbce52b245ea4ec4d8d95ace26ab9ebe494
- sha256 of `git diff HEAD -- src` (tracked, src only): 179c4a8a292384e3fbf4affe4aa3410a0bf76ea850a641d3fac8ff142e81f8ba
- sha256 of `git diff HEAD` followed by "path sha256" lines for every untracked non-ignored file outside this evidence folder
  (git ls-files -o --exclude-standard, sorted): ee133746ced37e65dab42e33fb1baef8347c31821d53ce770082d1220c58ff1f
- Per-file hash of src/engine/ledger/acceptance.t.cpp: 0188afa22c942c1f78553bffc35aa91e5a7fe841c42ac960a7cb86788d2e4ca0.

IMPORTANT: this identity differs from the one in the prober's PROBES.md and the earlier gate report (diff HEAD 9af1cc57...,
src 20260273...). Comparing the per-file hashes in PROBES.md, the only candidate file that changed is
src/engine/ledger/acceptance.t.cpp (probe-time hash abf22e8d..., now 0188afa2..., modified 15:39, after the probes and the red
check; the other 11 candidate files match the probe hashes byte for byte). Test line numbers in acceptance.t.cpp moved by about
3 lines versus the earlier report. The probes (PROBES.md) and red logs were taken on the earlier acceptance.t.cpp; they are not
re-run on this exact state. This gate ran fully on the current tree.

## Commands

| # | command | exit | result |
|---|---------|------|--------|
| 1 | cmake --build build/rewrite-debug --target planar_tests | 0 | "ninja: no work to do." (already built from current files; raw/build-tests.log) |
| 2 | ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure | 0 | 100% tests passed, 0 failed out of 1041; 0 skipped, 0 not run (raw/gr2-ctest.log) |
| 3 | ctest --test-dir build/rewrite-debug -N | 0 | Total Tests: 1041 (raw/gr3-ctest-N.log) |
| 4 | python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug | 0 | "Formatting and Doxygen checked 142 active source files", no findings (raw/gr4-lint.log) |
| 5 | cmake --build build/rewrite-debug | 0 | "ninja: no work to do." (raw/gr5-build.log) |
| 6 | cmake --build build/rewrite-debug (second run) | 0 | "ninja: no work to do." zero steps, passes (raw/gr6-build.log) |
| 7 | python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME" <22 files> | 0 | clean on the 7 modified files and 15 untracked .cpp/.cppm/.py/.md files from git status (raw/gr7-redact.log, empty). The script exits 2 (usage) without --repo-path, --home; a run without file arguments was not performed. The evidence folder's own logs are not covered. |

ctest label summary: unit 1034 tests, acceptance 7 tests. Matched 1041, passed 1041, skipped 0. #531 (X06 B3 C a file replaced at
any allocation inside prepare_load is never published as verified) passed in 16.11 s (budget about 15.3 to 16.5 s of 30 s). Total
ctest time 382 s.

Limitation on the build checks: the tree was already fully built when the gate began, so commands 1 and 5 also reported no work;
there was no build step that did work in this run. Command 6 follows the successful build of command 5 and reports zero steps.

## Inventory

Compared against tests/native/v5-02-x06-c-c03-evidence/inventory.txt (1038 names, the most recent committed list; it holds plain
names without the `replacement.` prefix excluded as shown, and the new list uses the same format). The current list is saved as
inventory.txt next to this report (1041 names, sorted). Removed: none, all 1038 preserved (set difference, empty). New (3):
- replacement.X06 C C04 acceptance_result stays outside the internal seam namespace
- replacement.X06 C C04 direct seam tests live in their own implementation unit and the adapter tests import normally
- replacement.X06 C C04 seam entities live in a nested non-exported internal namespace

The two direct-seam tests kept their exact names and moved to src/engine/ledger/profile_acceptance_seam.t.cpp (C03-R6 split):
"X06 C seam interns an effective stdlib closure beyond the original tree limits" (line 203) and "X06 C C01 seam interns the registry
tree with the provenance limits" (line 265). The full registered inventory is inventory.txt (ctest -N order is in
raw/gr3-ctest-N.log).

## Acceptance criteria to tests

Names are ctest names without `replacement.`. Lines are in src/engine/ledger/acceptance.t.cpp (current file) unless stated.

| criterion | test or evidence | result |
|---|---|---|
| 1 seam in nested namespace, no hooks | "X06 C C04 seam entities live in a nested non-exported internal namespace" (acceptance.t.cpp:850); "X06 C C04 acceptance_result stays outside the internal seam namespace" (:875); "X06 C C03 no passkey grant or test hook vocabulary remains under src" (:915); "X06 C C01 seam is unreachable from a plain acceptance importer" (:770) | pass |
| 2 negative-compile probes r7, r8, c1, c2, C03 probes | prober evidence in probes/PROBES.md and probes/raw/negative-compile (not Catch2; on the earlier acceptance.t.cpp, product files identical); not re-run by this gate | not evidenced by this gate |
| 3 direct-seam tests in a separate unit | "X06 C C04 direct seam tests live in their own implementation unit and the adapter tests import normally" (:888); "X06 C seam interns an effective stdlib closure beyond the original tree limits" (profile_acceptance_seam.t.cpp:203); "X06 C C01 seam interns the registry tree with the provenance limits" (profile_acceptance_seam.t.cpp:265) | pass |
| 4 source scan second check | "X06 C C03 only the adapters call complete_native_candidate" (:952) | pass |
| 5 inventory | section above (1038 preserved, 3 new) | met |
| 6 invariant rows | prober deliverable (probes/PROBES.md: 131 rows, 91 killed, 27 build-failed, 13 survived); not run by this gate | not evidenced by this gate |
| 7 dependency audit | "X06 C C03 runtime product sources import only ledger completion from the ledger" (:1050) passes; every target builds; two builds with no work. The link-graph consumer list is a record item | tests pass; record not verified here |
| 8 diagnostics and #531 | "X06 C C03 shared body refusal message bytes are pinned" (:985); "X06 C C03 workspace mismatch message bytes are pinned" (:1038); "atomic workspace mismatch refuses before record writes" (:224); #531 16.11 s | met |
| 9 records and identity hashes | coordinator work | not verified here |
| 10 red-first | section below | partly met (see below) |

Corrective findings: C03-R1 closed by the first two C04 tests above plus probe r7 (prober) and the C01 unreachability test.
C03-R2 (identity wording, red snapshot incl. untracked files), R3, R4, R5 are record corrections with no tests; R6 is closed by the
split above. Not verified by this gate.

## Red-first

Source: red/RED.md. Candidate HEAD caaf0396; hash named there: sha256(`git diff HEAD -- src`) =
fe6980a9f0200ece75b5738ac867cde0628d4b01d0922c5ca55fe9126c1ca1d7. That pathspec is src, tracked only, so it excludes the untracked
candidate files, contrary to criterion 10; no saved diff or untracked bytes or per-file hashes were found in red/ (only RED.md
and raw/build.log, t1.log to t16.log). This is missing evidence, stated rather than invented. Command per test (from RED.md):
`ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`, logs red/raw/tN.log.

| test | file:line (current) | real red or guard | failing line in the red run |
|---|---|---|---|
| C04 seam entities live in a nested non-exported internal namespace | acceptance.t.cpp:850 | real red (failed-assertion) | 852 |
| C04 acceptance_result stays outside the internal seam namespace | acceptance.t.cpp:875 | real red | 875 (RED.md: case 872) |
| C01 seam is unreachable from a plain acceptance importer | acceptance.t.cpp:770 | real red | 808 |
| C03 only the adapters call complete_native_candidate | acceptance.t.cpp:952 | real red | 968 |
| C04 direct seam tests live in own impl unit, adapter tests import normally | acceptance.t.cpp:888 | real red | 58 (RED.md: case 886) |
| C03 no passkey grant or test hook vocabulary remains under src | acceptance.t.cpp:915 | guard, already passed | none |
| I1, I2, I7 to I14 guards (RED.md rows 7 to 16) | not itemised in RED.md | guards, already passed | none |

Failing-line numbers are those recorded in RED.md for the older acceptance.t.cpp (the file has since shifted by about 3 lines).
The negative-compile probe r7 and probes c1, c2, r1 to r8 are prober items, not registered tests, and have no red log. All red
tests are green in this gate run. Red-first for the two seam tests moved to profile_acceptance_seam.t.cpp is not itemised in RED.md.

## Deliberate behaviour differences

None found in the evidence; diagnostics are pinned by the tests under criterion 8 and all pass.

## Limitations

The gate covers build, test, inventory, lint, redact check and zero-work only. Criteria 2, 6, 7 (record), 9 and the
untracked-file red snapshot (criterion 10) are not evidenced by this gate. Acceptance.t.cpp changed after the probes and the
red check (see identity), so those two bodies of evidence are not on the exact gated state. Prober survivors (13, including the
accepted C05, C19, X12, X18, X20, I5b, E6, plus the scan gap for `seam_detail` and unpinned D11) are in PROBES.md, not judged here.
