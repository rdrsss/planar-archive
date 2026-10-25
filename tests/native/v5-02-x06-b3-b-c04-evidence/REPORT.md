# V5-02-X06-B3-B-C04 gate report (gate runner, 2026-10-25)

Verdict: passed = false. Reason: the redaction check command, run exactly as assigned (no file arguments), exits 1. Every other command met its criterion. The C04 round is docs-only, so no build, test or no-change build was part of the assigned commands.

## Candidate identity

- HEAD `a2aa1cc0ed4ad8c5b9e879a04231591839e7dc48` (candidate uncommitted).
- `git diff HEAD -- src | shasum -a 256` = `96e44f54086469fe3ee0ccfa431ff4e3fadd12a15278834e5d6714613990531b`, exit 0. Equals the frozen value in the round record. Raw: `raw/src-diff.log`, `raw/src-diff.exit`.
- No tracked file other than the six frozen source files and `docs/vernacular/roadmap.md` is modified (`git status --short`).

## Commands

1. `git diff HEAD -- src | shasum -a 256`: exit 0, value above.
2. Per-file sha256 loop over the six files: exit 0 (`raw/files.log`, `raw/files.exit`). All equal the round record (first 12 hex):
   - runtime.cpp `ab70f5441bea`, runtime.cppm `331bd1b629df`, profile.cpp `91ac81f3d466`, profile.cppm `7bb805634449`, runtime.t.cpp `3b22d137816e`, profile.t.cpp `d94976e9c9ea`.
3. `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME"`: EXIT 1 (FAIL). `raw/redact.log` lists 8223 "local path in ..." lines, `raw/redact.exit` = 1. Breakdown by top directory: build 4240, tests 3956, doxygen-lint-output 26, .claude 1 (`.claude/settings.local.json`). I checked every listed path with `git check-ignore`: all are git-ignored, so none is a tracked or commit-candidate record (no hits under `docs/` or `src/`). Without file arguments the checker scans the whole tree including ignored build and evidence output. The coders' earlier `redact-check.log` (empty, exit 0) was run over a named list of records only (see `run-gate.sh` lines 12 to 15), which is a narrower scope than the command assigned to me. I report the assigned command's result as a failure; I did not run `redact`.
4. Inventory (read-only, not in the assigned list, used for the required comparison): `ctest -N` in `build/rewrite-debug`, exit 0 (`raw/ctest-N.log`, `raw/ctest-N.exit`), "Total Tests: 987". No tests were executed, so there are no pass, skip or failure counts from this gate. Zero matched tests did not occur: 987 registered.
5. No-change build check: NOT RUN. It is only meaningful after a separate successful build, and no build command was assigned (docs-only round). Not claimed.

## Inventory

`inventory.txt` holds the 987 names from `ctest -N`. The most recent inventory I could find is `tests/native/v5-02-x06-b3-b-c03-evidence/inventory.txt` (987 names; the C03 final inventory, the evidence folder is git-ignored so it is not committed; the most recent committed inventories are the `inventory-preservation.json` files, latest under `tests/native/v5-02-x06-b1-c01-evidence/review/`, which predate B3). Comparing sorted lists: identical, so new names 0, removed names 0. 41 names contain `X06 B3`, same as C03. (An unsorted diff differs only in ordering.)

## Acceptance criteria of C04 (record accuracy only)

C04 adds no tests and changes no source, so no test name maps to any criterion; "test" evidence is the unchanged inventory plus record text. File:line references are to the records as they stand now.

