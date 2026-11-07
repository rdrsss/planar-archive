# RED-CHECK V5-02-R15-A2-C01 (checker-based red evidence, package P3)

Owner: coder, package P3 (role test). Base commit: `cdcfe829c466bf6338815ff6f5b6ea90ffddc570`.
This record replaces the ctest table. The ctest table is kept, unchanged, as `superseded/RED-ctest-table.md`
(`superseded/RED-ctest.md` and `superseded/RED-pre-redcheck3.md` are kept as they were).
Scope of this package: `red/` only (new files, `RED.md`, `snapshot-identity.txt` rewritten after an attempt-1 copy).
No file under `checks/`, `base/`, `plan/`, `docs/` or any record was edited, and nothing was run in place.

Mutation probes are not run or claimed here. The finisher produces them (`probes/PROBES.md`, `mutants.tsv`).

## 1. Snapshot pin

- The pre-edit snapshot is pinned by the base commit ID `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` plus the
  twenty `cmp` exits of `red/tree-verify.txt` (the original redtree check, kept unchanged).
- The re-run trees are verified in `red/tree-verify.attempt-2.txt`: section (a) `redtree` (20 `cmp` exits, the
  scratch tree built only from `base/`) and section (b) `stubtree` (20 `cmp` exits: ten base copies against `base/`
  and ten against the git blob at the base commit). All 40 exits are 0.
- Per-file sha256 (`shasum -a 256`) and the `git ls-tree` blob IDs of the ten base files are in
  `red/snapshot-identity.txt`. The earlier file is kept as `red/snapshot-identity.attempt-1.txt`.
  The workflow `identity.py` hash `e3b0c442...` quoted there is the empty diff of the gitignored `base/` folder,
  not a snapshot hash.

## 2. Commands

Placeholders: `<repo>` is the checkout root, `<scratch>` is the session scratch directory, `<home>` is unused
in this record. Nothing was run under `tests/native` in place. The checker and the stub script were copied to scratch
first, and the copies are byte-equal to the checkout originals (verified with `cmp` and `diff -r`).

Scratch construction:

```
cp <repo>/tests/native/v5-02-r15-a2-evidence/checks/contract_checks.py <scratch>/checks-copy/
cp -R <repo>/tests/native/v5-02-r15-a2-evidence/checks/fixtures <scratch>/checks-copy/fixtures
cp <repo>/tests/native/v5-02-r15-a2-evidence/red/stub-tree/make-stub-tree.py <scratch>/make-stub-tree.py
# redtree: ten files of base/ copied to <scratch>/redtree/base/ and to <scratch>/redtree/docs/vernacular/
python3 -I <scratch>/make-stub-tree.py <scratch>/redtree/base <scratch>/stubtree
```

`make-stub-tree.py` was run from the scratch copy `<scratch>/make-stub-tree.py`, never from `red/stub-tree/`.
Its source directory was the scratch `redtree/base`, which is byte-equal to `base/`.

Exact red-run command (snapshot, base bytes; output `red/red-run.txt`, exit `red/red-run.exit`):

```
cd <scratch>/checks-copy && python3 -I contract_checks.py --repo <scratch>/redtree --base <scratch>/redtree/base --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570
```

Exact stub-run command (headings present, no contract text; output `red/stub-run.txt`, exit `red/stub-run.exit`):

```
cd <scratch>/checks-copy && python3 -I contract_checks.py --repo <scratch>/stubtree --base <scratch>/stubtree/base --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570
```

Per-check logs: the same two commands with `--only NNN` added, one invocation per check (123 per tree), written to
`red/raw/checker.attempt-2/` (redtree) and `red/raw/stub.attempt-2/` (stubtree).

## 3. Results

| run | output | exit | summary line | Traceback lines |
| --- | --- | --- | --- | --- |
| red (base bytes) | `red/red-run.txt` | 1 | `SUMMARY pass=8 fail=115` | 0 |
| stub (headings present) | `red/stub-run.txt` | 1 | `SUMMARY pass=10 fail=113` | 0 |

- All 123 checks ran in each tree. Each per-check log's result line equals the line of the full run for the same
  check (compared by script: 0 mismatches in either tree). No `internal error` line appears.
- The FAIL set of the red run is the 115 checks whose `expectRed` is true. The PASS set is the eight guards
  `000 004 021 023 100 116 133 160`.
- The attempt-1 runs are kept: `red/red-run.attempt-1.txt` and `.exit` (byte-copies of the earlier `red-run.txt` and
  `.exit`, `cmp` exit 0 against `red/checker-run.txt`; `SUMMARY pass=7 fail=99`), and `red/stub-run.attempt-1.txt`
  and `.exit` (`SUMMARY pass=9 fail=97`). Reason for the re-run: see section 7.

