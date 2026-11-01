# V5-02-RA01-B-W2-C02 gate report (second gate run)

Gate runner, 2026-10-31 to 2026-11-01. No source file was edited by this stage. Raw logs of this run are under `raw/gate2/`.
The first gate run's logs (`raw/01-*.log` to `raw/14-*.log`, `raw/linux/`) and its report (`raw/gate1-REPORT.md`) are kept, none deleted.
This run repeated every command after the prober and probe repair stage. Source and test bytes did not change between the two runs (identical identity hashes below).

## Result

passed = true. Lint, both macOS builds, both macOS ctest runs (1437 of 1437, 0 failed, 0 skipped), four no-work builds (zero steps),
the Linux container gate (Debug and RelWithDebInfo, 1437 of 1437 each, `x07.two_process_cas_one_winner` Passed in both), and the redaction
check on 178 explicit committable files all met their criteria. One redaction finding remains, unchanged from the first run: the coder logs
`red/guard1.log` and `red/guard2.log` contain local paths and must not be staged.

## Candidate identity

HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`. identity.py output is verbatim in `raw/gate2/identity-src.txt`, `identity-tests.txt`, `identity-docs.txt` and `identity-pin.txt`.

- sha256 of plain `git diff` (tracked, whole tree): `92b501adce5e2bf544840a311539b262e476676a001e2d913a1de261c94c2603`
- pathspec `src`: tracked diff `7a206bc2cc84ef0bb69602bc83f11761a49932e553e90f2d67153208cee291aa`, combined with untracked bytes `68097c45b99b1ea681a856099c0d9f9315134e37bceeb47c8b3946cc7d338151` (same as run 1)
- pathspec `tests/native` excluding evidence: `15899df20287472033b359e593096e723403b3c93716162626c6a5eb07684c70` (same as run 1)
- pathspec `docs/vernacular/authoring-contract.md docs/vernacular/rounds`: tracked `e8f2e9c9a5b9310bdf1e80d0fd2fd3458ecd4191d18b86b5ff93fdb952a0fc55`, combined `76fffcbe5c034a62e8a9e9b8a60896375a9f85433205ead2d1b030884f4ce63d` (run 1 recorded `854bdaa8...`; the round record `V5-02-RA01-B-W2-C02.md` was edited after run 1, a record-only change)
- boundary pin files (identity-pin.txt): `runtime.cpp` `9c8c15cf...a71d`, `commands.lua` `f32c5be4...10b7`, `schema.lua` `e71fa6d9...6fe9`, `semantic.lua` `82bfe3dc...89aa`; all four hashes appear in `tests/native/v5-02-ra01-b-w2-evidence/c02-pin.md` (12 matching hash-prefix lines).

## Commands

| # | command | exit | result | log |
|---|---|---|---|---|
| 1 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format --Werror and Doxygen: "checked 171 active source files" | raw/gate2/01-lint.log |
| 2 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | success (`ninja: no work to do.`, the tree was already built at these bytes by run 1) | 02-debug-build.log |
| 3 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j4` | 0 | `100% tests passed out of 1437`, 0 failed, 0 skipped (acceptance 53, unit 1384; 161.90 s real) | 03-debug-ctest.log |
| 4 | `cmake --build build/rewrite-release --target planar_tests` | 0 | success | 04-release-build.log |
| 5 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure -j4` | 0 | `100% tests passed out of 1437`, 0 failed, 0 skipped (69.80 s) | 05-release-ctest.log |
| 6a, 6b | `cmake --build build/rewrite-debug --target planar_tests`, twice | 0, 0 | both `ninja: no work to do.` | 06-debug-nowork1/2.log |
| 7a, 7b | same for `build/rewrite-release` | 0, 0 | both `ninja: no work to do.` | 07-release-nowork1/2.log |
| 8 | `scripts/linux-gate.sh <scratch dir outside checkout>`; df before 142606208 blocks of 512 B (68 GiB) free, above 15 GiB; no `-t`; script ran `docker builder prune -f` (exit 0); df after 57 GiB free | 0 | docker-build.exit 0; container configure, build, inventory exits all 0; both container ctest logs `100% tests passed out of 1437`; `x07.two_process_cas_one_winner` Passed (0.99 s Debug, 0.26 s RelWithDebInfo); container no-work builds print `ninja: no work to do.`; manifest-compare: host before/after equal, host vs container context equal | 10-linux-gate.stdout, linux/ |
| 9 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <178 explicit files>` (`raw/gate2/redact-files-clean.txt`: every modified or untracked file of the earlier list plus every non-raw, non-snapshot, non-base file in the C02 evidence folder, including this REPORT.md written before the final check; guard logs excluded) | 0 | no output | 11-redact-clean.log |
| 9b | same plus `red/guard1.log` and `red/guard2.log` | 1 | `local path in` both files; do not stage them | 11c-redact-with-guards.log |
| 9c | `python3 scripts/redact-local-paths.py check` with no arguments | 2 | usage error (requires `--repo-path`, `--home`, files); the explicit-file runs are the real check | 11b-redact-nofiles.log |

