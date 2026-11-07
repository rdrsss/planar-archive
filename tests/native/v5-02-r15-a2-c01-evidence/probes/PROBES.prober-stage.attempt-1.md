# Probes for V5-02-R15-A2-C01

Pointer (C02, 2026-11-07): this prober-stage table is kept unchanged. The true verdicts of the first attempts of I07, I12, I18, I21, I60, I63 and I69 (five survived the pre-fix checker, I18 was killed only by CHK-A2-017, I63 attempt 1 has an empty diff) are in the Correction (C02) block of `PROBES.md` in this folder and in `tests/native/v5-02-r15-a2-c02-evidence/probes/PROBES.md`.

Mutation prober, 2026-11-06. Documentation round: the observer is the checker `contract_checks.py`, run on a scratch copy outside the checkout (`<scratch>/pw`: `docs/vernacular/*.md` and the evidence `base/` and `checks/`); no product build is involved, so no cmake target was rebuilt (the warm build tree is irrelevant to a Python checker). Each mutant is a text edit of one scratch file, applied only if the replace count matched, run with `python3 -I contract_checks.py --repo <scratch>/pw --base <scratch>/pw/ev/v5-02-r15-a2-evidence/base --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570`, then restored by byte copy (sha256 re-verified, column Restored). Baseline on the unmutated scratch copy: `raw/baseline.log`, `SUMMARY pass=123 fail=0`.

Identity (tool output verbatim; pathspec includes the checker folder):

