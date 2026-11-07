# V5-02-R15-A1 gate report (documentation round)

Gate runner run, 2026-11-01. No source file was edited. Raw logs are under `tests/native/v5-02-r15-a1-evidence/raw/`.

Bottom line: every command the brief lists met its criterion. The round changes only Markdown, so there is no product build, no test binary and no Docker gate. Matched-test count for this round is therefore not applicable (a docs round has no TEST_CASE). Thirteen scratch text checks (`red/ex_checks.py`) still FAIL against the live checkout. They are not in the listed command set, so they do not decide `passed`, but they are open findings for the reviewer (see "Open items").

## Candidate identity

HEAD `d536922b59bf9118b91e4188a26d770f9745d600` (V5-02-RA01-C-C01), branch `dev/planar-v2-vernacular`. Output of `identity.py` over `src tests`, verbatim (`raw/identity-src-tests.txt`, exit 0):

| identity | value |
|---|---|
| sha256(git diff --binary HEAD -- src tests), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256, pathspec src tests, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

Both are the sha256 of empty input: no source or test byte changed (acceptance criterion 9). The wider identity over `src tests/native docs/vernacular CLAUDE.md` (`raw/identity-wide.txt`) has tracked diff sha256 `eb2deed97a9fa03401a5625c75d0ccbd4d869c314457b5b634fa474253f27302` and combined sha256 `b5bb09d612bc9dda648a60efa5fd37cccf5f3751ed3dec6c1ac0005d57e115dc`; its per-file rows are in that file (not copied here, per criterion 10).

## Commands, exit codes and results

| command | exit | result |
|---|---|---|
| `redact` then `check` per file, one file per invocation, 8 files (CLAUDE.md, acceptance-matrix, external-profile-contract, implementation, ledger-contract, v5-alignment, execution-contract, rounds/V5-02-R15-A1) | 0 for each of the 8 | `redacted 0 files` every time; nothing needed redacting. `raw/redact-exits.txt`, `raw/redact-1.txt` to `redact-8.txt` |
| positive control: scratch file `control.txt` in the scratchpad containing the repository root | 1 (non-zero, as required) | `raw/redact-control.txt` |
| link and anchor check, scratch script `raw/linkcheck.py` (outside the checkout when run; copy preserved) over the 8 changed documents | 0 | 410 links checked, 0 broken. Percent-encoded names are decoded and inline-code spans are skipped. `raw/linkcheck.txt` |
| `git diff --stat HEAD` | 0 | six tracked files, 31 insertions and 7 deletions; only owned files (`raw/diffstat.txt`). Untracked: execution-contract.md and the round record, both owned |
| `test -L AGENTS.md && readlink AGENTS.md` | 0 | prints `CLAUDE.md`; symlink intact |
| `identity.py src tests` | 0 | empty identity, above |
| `identity.py src tests/native docs/vernacular CLAUDE.md` (extra) | 0 | `raw/identity-wide.txt` |
| `ctest --test-dir build/rewrite-debug -N` (extra, read-only) | 0 | 1439 registered tests, `inventory.txt` |
| no-change build check | not run | no build was run (docs round); a no-change check is only meaningful after a successful build, so it is not applicable |
| Docker / Linux gate | not run | docs round; disk had 139 GiB free, so the 15 GiB floor was not the reason |

One correction to the control wording above: the control file is `/private/tmp/claude-501/-Users-mn-projects-github-rdrsss-planar/aa194b3a-a2f1-48b9-a031-343fdb418741/scratchpad/control.txt`.

An earlier link-check draft reported 4 false breaks (three percent-encoded `Planar%20design%20spec%20-%20v4.md` links that resolve to the existing file `docs/vernacular/Planar design spec - v4.md`, and one link inside an inline-code span at `execution-contract.md:964`). The script was fixed and rerun; the final run is the one preserved.

## Test inventory