Skips: the word "skipped" appears in the ctest logs only inside the test name `replacement.command load refuses forged family order skipped duplicate and malformed arguments`, which Passed. No test was Skipped or Not Run, in any of the four ctest runs.

## Inventory

`inventory.txt` (next to this report) is the sorted name list from `ctest --test-dir build/rewrite-debug -N`, 1437 names, regenerated in this run (`raw/gate2/08-inventory-debug.txt`).
Compared with the most recent earlier inventory, `tests/native/v5-02-ra01-b-w2-c01-evidence/inventory.txt` (1437 names): identical. New names: none. Removed names: none.
The Debug and RelWithDebInfo macOS inventories are identical (`raw/gate2/inv-debug.sorted`, `inv-release.sorted`, diff empty), and the Linux container Debug and RelWithDebInfo sorted inventories are identical to each other (diff empty).
The evidence folders are gitignored, so the C01 inventory is a prior evidence file, not a git-committed one.

## Boundary pin (acceptance 6)

The four pinned files are byte-identical to the c02-pin.md values (hashes above). `replacement.X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`
(src/engine/runtime/external_registration.t.cpp:4589) Passed on macOS Debug, RelWithDebInfo and both Linux configs. This test has a window of 8 padding bytes, so the per-file hash comparison is the authoritative pin (see the round record limits).

## Acceptance criteria to tests (round record, criteria 1 to 6, and findings R1 to R6)

| criterion / finding | evidence | test names (file:line) |
|---|---|---|
| 1, R1: both digest helpers map `Canonical allocation failed` as well as `Hash allocation failed`; guards pin it; no new message text | source guards (no allocator fault seam exists or may be added) | `RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call` (src/engine/capture/capture.t.cpp:1064, SECTION `file_digest allocation site`, assertions about :1097 to :1100); `RA01-B2 C01 observe digest allocation failure is a bound refusal` (src/cmd/planar/authoring_observe.t.cpp:770, assertions about :785 to :789). `diff -U0` against `base/` (`raw/gate2/12-diff-vs-base.txt`): only the two `digest_failure` / `observation_digest_failure` conditions (each gains `|| error.message == "Canonical allocation failed"`, keeping `"Hash allocation failed"`) and the capture.cppm `@note` changed; no other message literal changed. |
| 1 (existing behaviour kept) | guards that already passed | `RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed` (capture.t.cpp:1038) |
| 2, R2: directory.t.cpp comment (E02c), C01-46 row, E02 and E02b limits | comment at directory.t.cpp:342-344 describes `list()`'s `openat(".")` failure on a readable but unsearchable directory; test name unchanged. E02 and E02b recorded as limits (round record Concerns). Record rows and the E02c probe re-run are for the reviewer and `probes/PROBES.md` | `RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect` (src/engine/capture/directory.t.cpp:341), Passed in all four runs |
| 3, R3: four decision entries, item 7, DECISIONS sync | record-only. `authoring-contract.md` and `V5-02-RA01-B-W2-C01.md` differ from `base/` (`raw/gate2/13-changed-vs-base.txt`); not otherwise verified by this stage | none |
| 4, R4 to R6 | R4 is the `c02-pin.md` text (changed vs `base/`); R5 is `red/RED.md` text; R6: `capture.cppm` `@note` changed (diff above); `authoring_prompts.t.cpp` "secant search" and `authoring_orchestrator.t.cpp` package I-B wording are byte-equal to `base/` (made before the base snapshot, see limits). Carried concerns 2 and 8 restated in the round record | no test; comments and records only |
| 5: 1437 names kept, identical across configs, new names none | inventory comparison above | all names, `inventory.txt` |
| 6: boundary pin | per-file hash comparison above | `replacement.X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` (external_registration.t.cpp:4589) |