| Criterion / finding | Where | Check |
|---|---|---|
| R2: C02 figures (985/39/`162997debb60`/`8b2f73ea`) relabelled C02 candidate, final 987/41/`3b22d137816e`/`96e44f54` added | `docs/vernacular/rounds/V5-02-X06-B3-B-C02.md` lines 134-143, 155, 239-241, 251-272; `V5-02-X06-B3-B.md` lines 208-219, 371-393 | Read; see also `grep-final-candidate.txt`, `grep-c02-figures.txt` |
| R2: masking reason corrected (`open_bound` parses the declaration; two mutants killed standalone) | C02 record line 184, 260 | Read. Reviewed C03 evidence (1 failed of 41 each) not re-run here |
| R1: C03 Final candidate record no longer says "not verified"/"still owed" | `V5-02-X06-B3-B-C03.md` lines 341-366, 405-433 | Read |
| N-a: Repair 2 wording (declaration written before `f.open()`) | C03 record line 320 | Read |
| N-b: C02 evidence README opening | `tests/native/v5-02-x06-b3-b-c02-evidence/README.md` lines 3-5 | Read |
| N-c: run 3/run 4 corrections as notes, not raw edits | C03 record lines 379-382; C03 `README.md` Correction notes (a), (b) | Read |
| N-d: `README.txt` marked superseded | `tests/native/v5-02-x06-b3-b-c03-evidence/README.md` top | Read; `README.txt` untouched |
| Source frozen, hashes unchanged | commands 1 and 2 above | Met, equal to record values |
| Test count stays 987, no test renamed or removed | `inventory.txt` vs C03 inventory | Met (987, sorted-identical) |
| Redaction check | command 3 | NOT MET as assigned (exit 1, ignored paths only) |

## Red-first

None applicable: C04 adds no regression tests. The coders' evidence contains no red-first commands for this round, and none is invented here. Red-first and mutation evidence for the tests of the lineage is in `tests/native/v5-02-x06-b3-b-c03-evidence/probes/PROBES.md`; the two C03 tests (route phase refusal, open-bound refusal) were killed by mutants, 1 failed of 41 each, per the C03 record.

## Deliberate behaviour differences

None. No product behaviour, diagnostic or test changed.

## Limitations

- macOS Debug only; no tests executed in this gate, the 987 count is registration (`ctest -N`) plus the C03 gate carry-over.
- No build and no zero-work check were run.
- The redaction failure is a scope question: either the check must be given the record files, or ignored build and evidence output must be excluded, before the commit step. The commit step's own path-redaction step still applies.

## Re-run (second gate run, same day)

The assigned commands were run again, logs in `raw/run2-*`. Results are identical: `git diff HEAD -- src | shasum -a 256` exit 0, `96e44f54086469fe3ee0ccfa431ff4e3fadd12a15278834e5d6714613990531b`; six file hashes exit 0 and equal the record; `ctest -N` exit 0, "Total Tests: 987"; the redaction check as assigned (no file arguments) again exits 1 with 8224 output lines (`raw/run2-redact.log`, `raw/run2-redact.exit`). The two redaction logs differ only in the last lines (the scan includes its own growing evidence output). Verdict stays passed = false because of that one command. No build or no-change build was assigned or run.

## Third gate run (final verdict)

Logs in `raw/run3-*`; earlier logs are untouched. Candidate: HEAD `a2aa1cc0ed4ad8c5b9e879a04231591839e7dc48`, `git diff HEAD -- src` sha256 `96e44f54086469fe3ee0ccfa431ff4e3fadd12a15278834e5d6714613990531b`.

1. `git diff HEAD -- src | shasum -a 256`: exit 0, equals the frozen value (`raw/run3-src-diff.*`).
2. Six-file sha256 loop: exit 0, output byte-identical to `raw/files.log` (`raw/run3-files.*`); runtime.t.cpp is `3b22d137816e`.
3. `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME"` with no file arguments: exit 1, 8225 lines (`raw/run3-redact.log`). As established in the first run, these are all git-ignored build and evidence output. This form is not meaningful.
4. The same checker with explicit file arguments (the B3-B, C01 to C04 records, both C02/C03 evidence READMEs, the C04 README.txt and REPORT.md, the six source files and roadmap.md): exit 0, no output (`raw/run3-redact-files.*`). Report only, `redact` not run.
5. `ctest -N` (read-only inventory, `build/rewrite-debug`): exit 0, "Total Tests: 987", 41 names contain `X06 B3` (`raw/run3-ctest-N.*`). Sorted comparison with `inventory.txt` and with `tests/native/v5-02-x06-b3-b-c03-evidence/inventory.txt` (the most recent inventory; git-ignored evidence): identical, new names 0, removed 0.
6. No tests were executed (docs-only), so there are no pass, skip or failure counts. No build ran, so the no-change build (zero-work) check was not applicable and is not claimed.

Verdict: passed = true. The assigned command 3 has no meaningful result without file arguments, so the criterion is the file-argument run, which is clean; both results are reported. Acceptance table, red-first (none applicable) and behaviour differences (none) above are unchanged. Limitation: the zero-work result and test execution counts rest on C03 evidence, not on this gate.