| identity | value |
|---|---|
| HEAD | `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` |
| sha256(git diff --binary HEAD -- docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md tests/native/v5-02-r15-a2-evidence/checks), tracked only | `312fabfc4520d7670aa085a70508672b0232a514b32d3d8ceaed426c47b7c08f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md tests/native/v5-02-r15-a2-evidence/checks, evidence excluded | `312fabfc4520d7670aa085a70508672b0232a514b32d3d8ceaed426c47b7c08f` |

| file | state | sha256 |
|---|---|---|
| `docs/vernacular/command-contract.md` | modified | `34ae9d4010f8e13a4183ce452d18395be9eb7f0f95d54eb3bc7aad34d48d4e8a` |
| `docs/vernacular/execution-contract.md` | modified | `38c0d3f70afe6412dc7b7ffe90270bb956562752493c690f0b740714f9176d75` |
| `docs/vernacular/execution-design.md` | modified | `1a5134a0815bb6a18bcb069b96eed94bd28ce247db88b5e4cf54c798c612cb87` |
| `docs/vernacular/external-profile-contract.md` | modified | `86fcbefbf070293b4da7f7efbab1dd7b6065b6d02abc0df7e4bcfa5199f0c493` |
| `docs/vernacular/ledger-contract.md` | modified | `46d8a0eef9f5a65822efbeea1a6e5b61dbd14f3b0adf3e0c01ade94112f4a84f` |
| `docs/vernacular/load-command-contract.md` | modified | `af99144c596184c3ec90a60f3f773fff8509401e14f76502f49b59684e70ee22` |
| `docs/vernacular/render-contract.md` | modified | `22a702eb57177f0c4a3d9b52ca3eb43cf9533a09d63c8586b3c3292d1e2ca874` |


No earlier PROBES.md exists in this lineage (R15-A2 has no `probes/`), so nothing is carried forward; the first-round rows I01 to I111 are re-probed by the finisher (acceptance 7), not here.

## Probe table

Killed means the named check printed a FAIL line because of the mutant. "Other FAIL" lists additional checks that failed; they are noted, not counted. Mutant hash is the first 16 hex of the sha256 of the unified diff in the raw log.

| Row/variant | Expected killer | Mutant hash | Killed | Killing line | Other FAIL | Fails | Restored | Source |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| I112 | CHK-A2-144 | `a938d8d59c64fbc4` | yes | CHK-A2-144 preview_manifest_run | CHK-A2-043, CHK-A2-044 | 3 | yes | run, `raw/I112.log` |
| I113 | CHK-A2-145 | `58491822a516ab4a` | yes | CHK-A2-145 preview_manifest_same_keys | - | 1 | yes | run, `raw/I113.log` |
| I113b | CHK-A2-145 | `f8931992ba199b95` | yes | CHK-A2-145 preview_manifest_same_keys | - | 1 | yes | run, `raw/I113b.log` |
| I113c | CHK-A2-145 | `af48a8359708f4bd` | yes | CHK-A2-145 preview_manifest_same_keys | - | 1 | yes | run, `raw/I113c.log` |
| I114 | CHK-A2-146 | `70629d92d185347c` | yes | CHK-A2-146 preview_run_spelling_no_collision | CHK-A2-042, CHK-A2-145 | 3 | yes | run, `raw/I114.log` |
| I114b | CHK-A2-146 | `acfdb22c1161e0fb` | yes | CHK-A2-146 preview_run_spelling_no_collision | CHK-A2-042, CHK-A2-145 | 3 | yes | run, `raw/I114b.log` |
| I114c | CHK-A2-146 | `2bbeadaf3452d80c` | yes | CHK-A2-146 preview_run_spelling_no_collision | CHK-A2-042, CHK-A2-111, CHK-A2-145 | 4 | yes | run, `raw/I114c.log` |
| I115 | CHK-A2-147 | `d95e4dc0ffc73846` | yes | CHK-A2-147 dispatch_v1_defined | - | 1 | yes | run, `raw/I115.log` |
| I116 | CHK-A2-148 | `759b3db21a8ac4f3` | yes | CHK-A2-148 render_pointer_dispatch_v1 | - | 1 | yes | run, `raw/I116.log` |
| I117 | CHK-A2-149 | `5fce03bdb29a5066` | yes | CHK-A2-149 no_identifier_carries_data | CHK-A2-150 | 2 | yes | run, `raw/I117.log` |
| I117b | CHK-A2-149 | `03e552100988e21b` | yes | CHK-A2-149 no_identifier_carries_data | CHK-A2-096, CHK-A2-150 | 3 | yes | run, `raw/I117b.log` |
| I117c | CHK-A2-149 | `bb42bb41f7cc0af3` | yes | CHK-A2-149 no_identifier_carries_data | CHK-A2-096, CHK-A2-150 | 3 | yes | run, `raw/I117c.log` |
| I118 | CHK-A2-150 | `e08c05946c26db00` | yes | CHK-A2-150 generation_is_manifest_key | CHK-A2-096, CHK-A2-149 | 3 | yes | run, `raw/I118.log` |
| I118b | CHK-A2-150 | `cee14cad5093ad60` | yes | CHK-A2-150 generation_is_manifest_key | - | 1 | yes | run, `raw/I118b.log` |
| I119 | CHK-A2-151 | `d77ac63142a25b0d` | yes | CHK-A2-151 ex4_none_statement_p25 | - | 1 | yes | run, `raw/I119.log` |
| I120 | CHK-A2-152 | `d1b4072b88766fc7` | yes | CHK-A2-152 ex4_none_statement_d143 | CHK-A2-151 | 2 | yes | run, `raw/I120.log` |
| I121 | CHK-A2-153 | `d7d9a12717a3963d` | yes | CHK-A2-153 a3_token_sentence_split | CHK-A2-076 | 2 | yes | run, `raw/I121.log` |
| I122 | CHK-A2-154 | `44c08130339ca5f4` | yes | CHK-A2-154 load_pointer_help_order | - | 1 | yes | run, `raw/I122.log` |
| I123 | CHK-A2-155 | `52410691cfa3d8d9` | yes | CHK-A2-155 checklist_ra01c | - | 1 | yes | run, `raw/I123.log` |
| I123b | CHK-A2-155 | `e99baf200f6a937a` | yes | CHK-A2-155 checklist_ra01c | CHK-A2-112 | 2 | yes | run, `raw/I123b.log` |
| I124 | CHK-A2-156 | `f5a9c534b3eff029` | yes | CHK-A2-156 trace_r15_4_claim | - | 1 | yes | run, `raw/I124.log` |
| I125 | CHK-A2-157 | `c54a3bbc34bbad03` | yes | CHK-A2-157 trace_r15_5_takeover | - | 1 | yes | run, `raw/I125.log` |
| I126 | CHK-A2-158 | `6e7d18ab579e0fc0` | yes | CHK-A2-158 lead_sentence_listed | - | 1 | yes | run, `raw/I126.log` |
| I126b | CHK-A2-158 | `b9aec854be7c9d10` | yes | CHK-A2-158 lead_sentence_listed | - | 1 | yes | run, `raw/I126b.log` |
| I127 | CHK-A2-159 | `0f69706eed33694a` | yes | CHK-A2-159 a2_line_cite | - | 1 | yes | run, `raw/I127.log` |
| I128 | CHK-A2-111 | `25afe575d8022c27` | yes | CHK-A2-111 decisions_rows | - | 1 | yes | run, `raw/I128.log` |
| I129 | CHK-A2-160 | `8961dec7fd0738b9` | yes | CHK-A2-160 allowed_messages_extraction | CHK-A2-022, CHK-A2-000 | 3 | yes | run, `raw/I129.log` |
| I130 | CHK-A2-022 | `d8c3b285add1d5b5` | yes | CHK-A2-022 no_new_message | CHK-A2-017 | 2 | yes | run, `raw/I130.log` |
| I131 | CHK-A2-022 | `5c16b2c80a33902c` | yes | CHK-A2-022 no_new_message | CHK-A2-021, CHK-A2-084, CHK-A2-160, CHK-A2-133 | 5 | yes | run, `raw/I131.log` |
| I132 | CHK-A2-000 | `8961dec7fd0738b9` | yes | CHK-A2-000 selftest_positive_negative | CHK-A2-022, CHK-A2-160 | 3 | yes | run, `raw/I132.log` |
| I133 | CHK-A2-043 | `3821529dcb1b2543` | yes | CHK-A2-043 manifest_fields | CHK-A2-044, CHK-A2-144 | 3 | yes | run, `raw/I133.log` |
| I134 | CHK-A2-005 | `4a087f91b28b9e4b` | yes | CHK-A2-005 part1_amendments_listed | - | 1 | yes | run, `raw/I134.log` |
| I135 | CHK-A2-100 | `bb3521f95b9021d9` | yes | CHK-A2-100 pointers_narrow_nothing | - | 1 | yes | run, `raw/I135.log` |
| I136 | CHK-A2-021 | `a02f75c3478703c4` | yes | CHK-A2-021 ex7_messages_identical | CHK-A2-004, CHK-A2-005 | 3 | yes | run, `raw/I136.log` |
| I144 | CHK-A2-096 | `cebdde93ef4898d0` | yes | CHK-A2-096 rc_pointers | CHK-A2-150 | 2 | yes | run, `raw/I144.log` |
| I144b | CHK-A2-096 | `a8d58d1779e4b159` | yes | CHK-A2-096 rc_pointers | CHK-A2-150 | 2 | yes | run, `raw/I144b.log` |
| I148 | CHK-A2-111 | `6572755233e0f742` | yes | CHK-A2-111 decisions_rows | - | 1 | yes | run, `raw/I148.log` |
| I148b | CHK-A2-111 | `9baa219025214f9b` | yes | CHK-A2-111 decisions_rows | - | 1 | yes | run, `raw/I148b.log` |
| I148c | CHK-A2-111 | `bbb59631c66458fa` | yes | CHK-A2-111 decisions_rows | - | 1 | yes | run, `raw/I148c.log` |
| X01 | CHK-A2-151 | `6e0dfd0dd3e7d0c0` | yes | CHK-A2-151 ex4_none_statement_p25 | CHK-A2-011 | 2 | yes | run, `raw/X01.log` |
| X02 | CHK-A2-147 | `8d93d852a9d62c90` | yes | CHK-A2-147 dispatch_v1_defined | - | 1 | yes | run, `raw/X02.log` |
| X03 | CHK-A2-145 | `a97050b627ed5d15` | yes | CHK-A2-145 preview_manifest_same_keys | - | 1 | yes | run, `raw/X03.log` |
| X04 | CHK-A2-145 | `e2391d2e01cc7114` | yes | CHK-A2-145 preview_manifest_same_keys | - | 1 | yes | run, `raw/X04.log` |
| X05 | CHK-A2-148 | `8dd4be9bc195916c` | yes | CHK-A2-148 render_pointer_dispatch_v1 | - | 1 | yes | run, `raw/X05.log` |
| X06 | CHK-A2-155 | `9c5f8865f7c7e7b0` | yes | CHK-A2-155 checklist_ra01c | - | 1 | yes | run, `raw/X06.log` |
| X07 | CHK-A2-156 | `34fd5c9c3bc336ed` | yes | CHK-A2-156 trace_r15_4_claim | - | 1 | yes | run, `raw/X07.log` |
| X08 | CHK-A2-146 | `8222be12b99aa231` | yes | CHK-A2-146 preview_run_spelling_no_collision | - | 1 | yes | run, `raw/X08.log` |
| X09 | CHK-A2-042 | `252ed2d137e2556c` | NO | - | CHK-A2-146 | 1 | yes | run, `raw/X09.log` |

Total 48 mutants run, 47 killed by the named check, 1 survived.
## Notes on specific rows

- X09 (not a survivor of the contract text, a survivor of CHK-A2-042): replacing one of the two P2-2 `run request: (preview)` occurrences by `run request: preview` is killed by CHK-A2-146 only; CHK-A2-042 still passes because its literal remains present at the other site. Finding for the record: CHK-A2-042 alone does not detect a one-site respelling; CHK-A2-146 and CHK-A2-145 do.
- Exploratory X06 was first run as an extra item `F9` after F8; CHK-A2-155 passed (it counts only F3 to F8 items), so that variant survived. It was replaced by a duplicated `F8` item (seven F3 to F8 items), which CHK-A2-155 kills. Only the second run is logged (`raw/X06.log`). The first form is a spec reading, not a defect: an F9 item is outside the six-item scope of I123.
- I129 first attempt (union of whole-file and per-line extraction) did not fail CHK-A2-160 because the union is a superset; the logged I129 and I132 runs replace the per-line loop by the whole-file extraction. Both are killed by CHK-A2-160 and by CHK-A2-000, and also by CHK-A2-022.
- I117c is killed by CHK-A2-149 on the presence precondition (`P2-3 generation bullet: missing literal ...`), as required.
- I113c is killed by the absence clause (`forbidden text present: always differ`), as required.
- I114, I114b, I114c are killed with the sentinel message `preview run token ... is a legal request ID spelling`; I114c also fails CHK-A2-111 (noted).
- I131 mutates the scratch `base/` copies of `execution-contract.md` and `authoring-contract.md`; the killing line CHK-A2-022 is present (CHK-A2-133, 021, 084, 160 also fail).
- I144 uses `, not a W field, so` replaced by `, not by W, so` (the pointer line holds `not a W field` twice, so the unqualified replace was refused as a count mismatch and the logged run uses the qualified form); I144b deletes `, not a W field`.
- Several mutants are also killed by neighbouring checks (see "Other FAIL"); in no row was the named check silent.

## Red-first table (checks added in this round lineage and the mutant that makes each fail)

| Check | Mutant row (killed) |
| --- | --- |
| CHK-A2-144 preview_manifest_run | I112 |
| CHK-A2-145 preview_manifest_same_keys | I113, I113b, I113c, X03, X04 |
| CHK-A2-146 preview_run_spelling_no_collision | I114, I114b, I114c, X08, X09 |
| CHK-A2-147 dispatch_v1_defined | I115, X02 |
| CHK-A2-148 render_pointer_dispatch_v1 | I116, X05 |
| CHK-A2-149 no_identifier_carries_data | I117, I117b, I117c |
| CHK-A2-150 generation_is_manifest_key | I118, I118b |
| CHK-A2-151 ex4_none_statement_p25 | I119, X01 |
| CHK-A2-152 ex4_none_statement_d143 | I120 |
| CHK-A2-153 a3_token_sentence_split | I121 |
| CHK-A2-154 load_pointer_help_order | I122 |
| CHK-A2-155 checklist_ra01c | I123, I123b, X06 |
| CHK-A2-156 trace_r15_4_claim | I124, X07 |
| CHK-A2-157 trace_r15_5_takeover | I125 |
| CHK-A2-158 lead_sentence_listed | I126, I126b |
| CHK-A2-159 a2_line_cite | I127 |
| CHK-A2-160 allowed_messages_extraction (guard) | I129, I131 |
| CHK-A2-111 decisions_rows (extended) | I128, I148, I148b, I148c |
| CHK-A2-022 no_new_message (repaired, both directions) | I130, I131 |
| CHK-A2-096 rc_pointers (extended) | I144, I144b |
| CHK-A2-043 manifest_fields (changed literal) | I112, I133 |
| CHK-A2-000 selftest allowed family | I132 |

Guards re-probed: I134 (CHK-A2-005), I135 (CHK-A2-100), I136 (CHK-A2-021): all killed by the named check.

## Skipped and not probed

- Rows I137 to I147, I149, I150 (evidence and workflow rows): by their definition not applied by the prober (no script observer); reviewer checks.
- Rows I01 to I111 of the R15-A2 plan: re-probed by the finisher on the final bytes (acceptance 7); no earlier PROBES.md exists to carry forward.
- Exploratory budget: 9 of 20 used (X01 to X09); none of the other extra mutants were needed.
- Not probed, by limit: whether CHK-A2-000 self-test fixtures `allowed.good.txt` and `allowed.bad.txt` individually kill (I132 covers the pair through the check outcome only); acceptance 6 and 7 outputs (red/, green/ folders) are not mutation-testable and are left to the reviewer and finisher.
- Checkout state: no checkout file or build tree was edited; the only writes are this `probes/` folder. Scratch copies and the prober script live under the session scratchpad (`pw/`, `prober/run.py`); the warm cmake build `probe-warm` was not used because no compiled target is involved.
