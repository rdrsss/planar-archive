# V5-02-R15-A2 plan: supersession by V5-02-R15-A2-C01

Written in round V5-02-R15-A2-C01, package P4, 2026-11-06 (C01-D21). This is the only file added to this plan folder by C01. The R15-A2 plan files (`DECISIONS.md`, `INVARIANTS.md`, `superseded-1/` to `superseded-3/`) are not edited. Where this note supersedes a row, footer or count, the R15-A2 text stays as written and this note states the replacement. The C01 plan is in `tests/native/v5-02-r15-a2-c01-evidence/plan/`. Paths use the placeholders `<repo>`, `<scratch>` and `<home>`.

The counts below are derived from saved runs or are marked as the finisher's. The attempt-2 red run is `red/red-run.txt` (`SUMMARY pass=8 fail=115`, exit 1). The C01 P2 diagnostic over the final text gave `SUMMARY pass=123 fail=0`; it is scratch output and not evidence. The green run on the final text is the docs evidence finisher's, and is pending.

## 1. Row I67 (retargeted)

The R15-A2 row I67 covers the RC pointers. Its second mutant deleted the sentence that the generation is carried by `dispatch_v1` and `dispatch_manifest_v1` (`generation`), not by W. C01 removes that sentence (C01 L5, L6 and L8; C01-D5), so the second mutant is retargeted.

- Invariant (retargeted): RC pointers: dispatch mode, `work_exec_v1`, identifier list, render stays database-free; the `work_exec_v1` pointer states that `repair` enters W and that the generation is the `generation` key of `dispatch_manifest_v1` and the `{generation}` of the authority statement, not a W field, citing the EX-5 bullet (E5 A) and the rule that takeover must not move W.
- Mutant 1 (unchanged): delete `database-free` from the RC pointer.
- Mutant 2 (retargeted, replaces the R15-A2 second mutant): in `render-contract.md`, in the `work_exec_v1` pointer, delete the words `not a W field`.
- Test: CHK-A2-096 `rc_pointers`, which after C01 requires the three RC literals `not a W field`, the L8 generation wording and the L5 wording. Row I144 is a further row on the same check.

## 2. Row I83 (restated)

- R15-A2 wording: the FAIL count equals the number of `CHK-A2-*` rows with expectRed true (footer 99), and every guard PASSes (`SUMMARY pass=7 fail=99`).
- Restated: the FAIL count equals the number of checks with expectRed true in the final checker, 115 (the 99 of R15-A2 plus CHK-A2-144 to CHK-A2-159). The guards are CHK-A2-000, 004, 021, 023, 100, 116, 133 and 160.
- Observed (attempt 2, `red/red-run.txt`, exit 1): `SUMMARY pass=8 fail=115`, no Traceback. The attempt-1 figure `pass=7 fail=99` is kept in `red/red-run.attempt-1.txt`.

## 3. Row I106 (restated)

- R15-A2 wording: each mutant log shows a SUMMARY total of 106, and the baseline reads `SUMMARY pass=106 fail=0`.
- Restated: each mutant log shows a SUMMARY total of 123, and the baseline (the unmutated positive control of the final text) must read `SUMMARY pass=123 fail=0` with a passing self-test. Both are the finisher's outputs and are pending.

## 4. Footer totals (restated)