## 4. FAIL lines by cause category

Categories are assigned by the grep patterns below, applied to the saved logs in this order (first match wins):

1. heading missing: `heading .* occurs 0 times`
2. literal missing: `missing (literal|exact|step marker)`
3. grammar/row observer (a row, cell, paragraph, count or token was not found): `lacks |Traceability row|Decisions table: |Acceptance checklist has|paragraph|listed 0 times|row A-[0-9]+ not found|no precondition member|no preview run token`
4. diff/other: `expected exactly one changed line|forbidden text present`

Counts were computed with `grep -c` on the saved logs. The check-number lists below were generated from the same two logs, and their lengths equal the counts:

| category | red run (`red-run.txt`) | stub run (`stub-run.txt`) |
| --- | ---: | ---: |
| heading missing | 84 | 0 |
| literal missing | 19 | 96 |
| grammar/row observer | 9 | 15 |
| diff/other | 3 | 2 |
| total FAIL | 115 | 113 |

Check numbers per category (`CHK-A2-` prefix omitted):

Red run:

- heading missing (84): 002 003 005 010 011 012 013 014 015 016 017 018 019 020 022 024 025 026 027 028 029 030 031 032 040 041 042 043 044 045 046 047 050 051 052 053 054 060 061 062 063 064 065 066 070 071 072 073 074 075 076 080 081 082 083 084 095 101 115 117 118 119 120 121 122 123 124 127 134 135 137 138 139 140 141 142 144 145 146 147 149 150 151 159
- literal missing (19): 090 091 092 093 094 096 097 098 099 112 128 129 131 132 136 143 148 153 158
- grammar/row observer (9): 110 111 125 130 152 154 155 156 157
- diff/other (3): 001 113 114

Stub run:

- heading missing (0): none
- literal missing (96): 002 010 011 012 013 014 016 017 018 019 020 022 024 025 026 027 028 029 030 031 032 040 041 042 043 044 045 046 047 050 051 052 053 054 060 061 062 063 064 065 066 070 071 072 073 074 075 076 080 081 082 083 084 090 091 092 093 094 095 096 097 098 099 112 115 117 118 119 120 121 122 123 124 127 128 129 131 132 134 135 136 137 138 139 140 141 142 143 144 147 148 149 150 151 153 158
- grammar/row observer (15): 005 015 101 110 111 125 130 145 146 152 154 155 156 157 159
- diff/other (2): 113 114

PASS lines of the red run (8): `CHK-A2-000`, `004`, `021`, `023`, `100`, `116`, `133`, `160`. All are guards.

### Base-snapshot run: what it shows

On the base snapshot most FAIL lines share one cause. 82 of the 115 FAIL lines (71%) report a missing P2 heading
(`### P2-x ... occurs 0 times` or `## Part 2 ... occurs 0 times`), and two more (`005`, `159`) report the missing
`## Part 1 amendments (R15-A2)` heading. The base run therefore shows that the checks need the new text. It does not
show that each check detects its own invariant. The stub run below is the distinct-cause evidence.

The 31 non-heading base FAIL lines are: 19 literal misses, 9 structure observers and 3 others. The structure
observers are `110` (Traceability row `R15-2` lacks its anchor), `111` and `152` (the base Decisions table has rows
D1 to D103 only, so the first required row, `D104` or `D143`, is absent), `125` (the revision-conflict cell lacks the
A-8 text), `130` (no `Pointer (R15-A2):` paragraph), `154` (no load pointer paragraph), `155` (the Acceptance checklist
has 0 F3 to F8 items) and `156`, `157` (the `R15-4` and `R15-5` Traceability labels are absent). The others are `001`
and `114` (placeholder text present on base) and `113` (the Execution design file has no changed line).

## 5. expectRed checks that PASS on the stub run

Computed from `red/stub-run.txt` (10 PASS lines: `000 001 003 004 021 023 100 116 133 160`). Of these, the checks
whose `expectRed` is true in the checker and in the `expectRed` column of the invariant tables (R15-A2 `plan/INVARIANTS.md`
and C01 `plan/INVARIANTS.md`) are exactly two: `CHK-A2-001` and `CHK-A2-003`. The other eight PASS lines are guards
(`expectRed` false). Their reasons:

- `CHK-A2-001 no_placeholder_text` (row I01). Its only red cause on base is the placeholder text `Not yet written`
  (`forbidden text present`). The stub removes that placeholder section, so the check passes. It has no content-level
  cause on base. Its mutant (re-inserting the placeholder) is for the prober on the final text.
