# V5-02-R15-A2 review (execution contract, part 2)

Decision: **request-changes**. The contract text is close and mostly sound. All seven placeholders are filled, the
traceability, decisions and checklist rows are present, Part 1 changes are almost all on the amendment list, and the
three delegated choices have recommendations that I accept. The round cannot be approved for two reasons. First, the
agreed evidence is not there: there is no green checker run, no mutation probe, no redaction log and no final log. The
record's own Acceptance table and Final results say this. Second, I found four defects in the normative text. Two are
exactness gaps under criterion 4 (the preview manifest's `run` field, and the undefined `dispatch_v1` identifier). One
is a false citation that contradicts P2-1. One is a set of wording errors in normative sentences, including the
token-exposure rule. Fixing the findings needs evidence data (checker repair, red re-run, probes, final logs) and
contract text, so this is not a records-only change.

Scope: the uncommitted candidate on HEAD `cdcfe829c466bf6338815ff6f5b6ea90ffddc570`. That is the seven edited
documents, the untracked round record [V5-02-R15-A2](V5-02-R15-A2.md) and the evidence folder
`tests/native/v5-02-r15-a2-evidence/` (`plan/`, `base/`, `checks/`, `red/`, `README.md`). I ran no script under
`tests/native/`, in place or as a copy. I read the checker source but did not execute it.

## Candidate identity (workflow `identity.py`, run by me, verbatim)

| identity | value |
|---|---|
| HEAD | `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` |
| sha256(git diff --binary HEAD -- docs/vernacular/execution-contract.md docs/vernacular/command-contract.md docs/vernacular/render-contract.md docs/vernacular/load-command-contract.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/execution-design.md), tracked only | `ed557b11935786b22ad1e8844a5099d6b0f49b26ee7edec284bb13d69fa04ccf` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/execution-contract.md docs/vernacular/command-contract.md docs/vernacular/render-contract.md docs/vernacular/load-command-contract.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/execution-design.md, evidence excluded | `ed557b11935786b22ad1e8844a5099d6b0f49b26ee7edec284bb13d69fa04ccf` |

| file | state | sha256 |
|---|---|---|
| `docs/vernacular/command-contract.md` | modified | `34ae9d4010f8e13a4183ce452d18395be9eb7f0f95d54eb3bc7aad34d48d4e8a` |
| `docs/vernacular/execution-contract.md` | modified | `17f9f4f3ad83b006c63f0444e4d4ab45ac36a5083bfe301268fc03f69fb915cd` |
| `docs/vernacular/execution-design.md` | modified | `1a5134a0815bb6a18bcb069b96eed94bd28ce247db88b5e4cf54c798c612cb87` |
| `docs/vernacular/external-profile-contract.md` | modified | `86fcbefbf070293b4da7f7efbab1dd7b6065b6d02abc0df7e4bcfa5199f0c493` |
| `docs/vernacular/ledger-contract.md` | modified | `46d8a0eef9f5a65822efbeea1a6e5b61dbd14f3b0adf3e0c01ade94112f4a84f` |
| `docs/vernacular/load-command-contract.md` | modified | `eab326e239d94b6fc3ca6585bafe0c27b9e276736d6bdbebb4cb986490a5201d` |
| `docs/vernacular/render-contract.md` | modified | `7324432984cb9886fe5cee4cf88f0e3aaad625fe23b86083227017ea38854dd5` |

This matches the record's Identity section byte for byte, so the reviewed text is the text the record describes.

## Findings

### V5-02-R15-A2-R1 (blocking, evidence): the agreed verification was not run

The round's "Agreed verification" requires a green checker run (V1), mutation probes with a positive control (V3),
a final link-check log (V4) and per-file redaction with a control (V5). It also requires the final logs to be written
last. `tests/native/v5-02-r15-a2-evidence/` holds only `base/`, `checks/`, `plan/`, `red/` and `README.md`. There is no
`probes/`, `green/` or `raw/`. The record's Acceptance table marks V1, V3 and V5 "Not met" and every probe-outcome cell
of the invariant table reads "not run (probes/ does not exist)". The review was dispatched before V1 to V5, although the
record says review "follows V1 to V5".

This matters for more than bookkeeping. The red run is behavioural (exit 1, `SUMMARY pass=7 fail=99`, no traceback, and
the FAIL set equals the 99 expectRed rows). But 77 of the 99 FAIL lines have one cause: a required heading occurs 0
times (`grep '^FAIL' red/red-run.txt | grep -c 'occurs 0 times'` gives 77). So the red run shows that the checks need the
new headings. It does not show that each check detects its own invariant. Only the mutation probes of I01 to I111 can show
that each row's mutant is killed by the named check. Until `probes/PROBES.md` exists, with a baseline log, a non-empty
diff per mutant and the killing `FAIL` line, no invariant row is shown to be non-hollow.

Fix: run the prober stage, then the final-logs stage, exactly as Verification commands items 6 to 11 state. Keep every
earlier attempt as `<id>.attempt-<n>`.

### V5-02-R15-A2-R2 (blocking, evidence): checker defect CHK-A2-022 is still open

Open item 1 is accurate. `CHK_A2_022_no_new_message` (`checks/contract_checks.py:881-888`) builds its allowed message
set with `re.findall(r'`([^`]+)`', ctx.base(name))` over each whole base file. One stray backtick shifts the pairing
for the rest of the file. That is why the existing strings `Unknown selected job` and `Unknown selected role` (base
`execution-contract.md` Messages rows and `authoring-contract.md:506`) are reported as new. The contract is right. The
check is wrong, and a check that fails on correct text gives no evidence for I18.

Fix: repair the extraction, for example per line or per table cell, or from the Messages and EX-7 table cells only.
Have the repair reviewed as a checker change. Repeat the red run on the base snapshot with the repaired checker, keeping
`red-run.txt` as `red-run.attempt-1.txt`. Then run the I18 probe (`Role is not claimable`) and a probe that removes one
existing message from the allowed set, so that both directions are shown.

### V5-02-R15-A2-R3 (blocking, evidence): `red/RED.md` is the wrong red narrative

`red/RED.md` (21:07:37, after the checker red run at 20:55:14) is the ctest-based table. Every row is `not-registered`
with ctest exit 8, and its identity is the empty-diff hash `e3b0c442...` of `src` and `tests/native`. It contradicts
the checker-based red result. The checker-based narrative is `red/superseded/RED-pre-redcheck3.md`. The record works
around this (open item 3), but the task contract names `red/RED.md` as the red record, and a reader of the evidence
folder gets the wrong answer.

Fix: move the ctest table into `red/superseded/` under a new name and restore the checker-based narrative as
`red/RED.md`. Update it after the R2 re-run. Delete nothing. Also state plainly in `RED.md` that the pre-edit snapshot is
pinned by the base commit ID plus the twenty `cmp` exits in `red/tree-verify.txt`. The `e3b0c442...` value in
`red/snapshot-identity.txt` is the hash of an empty diff, not of the snapshot, so it must not be presented as the
snapshot hash.

### V5-02-R15-A2-R4 (blocking, contract): the preview manifest has no defined `run` value

`preview_v1` returns `manifest` (P2-1, "Verb behaviour and responses"), and the `preview` packet is built "the same
way" (P2-2). But the `dispatch_manifest_v1` table defines `run` only as `{request,kind}`, where `request` is the claim
or takeover request ID and `kind` is `claim` or `takeover`. A preview has no request and is neither kind. So the
preview manifest is not field-by-field exact (criterion 4), and R15-C would have to invent a value. The run section has
the same weakness in a milder form: preview writes `run request: none`, but `none` matches the request-key grammar
`[A-Za-z0-9_.:-]{1,128}`, so that line alone cannot tell a preview from a dispatch whose request ID is `none`. The
authority template still tells them apart.

Fix: state the preview value of the manifest `run` key exactly (or say that the key is absent in a preview manifest),
state whether `versions`, `generation` and `inputs` are the same as for a dispatch, and add a checker literal and a
probe for it. Optionally, choose a run-section spelling for preview that cannot collide with a legal request ID, as was
done for `--expect-job NONE`.

### V5-02-R15-A2-R5 (blocking, contract): `dispatch_v1` is used but never defined

`dispatch_v1` is added to the render interpretation-identifier list (render pointer, P2-3, P2-7) and is the last
`versions` entry of every manifest (P2-2). render-contract says identifiers "identify shipped semantics" and that "any
semantic change needs a new corresponding identifier and reviewed vectors". No sentence says which semantics
`dispatch_v1` names. Three places also say that the generation "is carried by `dispatch_v1` and `dispatch_manifest_v1`
(`generation`)": P2-3, P2-7 and the render pointer, and D120 says the same. That treats a version identifier as a data
carrier. Only the manifest has a `generation` key.

Fix: add one normative sentence that defines `dispatch_v1`, for example as the dispatch projection rules of P2-2 (packet
mode, section order with `run` second, the two authority templates, the run-section bytes and the
`dispatch_manifest_v1` recipe). Reword the "carried by" sentences to say that the generation is the manifest's
`generation` key, and the authority statement's `{generation}`, and is not a W field. Keep the render pointer
consistent with this.

### V5-02-R15-A2-R6 (blocking, contract): false citation "EX-4 rejects `none` for a job"

P2-5 (format3 event summary paragraph) and decision row D143 both say that "`none` is a legal `id` per
`declarations.md:33` and EX-4 rejects `none` for a job". EX-4 says nothing of the kind. Its payload rule (base line
679) says only that `"none"` is not used to spell an absent job, because `none` is a legal job ID. Read as written, the
P2-5 sentence suggests that a job named `none` is refused, which contradicts P2-1 ("`none` is a legal job ID and stays
an ordinary job ID") and D104. The record's "Reviewer note on D143" says the contract text is correct and only the plan
is wrong. That is not so: the contract repeats the plan's wording in both places.

Fix: in P2-5 and D143, replace the clause with the EX-4 statement ("EX-4 does not spell an absent job `none`, because
`none` is a legal job ID"). Correct the record note.

### V5-02-R15-A2-R7 (blocking, minor, contract wording): normative sentences that are garbled or inaccurate

1. **A-3 result, line 540 (token exposure, a security-relevant rule).** The amended sentence reads "It is never shown
   in `status` other than its request lookup, in a history event expansion, in render output, in any packet, and in the
   claim and takeover results only in the field `run.token`, in logs or in error details." The inserted clause sits
   inside a "never shown in" list. Read literally, it says the token is never shown in the claim results. Split it into
   two sentences: a "never shown in" list (status other than lookup, event expansion, render output, any packet, logs,
   error details) and a separate sentence that the claim and takeover results carry it only in `run.token`.
2. **Load-command pointer.** "...`JOB_OR_NONE` included, and are: Nothing is shipped until the R15 integrated gate;
   ..." The colon introduces the help lines, but the not-shipped sentence comes between the colon and the code block.
   Move the not-shipped sentence after the block.
3. **Acceptance checklist F3 to F8.** Each item says it is "carried from the R15-A1 coverage gaps". F3 to F8 are RA01-C
   gaps (round record "Carried RA01-C coverage gaps"; D140). Say RA01-C.
4. **Traceability labels.** The new rows are labelled "R15-4 Claim and takeover precondition sets" and "R15-5 Lock-time
   mismatch mapping for claim and takeover". In the design, R15-4 is Claim (parameters, response `claim_v1`, retry bytes)
   and R15-5 is Takeover (current run id, no secret). Relabel them, and let R15-4 also cite the `claim_v1` response and
   the checklist retry-bytes item.
5. **Unlisted Part 1 edit.** The Pointer amendments lead sentence (base line 966) changed from "The amendments are
   insertions only" to "...insertions only, except the reworded F2 bullet of item 8". No A-n row lists this change. Add
   it to A-11 or a new row. Also, A-2 lists line 527, but the diff touches 518 and 528 only (527 is the first half of the
   528 sentence). Say "527 to 528" or "528".

## Advisories (not blocking)

- **A1 (D149 / A-13, reviewer attention).** Reusing `state_conflict` at two deterministic snapshot-time sites goes
  against the frozen EX-7 meaning ("changed between snapshot and commit ... retry with the same request ID"). A driver
  also cannot tell these refusals from the retryable lock-time `active_job` refusal, which uses the same code. The record
  states the limit, and no closed-set code fits better (`no_active_job` would describe a job that does exist, and
  `fenced` would need an invented `run_state`). I therefore do not disagree, and nothing halts for the operator. I
  recommend that the R15-B/C round adds a test that pins the deterministic behaviour, and that the operator-facing
  roadmap note mentions the limit.
- **A2 (D134, delegated option A).** I accept the recommendation. Operator answer D2/D3 option B gives the production
  binary no RA01 verbs, so admitting them under `with-profile` in R15 would only matter for the authoring bundle.
  However, the approved `authoring-contract.md` RA-9 closing paragraph now reads stale, and the amendment is by
  reference only. The next round that may edit that file should insert a pointer. Also, item 12 says the later round
  "decides which reads are exempt", while P2-6 already makes `orchestrator --ledger` an exempt read. Say that the later
  round decides exemption under `with-profile` only.
- **A3.** `dispatch_context_v1` is named as the internal projection input but has no field list. The retained packet
  bytes make this safe for replay, but R15-C must pin it in its own record.
- **A4.** The round record copies the Part 1 amendments table with links such as `[P2-1](#p2-1-verbs-and-services)`.
  In the record they point at anchors that do not exist (six broken links in my check). Repoint them to
  `../execution-contract.md#...`. This is a records-only edit.

## Delegated decisions

D104 (`--expect-job NONE`, metavariable `JOB_OR_NONE`), D134 (RA01 verbs stay embedded-only in R15) and D142
(`unsupported_ledger_version` for `status` and `orchestrator --ledger` on format1 and format2) each have a recorded
recommendation with rejected alternatives, in the record and in `plan/DECISIONS.md`. I agree with all three. Of the 13
reviewer-attention rows, I have no disagreement that would become an operator question. D149 is noted in A1. No choice
lacks a recommendation, so nothing halts.

## What I verified

- **Placeholders and headings.** `execution-contract.md` has no "Not yet written" text. The Part 2 heading is renamed,
  the lead-in says "Part 2 is normative", and `### P2-1` to `### P2-7` each occur once, in order (lines 896, 1027, 1088,
  1101, 1128, 1145, 1153).
- **Part 1.** `git diff -U0 HEAD` hunks in Part 1 are lines 1, 6 to 8, 518, 528, 540, 594, 645, 702, 724 and 730, the four
  Messages cells and the Pointer amendments lead sentence. All but the last are covered by A-1 to A-13 (R7 item 5). The
  SQL block, EX-7 and EX-8 tables and rows D7 to D103 and OP1 to OP3 have no hunk.
- **Verbs against E1 and R15-2 to R15-11.** The option maps match the design verb table (`execution-design.md:393-397`),
  with the D104 spelling. `status` exclusions match E1. The `claim_v1` keys match R15-4, with only `previous_run` added
  for takeover. The `status_v1` keys match R15-7. The provenance shape matches R15-8. The token exposure matches R15-9
  (once R7 item 1 is fixed). `--event` matches R15-10. The orchestrator `live` content matches R15-11. Precondition
  members come only from the eight closed members. Codes are only E16 codes and existing codes. No message string is
  new. The EX-7 tables and the external-profile message table (`external-profile-contract.md:558` and its neighbours)
  are unchanged.
- **R16 to R19 verbs.** None has an option map, a service list or a precondition set. D150 defers the sets, and the help
  lines add only the five R15 verbs.
- **Pointers.** All six non-execution diffs are insertions only, except the one F2 bullet line in
  `execution-design.md`. Each pointer is labelled `Pointer (R15-A2):` and says nothing is shipped until the gate. The
  production help lines are identical in the command, external-profile and load pointers and in P2-7. The current root
  help in `src/cmd/planar/bootstrap.cpp:26-34` has the history continuation as its second history line, as the pointer
  says, and the RA01 lines match `src/cmd/planar/authoring.cpp:127-132`. Capacity 59 to 54 follows X-LIMIT "64
  commands ... including shipped" (`external-profile-contract.md:531`).
- **F2 and F3 to F8.** The F2 bullet is reworded in `execution-design.md` (one line), and nothing else changed there.
  F3 to F8 appear in the checklist as R15-B to R15-E obligations (wording in R7 item 3).
- **Invariant table.** All 110 rows (I01 to I111, without I93) match `plan/INVARIANTS.md` in invariant, mutant and test.
  The one difference is the declared I80 placeholder. Every acceptance criterion, F2 and the probe and redaction
  workflow rows have a row. Whether each mutant is killed is not shown (R1).
- **Redaction (my diagnostic run, not round evidence).** `python3 scripts/redact-local-paths.py check --repo-path
  <repo> --home <home> <file>` exits 0 for each of the seven documents, the round record and the evidence `README.md`.
  A scratch control file containing the repository path exits 1.
- **Links (my own anchor check, not round evidence).** Every relative link and anchor in the seven documents resolves.
  The two exceptions are pre-existing and are not links this round added: a link inside a quoted backtick string in
  Pointer amendments item 1, and an archival v4 file reference in `ledger-contract.md`. The round record has six broken
  anchors (A4).
- **Hash pins.** The record pins only the seven contract documents. It pins no other record, review, REPORT or the
  roadmap.

## Limits

- I did not run `contract_checks.py` or `link_check.py`, as copies or in place, so I give no figure for them. The
  record's `pass=105 fail=1` is the coordinator's diagnostic, not a log in the folder.
- I did not apply any mutant. Whether the invariant rows are hollow stays open until the probes are run (R1).
- The evidence folder is gitignored. `identity.py` gives no per-file hash for it, so the integrity of `base/` rests on
  the base commit and `red/tree-verify.txt`.
- No files were added under `tests/native/v5-02-r15-a2-review-evidence/`. Every command above is short and stated
  inline.