## Red-first (from `red/RED.md`, the RF stage; this gate did not re-run them)

RED.md: HEAD `3343ad38...`; `candidate.diff` sha256 `c2a9be9914d91d735087a4683930319d5daeeadc588e0fa64ce8f2f8ea04ea4a`; `SHA256SUMS` sha256 `b48d790bfdc579e84c64cdd84eaaa6976a93b81ad777a772ea5b5cd97f8fb7c8` (snapshot in `red/snapshot/`);
build `cmake --build build/rewrite-debug --target planar_tests` exit 0 (`red/raw/build.log`); per-test command
`ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure` (`red/raw/testN.log`). `red/identity.txt` holds the identity.py output at red time (src combined `306e21bc01e349df81aa06e6f85a785ce8ba2b036f41091d9b3aceb96a18bd8c`).

| # | test | outcome | failing line | real red or guard |
|---|---|---|---|---|
| 1 | capture_diagnostic post-gate failures are remapped at every native call (capture.t.cpp:1064) | failed on assertions | capture.t.cpp:1097 and :1100 (`file_digest allocation site`) | real red (C02-01, C02-03); the rest of the test is guard |
| 2 | observe digest allocation failure is a bound refusal (authoring_observe.t.cpp:770) | failed on assertions | authoring_observe.t.cpp:785 and :789 | real red (C02-02, C02-04, C02-21); C02-36 guard |
| 3 | capture_diagnostic maps a post-gate add_input failure to file changed (capture.t.cpp:1038) | passed | none | guard that already passed |
| 4 | observe maps a failed entry inspection after listing to cannot inspect (directory.t.cpp:341) | passed | none | guard that already passed |
| 5 | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge (external_registration.t.cpp:4589) | passed | none | guard that already passed |

Missing from the evidence: C01-46 / E02c has no red-first record (recorded as a limit in the plan). `red/guard1.log` and `red/guard2.log` are coder logs whose headers hold ctest invocations; their exit files read 8 (not interpreted here, the tests they ran are not among the five above). The red snapshot is identified by `candidate.diff` and `SHA256SUMS` hashes (R5 limit), with identity.py output in `red/identity.txt`.

## Deliberate behaviour differences

Only the one the round requires: `capture_diagnostic` and the observe digest helper also map a `Canonical allocation failed` `resource_limit` (limit `canonical_bytes`) to `diagnostic_bytes` and `observation_total_bytes`, same as `Hash allocation failed`. Permitted by Q-C01-2 option A as applied in the round record R1. No message text changed.

## Limits

- `base/BASE.md` named by the plan does not exist. `base/` holds test files byte-identical to the current tree (taken after the T1 test edits), so `diff -U0` against it shows only product-file and record edits, not the test edits.
- The no-work build for Debug step 2 printed no work because run 1 had already built these bytes; the zero-work checks 6 and 7 were each run twice after a successful build step.
- This stage did not re-verify record-only items (R3, R4, R5 text, round-record edits, probe carry-forward). Probe results are in `probes/PROBES.md`.
- The Linux gate covers linux/arm64 only. ctest durations and `-j4` ordering vary; counts are the criterion.
- Allocator-fault paths in R1 are guarded by source guards only; E02, E02b, C02-15a, C02-15b and C01-37r survive as stated limits (round record).