- `CHK-A2-003 seven_headings_once` (row I03). Its only red cause on base is the missing headings (`heading ... occurs 0 times`).
  The stub has the P2 headings, each once and in order, so the check passes. It has no content-level cause on base.

These two PASSes show that the stub run gives no red cause for rows I01 and I03. Their red status rests on the
finisher's mutant probes against the final text, which are not claimed here. The previous stub run (attempt 1) also
passed these two checks; the new stub run adds only the new guard `CHK-A2-160` to the PASS list.

## 6. Comparison of the FAIL set with the expectRed rows

- The red-run FAIL set (115) equals the set of checks with `expectRed` true in the checker flags, and it matches the
  `expectRed` column of the rows of the two invariant tables, except for `CHK-A2-005` (see the last item of this
  section). Every one of the 123 checks is named by at least one row, and no row names a check the checker lacks.
- Stub-run: 113 FAILs. The only expectRed checks that do not FAIL are `001` and `003` (section 5).
- Guard check `CHK-A2-160 allowed_messages_extraction` (row I129) PASSes on base and on the stub, as required.
- `CHK-A2-149 no_identifier_carries_data` and `CHK-A2-155 checklist_ra01c` FAIL on base, as required by C01-D20.
  On base, `149` fails at the heading lookup that precedes its presence literal (`heading ### P2-3 work_exec_v1
  occurs 0 times`). The stub run reaches the presence literal (`missing literal: Generation is not a W field: ...`).
  `155` fails on base with `Acceptance checklist has 0 F3 to F8 items`.
- Difference found: `CHK-A2-005 part1_amendments_listed` is `true` in its checker flag and in row I05 of the R15-A2
  plan, but row I134 of the C01 plan gives `false` for the same check, and calls it a guard. On base `005` FAILs
  (`heading ## Part 1 amendments (R15-A2) occurs 0 times`), which matches the checker flag and not I134. This package
  does not edit plan rows or checks. It is reported for the record owner and the finisher.

## 7. Attempt-1 files kept, and why the checker was re-run

Kept, not overwritten: `red-run.attempt-1.txt`, `red-run.attempt-1.exit`, `stub-run.attempt-1.txt`,
`stub-run.attempt-1.exit`, `snapshot-identity.attempt-1.txt`.

Reason: the attempt-1 `CHK-A2-022 no_new_message` extraction defect. The helper that collected allowed messages paired
backticks over whole files (one regular expression run over the full text of each file, not per line). The ten base files contain 90 lines with an odd number
of backticks, so a stray backtick shifts the pairing for everything after it. Whole-file pairing and per-line pairing
give different string sets for these files. The attempt-1 red log of `CHK-A2-022` could not show this, because the
check failed earlier on its missing heading. The repaired helper pairs backticks per line and per table cell. `CHK-A2-160`
guards the repaired extraction, and the self-test `CHK-A2-000` gained the `allowed` fixtures (`allowed.good.txt`, `allowed.bad.txt`).
The attempt-2 runs use the repaired checker.

## 8. Raw evidence

- New folders: `red/raw/checker.attempt-2/` (123 per-check logs, `CHK-A2-NNN_<name>.log`, redtree) and
  `red/raw/stub.attempt-2/` (123 logs, stubtree). Each log has the header `tree: ...`, the `$ <command>` line, the
  result line, `SUMMARY` and `exit=`, in the attempt-1 format.
- New file: `red/raw/RED-A2-002_attempt2.log` (the before and after listings of `red/raw` with `shasum -a 256`, the
  `cmp` exits, and the steps taken).
- Existing files under `red/raw/` were not written: `checker/`, `stub/`, `checker-pre-020-fix/`, `stub-pre-020-fix/`,
  the ctest logs, `build.log` and the earlier logs. The two listings (before and after) of the pre-existing files are
  identical, as recorded in the log.
- The per-check logs show `contract_checks.py` run from `<scratch>/checks-copy` (the attempt-1 logs show the path
  `checks/contract_checks.py`; the bytes are the same, see the scratch copy check above).

## 9. Limits

- No mutation probe is run here, so no kill is claimed for any row. The logs show red and PASS behaviour only.
- The base run does not show a distinct cause for each check (section 4). The stub run is a scratch construction
  with headings and no contract text. It is not a product stub.
- `tree-verify.txt` has no git-blob lines. Section (b) of `tree-verify.attempt-2.txt` adds ten git-blob comparisons
  for the stub base copy. The redtree section has the same form as the original file.
- No CTest registration and no product build: no product code is changed by this package.
- The `expectRed` value of `CHK-A2-005` disagrees between plan rows (section 6). No check was edited.