Skips: none (no test run). Registered inventory: `ctest -N` on `build/rewrite-debug`, 1439 tests, saved as `inventory.txt` (with ctest's numbering). The most recent committed inventory is `tests/native/v5-02-ra01-c-c02-evidence/inventory.txt` (names only, 1439 lines, tracked by RA01 evidence at HEAD lineage). Sorted-name diff after stripping ctest's numbering: empty (`raw/inventory-diff.txt`, 0 lines). New names: 0. Removed names: 0. The inventory is from the existing debug build tree, not rebuilt for this round, which is consistent because no source changed.

## Zero-work build result

Not applicable: no build was performed. The brief's criterion (no-change build must report zero steps) was not exercised because there was no preceding successful build in this round.

## (a) Acceptance criteria to evidence

No TEST_CASE exists in a docs round, so "test name with file:line" is replaced by the scratch check id (CHK, defined in `red/ex_checks.py`, rows in `plan/INVARIANTS.md`) and the contract location (`docs/vernacular/execution-contract.md` line). The checks are scratch text checks, not product tests. Live result from `raw/ex_checks-final.txt` (57 PASS, 13 FAIL; exit 1).

| AC | criterion | contract location | checks and live result |
|---|---|---|---|
| 1 | EX-1 to EX-8, state machines, reconcile rules exact; format3 SQL complete | EX-1 line 42 (SQL block 81-353), EX-2 479, EX-3 540, EX-4 643, EX-5 710, EX-6 722, EX-7 730, EX-8 785, State machines 802 | I1 PASS, I3 PASS, I4 PASS, I6 PASS, I7 PASS, I9 to I21 PASS, I48 PASS, I49 PASS, I52 PASS. FAIL: I2, I5, I47, I53, I67 (SQL extraction, see Open items), I8 (cutover text) |
| 2 | every transition has before, cause, after, reason; precedence ordered with v5 lines | State machines 802; EX-4 precedence 643-710 | I16 PASS, I26 PASS, I27 PASS, I28 PASS, I56 PASS, I68 PASS, I41 PASS. FAIL: I60, I69 (stale to ready row check) |
| 3 | nine codes, one new message, reused messages named, no other new code | EX-7 730, Messages 920 | I22 PASS, I23 PASS, I58 PASS, I24 PASS. FAIL: I66 (reused-message lookup), I64 (limit name) |
| 4 | traceability and decisions table, OP1 to OP3 | Traceability 972, Decisions table 1023 | I31 PASS, I32 PASS, I33 PASS. |
| 5 | Part 2 placeholders without normative text | 888 (seven placeholders, first at 892) | I29 PASS, I30 PASS |
| 6 | pointer amendments scoped; one CLAUDE.md entry; symlink | Pointer amendments 960 | I35 PASS, I36 PASS, I37 PASS, I38 PASS, I40 PASS. FAIL: I39 (v5-alignment E2 entry) |
| 7 | citations read, every link and anchor resolves | all | I41 PASS. Independent link check: 410 links, 0 broken (this gate). I42 FAIL is a checker limitation (does not decode `%20`; see Open items) |
| 8 | redaction exit 0; positive control non-zero | n/a | this gate: 8 of 8 exit 0, control exit 1; I43 PASS |
| 9 | src and tests identity empty | n/a | this gate identity above; I44 PASS, I45 PASS |
| 10 | record rule: no cross-record hashes | round record, README | I46 FAIL: `tests/native/v5-02-r15-a1-evidence/README.md` does not exist yet; I59 PASS |

No corrective round and no findings are closed by this round.

## (b) Red-first

This round has no regression test. The scratch checks are the only red evidence, from the coders' file `red/red-run.txt` (section "Pre-round snapshot"):

- Command (verbatim from that file): `python3 -I <session scratchpad>/ex_checks.py --repo <session scratchpad>/coder-T1/pre-round`.
- Snapshot: local clone of HEAD `d536922b` with the round record and plan folder copied in, contract absent. Hash as recorded in the round record, section (a) 2: tracked diff sha256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`, combined `aaf3b235546232b5a5482431e815d3407292404d6f2193b221bab90a9742cf28`, pathspec `src tests/native docs/vernacular CLAUDE.md`. I could not recompute it (the snapshot is a scratch clone), so this is the coders' figure.
- Failing line: first is `FAIL CHK-I1-skeleton document missing: ...coder-T1/pre-round/docs/vernacular/execution-contract.md`.
- Result: 70 checks, 3 PASS (I24, I44, I45, guards that already passed, `expectRed=false`) and 67 FAIL. 62 of the 67 fail because the document is absent, so those are real red only for existence, not for rejecting a wrong clause. The mutation prober result for the checks is not in the evidence folder; I have no prober command or output to cite, so none is claimed.

## (c) Deliberate behaviour differences

No product behaviour changes. Documented contract differences, all in the round record: HALT-1 fixed texts for nine new codes and nine fallbacks (E16 A amendment, delegated under the 2026-10-31 standing rule), OP1 (`--confirm-stopped` required scalar `true`, E1 A sub-choice), OP2 (`history --request-id` returns execution results on format3 only, E1 A sub-choice and R15-9), OP3 (`evidence_inapplicable` for both directions of stale/done). The v5 deviation record (E2, v5 lines 110 and 497) is expected in `v5-alignment.md`, but check I39 reports it missing; see below.

## Open items for the reviewer (not decided by this gate)

Thirteen live check FAILs. Only the checker output is evidence; I did not investigate beyond a quick look and I did not edit anything.

- I2, I5, I47, I53, I67: the checker extracts only 14 tables and says `checkpoints`/`reservations`/`evidence` are missing from the SQL. The contract text does contain `CREATE TABLE checkpoints` (line 186) and `CREATE TABLE reservations` (line 193) inside the same SQL fence, so either the checker's extraction (`sql_block`, `ex_checks.py:326`) mis-parses after the first `-- Reserved for R16` comment or the SQL fails to execute. Needs reviewer attention; if the SQL really does not execute under sqlite, criterion 1 is not met.
- I8: old-binary refusal text not found by the checker.
- I39: `v5-alignment.md` E2 deviation entry not found, although the file shows +13 lines.
- I42: the checker does not decode `%20`; independent check shows 0 broken links.
- I46: evidence `README.md` not written.
- I60, I69: the checker reports a `stale -> ready` row.
- I64: `observation_bytes` is not kept with `Capture limit exceeded`.
- I66: a reused-message cell reads "The existing message of the shared JSON parser ...; no single string", which the checker rejects.

## Limitations

Docs round: no build, no ctest run, no Docker, no mutation prober. The inventory is of the existing debug build, not rebuilt. Raw logs preserved, none deleted.