| Quantity | R15-A2 footer | Final checker after C01 |
| --- | --- | --- |
| Checks named `CHK-A2-*` | 106 | 123 (106 plus 17 new checks, CHK-A2-144 to CHK-A2-160) |
| Checks with expectRed true | 99 | 115 (99 plus CHK-A2-144 to CHK-A2-159; 16 new checks) |
| Guards (expectRed false) | 7: CHK-A2-000, 004, 021, 023, 100, 116, 133 | 8: the same seven plus CHK-A2-160 (guard of the repaired extraction, row I129) |
| Red run on base | `pass=7 fail=99` (expected) | `pass=8 fail=115` (observed, `red/red-run.txt`) |
| Green run on final text | `pass=106 fail=0` (expected) | `pass=123 fail=0` (expected; the finisher's log is pending) |

The structural change is the 17 new checks CHK-A2-144 to CHK-A2-160. Of these, 16 are expectRed true (CHK-A2-144 to CHK-A2-159), and CHK-A2-160 is a guard. Existing checks whose literals changed keep their names.

## 5. Footer claim of no shared checks (R15-A2 `INVARIANTS.md` line 149) (superseded)

The R15-A2 footer says: "Every row has its own named check function; no check is shared between two rows." That claim is superseded. After C01 these checks are named by more than one row:

| Check | Rows that name it |
| --- | --- |
| CHK-A2-022 `no_new_message` | R15-A2 I18; C01 I130 (forward direction) and I131 (reverse direction) |
| CHK-A2-096 `rc_pointers` | R15-A2 I67 (retargeted in section 1); C01 I144 |
| CHK-A2-111 `decisions_rows` | R15-A2 I74; C01 I128 and I148 |
| CHK-A2-043 `manifest_fields` | R15-A2 I32; C01 I133 (extended in place) |

The R15-A2 record marks the counts, the I67 row, the I83 row and the I106 row as superseded by C01, with pointers to this note and to the C01 record Results.

## 6. Mutants added for the finisher

The finisher's set is the one in the subsection `Mutant set for the finisher` of the C01 record Results (`docs/vernacular/rounds/V5-02-R15-A2-C01.md`). The mutant text is in the C01 plan `INVARIANTS.md`. The rows below list the variants that the finisher must add to `mutants.tsv` as their own entries. Each has a non-empty diff, the killing `FAIL` line, and a SUMMARY total of 123. A probe whose replace matches nothing is a defect.

| Row | Variant | Mutant (summary) | Killing check (FAIL line names it) |
| --- | --- | --- | --- |
| I112 | main | P2-2 `run` row: delete the clause `for preview the value is {kind:"preview"} with no request key` | CHK-A2-144 |
| I113 | main | P2-2: delete the sentence that `render`, `sections` and `omitted` are computed from the preview packet's own bytes | CHK-A2-145 |
| I113 | b | preview_v1 definition: `current generation ordinal` to `next generation ordinal` | CHK-A2-145 |
| I113 | c | insert a separate sentence ` Its `sections` always differ from a dispatch's.` after `never stored.`; replace count 1 | CHK-A2-145, the FAIL line names `always differ` |
| I114 | main | the two `run request: (preview)` occurrences inside P2-2 to `run request: none`; replace count 2 | CHK-A2-146 (CHK-A2-042 may also fail) |
| I114 | b | the same two occurrences to `run request: preview`; replace count 2 | CHK-A2-146 |
| I114 | c | all three occurrences (including the D154 cell) to `run request: none`; replace count 3 | CHK-A2-146 (CHK-A2-111 may also fail) |
| I115 | main | delete the P2-2 bullet that starts `` `dispatch_v1` is the identifier of the dispatch projection rules `` | CHK-A2-147 |
| I116 | main | render pointer: delete the clause `` `dispatch_v1` names the dispatch projection rules of P2-2 `` | CHK-A2-148 |
| I117 | main | D120: restore `carried by `dispatch_v1` and `dispatch_manifest_v1` (`generation`)` | CHK-A2-149 |
| I117 | b | the same restore in the render pointer | CHK-A2-149 |
| I117 | c | delete the L5 literal from P2-3 while no `carried by` sentence exists | CHK-A2-149 (fails on presence) |
| I118 | main | P2-3: delete `the `generation` key of `dispatch_manifest_v1` and ` | CHK-A2-150 |
| I118 | b | the same deletion in the P2-7 render bullet | CHK-A2-150 |
| I119 | main | P2-5: replace statement S by `EX-4 rejects `none` for a job` | CHK-A2-151 |
| I120 | main | D143: replace statement S by `EX-4 rejects it for a job` | CHK-A2-152 |
| I121 | main | line 540: rejoin to `in any packet, and in the claim and takeover results only in the field `run.token`, in logs or in error details.` | CHK-A2-153 |
| I122 | main | load pointer: move the not-shipped sentence back between `and are:` and the fence | CHK-A2-154 |
| I123 | main | checklist: the F5 item reads `carried from the R15-A1 coverage gaps` | CHK-A2-155 |
| I123 | b | checklist: delete the F8 item (the count of six fails) | CHK-A2-155 |
| I124 | main | traceability: restore the label `R15-4 Claim and takeover precondition sets` | CHK-A2-156 |
| I125 | main | traceability: restore the label `R15-5 Lock-time mismatch mapping for claim and takeover` | CHK-A2-157 |
| I126 | main | A-11 Location: delete `and the lead sentence (base line 966)` | CHK-A2-158 |
| I126 | b | A-11 Change: delete the quoted lead-sentence change | CHK-A2-158 |
| I127 | main | A-2 row: restore `Lines 518, 527 and 528` | CHK-A2-159 |
| I128 | main | D154 tag cell: `new-engineering` to `operator-decided` | CHK-A2-111 |
| I129 | guard | checker copy: restore whole-file extraction in `allowed_messages` (must FAIL the guard) | CHK-A2-160 |
| I130 | main | P2-1: add the message `Role is not claimable` to a refusal row | CHK-A2-022 |
| I131 | main | scratch `base/`: delete every `Unknown selected job` in `execution-contract.md` and `authoring-contract.md` | CHK-A2-022 (CHK-A2-133 may also fail) |
| I132 | guard | checker copy: restore whole-file pairing in `allowed_messages` | CHK-A2-000 (self-test) |
| I133 | main | run manifest row: `kind:"preview"` to `kind:"claim"` | CHK-A2-043 |
| I134 | guard | change one word of an unrelated Part 1 sentence (EX-5 `Lineage` to `ancestry`) | CHK-A2-005 (the plan lists it as a guard; see the C01 record open item 4) |
| I135 | guard | delete one base line outside the load pointer | CHK-A2-100 |
| I136 | guard | EX-7: `Run is not current` to `Run is no longer current` | CHK-A2-021 |
| I144 | main | render pointer (line 416): restore `not by W` | CHK-A2-096 |
| I144 | b | render pointer (line 416): delete `not a W field` | CHK-A2-096 |
| I148 | main | D153 Design choice cell: delete `current generation ordinal` | CHK-A2-111 |
| I148 | b | D153 basis cell: `R15-7, R15-6` to `R15-6, R15-2` | CHK-A2-111 |
| I148 | c | D155 Design choice cell: delete `not a W field and ` | CHK-A2-111 |

The R15-A2 rows I01 to I111 (with I67 as retargeted in section 1, and I93 absent) are finisher re-probes on the final bytes, as the C01 plan's acceptance 7 requires. Each expectRed true mutant must turn the named check from PASS on the final text to FAIL with that name in the FAIL line.
