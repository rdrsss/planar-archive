# V5-02-R15-A2 plan: scope map, decisions table and normative content (authoritative)

Search stage, revision 4 (2026-11-06; revisions 1 to 3 are preserved in `superseded-1/`, `superseded-2/` and `superseded-3/`). Read-only search of the checkout at HEAD
`cdcfe829` (working tree: only the untracked round record). Every citation is `file:line` as the file stands now. All line
numbers are located by text, not authoritative after edits.

Revision 2 closes the plan-review gaps G1 to G12: G1 W recipe forms follow execution-design EX-5 (D120, 4.3, 4.2.3); G2
closed `claim_conflict`/`fenced` details (D114, 4.1.3, 4.1.4); G3 status exclusions are the operator-accepted E1 table (D106);
G4 the snapshot carries the acknowledged W value (4.1.5); G5 format3 event summary and at-event state (4.5, D143); G6 native
provenance always shown (4.4, 4.5, D123); G7 exempt reads on format1/2 (D142); G8 amendment list A-1 to A-12 (section 5,
D138); G9 pointers are inserted paragraphs only (D131, 4.7); G10 external-profile consequences (D145, 4.7.6); G11 by-reference
amendments of RA-1 and RA-7 (D144, 4.8); G12 evidence plan (D141, section 6 and 7, INVARIANTS.md).

Revision 3 closes the plan-review gaps R3-1 to R3-11 and applies the delegated D134 answer: R3-1 counts restated (101
`CHK-A2-*` rows, 94 with expectRed true, 7 guards; the red run reads `SUMMARY pass=7 fail=94`); R3-2 the record auditor
row (old I93) is removed and the link row is limited to the seven edited docs; R3-3 one ten-file snapshot and a red run
against a scratch tree built only from `base/` (section 7); R3-4 per-build R15 bytes and counts (D148, 4.6, 4.7.1, 4.7.2);
R3-5 amendment A-13 for two snapshot-time `state_conflict` sites (D111, D114, D149, section 5); R3-6 provenance input
handling (D146, 4.4.1); R3-7 run section purpose strings pinned byte-exact (D147, 4.2.1); R3-8 traceability extends the
existing R15-2, R15-3, R15-9, E1, E2, E3, E5, E11 and E16 rows and adds the new ones (4.8); R3-9 D105, D106 and D122
retagged operator-decided; R3-10 probe positive control (section 7, row I106); R3-11 status `roles` lists retired roles with
`kind` from the structure record (D115) and the activate help metavariable is `JOB_OR_NONE` (D104). Delegated D134 (option A,
standing delegation of 2026-10-31) is applied: D134 is tagged delegated and the RA-9 closing paragraph is amended by
reference (D134, D144).

Revision 4 closes the plan-review gaps S1 to S16: S1 format3 event summaries use the empty string for an absent `job` or
`role` (only `generation_id` and `run_id` use `"none"`; D143, 4.5, I87); S2 precondition sets for refresh, checkpoint,
release, the two check commits and verdict are deferred to their R16 to R18 contract rounds, stated in P2-1 and in
amendment A-4 (D150, I107); S3 native enforcement of the fixed `expected` set, claimability and takeover run currency
(D151, 4.1.3, I108); S4 `status --role` and `preview` for a role unknown to the active job or retired and undeclared
(D152, 4.1.4, I109); S5 `history --request-id` on format3 is bounded at 16 MiB and decoded without Lua (D125, D126, 4.5,
I110); S6 the render-contract pointer and D120 state that repair enters W and that generation is carried by `dispatch_v1`
and `dispatch_manifest_v1` (4.3, 4.7.3, I67); S7 the final identity covers the seven edited docs, the three guarded docs and
this round's evidence data files only (section 7); S8 the snapshot guard and the link check use the recorded base commit
`cdcfe829c466bf6338815ff6f5b6ea90ffddc570` through `--base-commit`, never HEAD (I100, I79, section 7); S9 RA-9 items 1, 4
and 6 are amended by reference (D144, 4.8, I96); S10 decision tags aligned (D105, D106, D122, D148 bare
`operator-decided`; D123 `operator-decided`; D126 and D136 `new-engineering`; I74); S11 an `activate` `--expect-job JOB`
mismatch is `no_active_job`, and only `--expect-job NONE` with a job active and an unknown takeover run id are
`state_conflict` under A-13, with the driver-visible retry limit (D111, D149, A-13, I104); S12 `init` is "not bound" rather
than an exempt read (D108, I12); S13 the production help lines are in the load-command-contract pointer content (4.7.4,
I66); S14 `acceptance-matrix.md` and `v5-alignment.md` join the equality guard (I71); S15 P6 order fixed (green logs,
FIN.md, identity, redaction; section 7); S16 the section-ID reservation of `run` and `orchestrator_state` applies in every
build (D135, D145, D148, 4.7.6, I111). Counts restated: 106 `CHK-A2-*` rows, 99 with expectRed true, 7 guards; the red run
reads `SUMMARY pass=7 fail=99`, the green run `SUMMARY pass=106 fail=0`; 110 invariant rows (I01 to I111 with I93 removed).

Legend for the "Source" column: **contract-decided** (cite clause), **design-decided** (cite
`execution-design.md` or `remaining-steps-design.md` line, question or Operator decisions table), **NEW-delegated**
(a choice the placeholders leave open; recommended option recorded as `operator-decided (delegated 2026-10-31, option A)`
under the standing delegation named in the round record; the reviewer may overrule). No row is a NEW choice without a
recommendation, so nothing halts for the operator. Rows marked "reviewer attention" are the ones with the most product
weight; if the Opus reviewer disagrees with one, that row becomes an operator question.

## 1. Scope map (file:line)

Round record `docs/vernacular/rounds/V5-02-R15-A2.md:1-150`: objective lines 12-50, owned files 83-100, acceptance 107-130,
verification 132-150.

`docs/vernacular/execution-contract.md` (1089 lines at HEAD):

| Lines | Content | Plan |
| --- | --- | --- |
| 1-9 | Title `# Execution contract v1 (part 1)` and Status paragraph ("Part 1 holds ... the Part 2 placeholders") | amend (A-1) |
| 21-41 | Common rules 1-6 | untouched |
| 43-480 | EX-1 (SQL block 70-390, invariants, cutover) | untouched, byte-exact |
| 482-542 | EX-2 and Run token (R15-9); line 518 forward reference "The `status` verb is defined in Part 2"; lines 520-528 `history --request-id` on format3 with "deferred to the placeholder `Part 2 (R15-A2): history --event selector`" (line 527-528); line 540 "in any packet other than the claim or takeover packet" | amend (A-2 pointer wording, A-3 token sentence) |
| 544-646 | EX-3; line 594 "defined by R15-A2 in Part 2"; line 646 "belong to R15-A2" | pointer wording (A-4) |
| 647-713 | EX-4; line 702 "Its shape is in the Part 2 placeholders" | pointer wording (A-5) |
| 714-725 | EX-5; line 724 "not written in this part ... Part 2 placeholder" | pointer wording (A-6) |
| 726-733 | EX-6; line 730 "Which verbs are exempt reads is a Part 2 clause" | pointer wording (A-7) |
| 734-788 | EX-7 (nine codes, nineteen messages) | byte-identical |
| 789-805 | EX-8 (ten bounds) | byte-identical |
| 806-890 | State machines M1-M41, N1-N11 | byte-identical |
| 892-922 | `## Part 2 placeholders (R15-A2)` and seven `###` placeholders | replaced by P2-1 to P2-7 |
| 924-962 | Messages | cell edits only (A-8 revision_conflict site, A-9 reference field values, A-10 schema provenance field, A-13 state_conflict refusal-site cell); no new message |
| 964-974 | Pointer amendments; item 7 "Not made by this round" | replace item 7 (A-11), add items 8 to 12 (five contracts, execution-design F2 bullet, by-reference amendments of RA-1 and RA-7, files not edited) |
| 976-1011 | Traceability | extend; any existing row that links `#part-2-placeholders-r15-a2` is re-pointed (A-12) |
| 1013-1025 | Acceptance checklist | extend (F3 to F8 obligations, A29, A34, new items) |
| 1027-1089 | Decisions table (D7 to D103, OP1 to OP3) | append D104 to D152; no existing row is edited |

Other owned files (pointer sites):

| File | Site (lines at HEAD) | Edit |
| --- | --- | --- |
| `command-contract.md` | status pointers 12-29; ABI table 55-60; option table 74-81; native checks paragraph 83-91; sole `--help` rule 104-106; opaque slots 192-196; services table 211-216; Load admission 262-275 | one inserted pointer paragraph, marked not shipped until the R15 integrated gate; no existing row, sentence or line is edited (section 4.7) |
| `render-contract.md` | identifiers 248-253; W recipe 294-412 (work fingerprint paragraph 402-412); packet schema 414-419 (`mode:"preview"` at 416); manifest table 420-436 | three inserted pointer paragraphs; no existing line edited |
| `load-command-contract.md` | pointer lines 17-19; service table 123-127; services paragraph 114-121 | one inserted pointer paragraph (services statement inside it); no existing row edited |
| `ledger-contract.md` | command-surface table 436-449; paragraph 451 (format3 request lookup); data selectors paragraph 456-461 | one inserted pointer paragraph after the format3 request-lookup paragraph (the `--event` row text and the selector sentence live inside it); the command-surface table is not edited |
| `external-profile-contract.md` | pointer lines 41-46; X-CLI 48-60; help 137-141; section-ID union 366-376 (list at 377-380); X-PROJ shipped-owned 279-281; format paragraph 655-657 | one inserted pointer paragraph after the RA01-A pointer (family list, section IDs, consequence statement); no message table or X-LIMIT row edited |
| `execution-design.md` | F2 bullet at line 527 ("- Hard to observe: none beyond capture races, which reuse the render capture receipt tests.") | reword only that bullet |

Bind inputs read: `execution-design.md` 62-336 (EX-1 to EX-8), 381-420 (verb surface), 549-660 (R15-2 to R15-11), 668-700
(strategy and split), 1367-1395 (E1 to E17); `remaining-steps-design.md` Operator decisions (Q1 to Q18, lines 740-765);
`authoring-contract.md` RA-1 (57-115), RA-7 (393-433), RA-9 (442-483); `x07-addendum.md` 1.2 and 1.3 (55-80);
`declarations.md:33` (id grammar `[a-z][a-z0-9_]{0,63}`), `declarations.md:284-286` (preferences);
`src/cmd/planar/bootstrap.cpp:25-37` (the exact current default root help bytes);
`src/stdlib/planar/selection.lua:95-97` (the `reference` field values `selection.job` and `selection.role`).

Facts found while reading that the contract text must respect:

- F-A. `declarations.md:33` defines `id` as `[a-z][a-z0-9_]{0,63}`, which `none` matches, and EX-4 (the Event payload
  paragraph, execution-contract.md:679) already says `none` is a legal job ID. The Part 2 placeholder at line 898 therefore
  says something true ("the spelling of the no-active-job expectation ... since none is a legal job ID") and needs no
  correction. The false wording ("since `none` is not a legal job ID") is only in the round record
  (`docs/vernacular/rounds/V5-02-R15-A2.md:20`, dispatch item 1); package P5 corrects the record. The `--expect-job`
  spelling must be a value no ID can have (decision D104).
- F-B. R15-9 says packets never show the token while EX-2 line 540 says "in any packet other than the claim or takeover
  packet". The token lives only in the `run.token` field of the claim response, never in packet Markdown or manifest
  (the packet is a retained record that `history --event` expands). Recorded as amendment A-3 (D118).
- F-C. The manifest and packet are rendered by Lua before the lock, but the run id is allocated `MAX+1` under the lock
  (Common rule 3). A packet that names the numeric run id would force a spurious `state_conflict` between two claims of
  different roles. The packet and manifest therefore name the run by its claim request ID (D112).
- F-D. `orchestrator` has options `--workspace!`, `--registry!`, `--job!` as required in `authoring-contract.md:94-100`.
  The ledger form `--ledger` makes all four options optional at the descriptor level, with an all-or-none presence rule
  enforced natively as `usage` (D133).
- F-E. The messages in the closed external-profile table are untouched. The plan needs no new message string and no new
  code (E16 set only: `no_active_job`, `claim_conflict`, `state_conflict`, `fenced`, `interpretation_mismatch` are the five
  R15 codes; `history_unavailable`, `drifted`, `check_in_progress`, `unsupported_policy` stay R16 to R18).

## 2. Decisions table

Numbering continues the contract's table (D103 and OP1 to OP3 are the last; D146 to D149 are added by revision 3, D150 to D152 by revision 4). Tag column uses the contract's closed tag set:
`contract-decided`, `operator-decided`, `new-engineering`, with the basis in the Source column (as existing rows D49, D50 and D78 do); only the rows to which the standing delegation of 2026-10-31 applies (halt questions with a recommendation: D104, D134, D142) read
`operator-decided (delegated 2026-10-31, option A)`.

| ID | Design choice | Source | Tag in contract | Reviewer attention |
| --- | --- | --- | --- | --- |
| D104 | `--expect-job` spelling: the uppercase value `NONE` means "no active job"; any other value is a job ID. `none` stays an ordinary job ID. The help metavariable is `JOB_OR_NONE` (root help `--expect-job JOB_OR_NONE`), not `ID_OR_NONE`, so that it does not share the metavariable of `--expect-revision`, whose spelling is the lowercase `none`; the spelling sentence stays in P2-1. Rejected: `none` (collides with the legal ID), a flag (flags cannot be required, external-profile-contract.md:266-267), empty string (scalars are nonempty). | NEW-delegated; E2 A; design R15-2 and verb table (execution-design.md:393) only say "`none` or ID" | operator-decided (delegated 2026-10-31, option A) | yes |
| D105 | Option maps of the five verbs (section 4.1 below); all scalars separate tokens, nonempty, not starting `--`; `--run-id` is a canonical positive decimal 1..9223372036854775807; `--actor` and `--request-id` use the load grammar. | design-decided (E1 A; verb table execution-design.md:393-405) | operator-decided | no |
| D106 | `status`: `--role` excludes `--request-id` and nothing else; `--request-id` may combine with `--workspace` and `--registry` (a request lookup together with a draft report is legal, as `status_v1` already carries `drafts?` and `request?` together; the status services row allows `read_request` and the draft services together); `--workspace` and `--registry` are both-or-neither. | design-decided (execution-design.md verb table, E1 status row, operator-accepted: "`--role` excludes `--request-id`" and nothing else) | operator-decided | no |
| D107 | Precondition sets (section 4.1.3): activate keeps D83; claim and takeover use `request_unused`, `active_revision`, `active_job`, `fact_version` for every role of the active job, `role_state(target)` and `generation_current(target, G)`; takeover omits any secret (`run_active` is not used). Reads have no preconditions. Only the eight closed members. The mismatch mapping follows Part 1 EX-3 Conflicts and the closed EX-7 details (D114). The sets of the reserved operations are deferred (D150) and native enforces the fixed sets (D151). | design-decided (EX-3 vocabulary, R15-4, R15-5); NEW-engineering for the exact sets (D34) | new-engineering | no |
| D108 | Exempt reads (D51): `status` without `--role`, `history`, `export`, `orchestrator --ledger`. Bound (not exempt): `activate`, `claim`, `takeover`, `preview`, `status --role`. `init` is classified `not bound (no active revision exists; not an execution verb)`: it creates a ledger, so it is a mutation (the read/mutation split of E1 stays correct: it is not listed among the reads) with no active revision to bind, and it is neither an exempt read nor bound. Rule: a verb is exempt exactly when it neither interprets the active revision nor computes `work_exec_v1` under current Lua. Exemption removes only the interpretation binding step; the format outcome of the exempt reads is D142. | design-decided (E3 A: reads that need no current Lua are exempt); NEW-engineering for the list | new-engineering | yes |
| D109 | Refusal order for a non-exempt execution verb: usage, format, early replay, interpretation binding, expectations, `no_active_job`, `reference` (unknown job then role), state refusals, provenance input (claim and takeover with `--provenance`, D146), then commit preconditions. The order for exempt reads is D142. | EX-3 step 1 and EX-6 (contract-decided) for the first four; NEW-engineering for the rest | new-engineering | no |
| D110 | `reference` field values at execution sites are `selection.job` and `selection.role` (the existing values at `selection.lua:95,97`); `schema` for a malformed provenance file has field `provenance` followed by the failing leaf path; no new code or message. | contract-decided for codes (Messages table); NEW-engineering for field values | new-engineering | no |
| D111 | `activate` expectations: `--expect-revision` mismatch is `revision_conflict` with `{expected,actual}` (existing code and message, reused where the meaning fits); `--expect-job` naming a job ID that is not the active job (or when no job is active) is `no_active_job`, because EX-7 freezes that code as "The ledger has no active job, or the named job is not the active job"; `--expect-job NONE` while a job is active is `state_conflict` at the snapshot, before the lock (a site added to the `state_conflict` Messages cell by amendment A-13, D149); the Messages row for `revision_conflict` names activate as a second site (recorded as amendment A-8 because R15-A1 I66 scoped that cell to the load compare-and-swap). | design-decided (R15-2 preconditions); NEW-engineering for the codes | new-engineering | no |
| D112 | The dispatch packet and manifest name the run by its claim or takeover request ID, never by the numeric run id, token or secret, because the run id is allocated under the lock (Common rule 3) and a predicted id would cause spurious `state_conflict` between claims of different roles. The numeric run id appears in the claim response and in later reads. | NEW-engineering (derived from Common rule 3 and the lost-update guard A-claims) | new-engineering | yes |
| D113 | Service protocols (section 4.1.5): `read_execution({})`, `read_request({request_id})`, `read_provenance({provenance})`, `open_workspace`, `capture_drafts({route})`, `commit_execution({change})`, and for format3 load the order `open_workspace`, `capture_load`, `read_execution({})`, `accept_load({bundle,semantic,execution})`. `status` calls `read_execution`, then `read_request` (only with `--request-id`), then `open_workspace` and `capture_drafts` (only with `--workspace` and `--registry`). Each callable once, in order; family-scoped. | design-decided for `read_execution({})` and `accept_load(... execution:CHANGE)` (R15-3, execution-design.md:565-568); NEW-engineering for the rest | new-engineering | no |
| D114 | Role-state refusals use only the closed EX-7 details (`claim_conflict` `{state}` is one of `blocked`, `running`, `done`, `retired`; `fenced` `{run_state}` is one of `active`, `released`, `interrupted`, `completed`, `fenced`). `claim`: role not `ready`/`stale` at the snapshot is `claim_conflict` with its state. `takeover`: a `--run-id` that names no run of that job and role is `state_conflict` at the snapshot, before the lock (no run state exists, so none is invented; a site added to the Messages cell by amendment A-13, D149; a same-ID retry cannot succeed for an unknown run id, recorded as a driver-visible limit in P2-1); a run id that is not the role's current active run, including when the role is not `running` (no run is current), is `fenced` with `{run_state}` of the named run; `claim_conflict` is never used by takeover. Under the lock: for `claim`, a changed `role_state(target)` is `claim_conflict` only when the locked state shows the role taken by another run (`running`) or in another closed-set state (`blocked`, `done`, `retired`), else `state_conflict`; for `takeover`, a named run that is no longer current is `fenced` with its locked `run_state`, else `state_conflict`. No new message. | design-decided (R15-5 requires the current run id; Part 1 EX-3 Conflicts and EX-7 details); NEW-engineering for the mapping | new-engineering | yes |
| D115 | `status` shape (section 4.1.4): full R15-7 field list; ids as decimal strings; `active_job` is the empty string when none; `generation` is the ordinal as a decimal string; `roles` lists every `role_state` row of the active job, including `retired` roles that the active revision no longer declares, sorted by role ID; `kind` is read from the `role_structure_v1` structure record of the role's current generation (`snapshot.roles[].generation.structure`), never from the interpretation, so a retired role keeps its kind; `role_detail` and `drafts` per R15-7 with exact keys; "role exists in the job" means declared by the active revision's recorded interpretation for the active job, and for a role that `status_v1` lists as `retired` but the active revision no longer declares `role_detail.current` is `none` (D152). | design-decided (R15-7); NEW-engineering for exact keys | new-engineering | no |
| D116 | Drafts: `status --workspace --registry` uses `open_workspace` and a status-only service `capture_drafts`; declaration-content errors come back as a value `outcome:"rejected"` so A20 can report `invalid`; capture instability, I/O, busy, resource errors fail the command. | design-decided (R15-7, A20); NEW-engineering for the service | new-engineering | no |
| D117 | Dispatch packet: `P.mode:"dispatch"` for both claim/takeover packets and the `preview` verb; two fixed authority templates (dispatched, preview); `run` section mandatory, second in order after `authority`; legal next actions by role kind with the purpose strings pinned byte-exact in 4.2.1 (D147); shipped `worker` and `reviewer` projections only, no new projection ID, no external projection for dispatch. | design-decided (R15-6); NEW-engineering for placement, templates and the external exclusion | new-engineering | yes |
| D118 | Run token appears only in `run.token` of claim and takeover responses (and their `status --request-id` and `history --request-id` lookups); never in a packet, manifest, `--event` expansion, status other than lookup, preview or log. Wording of EX-2 line 540 is clarified. | design-decided (R15-9, E11 A) | operator-decided | no |
| D119 | `dispatch_manifest_v1` exact field list (section 4.2.3); versions entry `dispatch_v1`; records referenced by ID (packet file record, manifest value record codec `dispatch_manifest_v1`, W value record codec `work_exec_v1`); bound `manifest_bytes` 16777216. | design-decided (R15-6 field list; EX-8) ; NEW-engineering for exact value forms | new-engineering | no |
| D120 | `work_exec_v1` is the render W fields (render-contract.md:331-338) with the changes of the execution-design.md EX-5 bullet "Execution W is a new versioned recipe `work_exec_v1`": `base` is the declared form `{status:"declared",root,label}`; `start_facets` take their semantic from the current publication (the word of the design; "current" is defined by R17, and no publication exists in R15); `versions` gains `work_exec_v1` (seven entries); and the new field `repair` is a decimal string (`"0"` before any repair), kept per structural lineage (Part 1 EX-5 Lineage). Digest `H("work_exec_v1", W)`. render-contract.md says "no repair revision or generation is fabricated. Later execution must add those through a versioned recipe": `repair` enters W through `work_exec_v1`, while the generation is carried by `dispatch_v1` and `dispatch_manifest_v1` (`generation`), not by W, so a takeover (new generation, same W) never moves W (execution-design EX-5 bullet, operator E5 A). | design-decided (execution-design.md EX-5 bullet, operator E5 A, R15-6; Part 1 EX-5 Lineage) | operator-decided | no |
| D121 | `driver_provenance_v1` field list, leaf rule (string 1..1024 bytes or exactly `{"status":"unknown"}`), bounds from EX-8, stored value record codec `driver_provenance_v1` wrapping `authority:"driver_assertion_unverified"`; absent file stores nothing (`runs.provenance_id` NULL) and every field reads unknown. | design-decided (R15-8, E6 A, EX-8 row Provenance file); NEW-engineering for the record wrapper | new-engineering | no |
| D122 | Attachment points: claim and takeover (live in R15); release and check are fields-only, event reference key `provenance` pinned by R16 and R17. | design-decided (R15-8; basis: operator E6 A); scope boundary in round record | operator-decided | no |
| D123 | Native per-run provenance (ledger identity, revision, source digest, job, role, generation, base label, W and render digests, requested preferences) is derived from rows and always reported by the `history --event` expansion of `run_claimed` and `run_taken_over` as `provenance.native`, whether or not a driver file was given; it is not stored again. The driver part of the expansion is the stored document or, without one, the all-unknown form (D121). | design-decided (R15-8, marked NEW (operator: E6), basis E6 A: Planar records native provenance per run; a missing file records every field unknown) | operator-decided | no |
| D124 | `history --event ID`: decimal selector; excludes `--revision`, `--request-id`, `--at-event`, `--after-event`, `--limit`; valid on every ledger format; `data` selector `event`. | design-decided (R15-10, A29); NEW-engineering for exclusivity | new-engineering | no |
| D125 | `--event` expansion: `{selector:"event",event,transitions,execution?}`; records decoded natively without Lua; whole response at most 16777216 bytes with the existing `resource_limit` `{limit:"output_bytes",maximum:16777216}` refusal; the reader never selects `runs.secret`. | design-decided (R15-10, R15-9, ledger-contract.md:447, R14-E2) | contract-decided | no |
| D126 | `history --request-id` on format3: per-operation result decoded: `activate_v1` and `claim_v1` exact keys, packet and manifest assembled natively without Lua from `attachment_id` and `dispatches.manifest_id`; the whole stdout is bounded at 16777216 bytes by the existing `resource_limit` `{limit:"output_bytes",maximum:16777216}` refusal with no truncation (the packet is up to 1 MiB and the manifest up to 16 MiB, so the result can exceed the ceiling); no `disposition`; format1/2 unchanged. | design-decided (basis OP2 A, EX-2 packet responses); NEW-engineering for the exact keys (the standing delegation covers halt questions only, not these keys) | new-engineering | no |
| D127 | `status --request-id` response: the base status plus `request:{request_id,operation,outcome,result,disposition?}`; `disposition` only for claim and takeover (EX-2); unknown id `request_not_found`. | design-decided (R15-7, EX-2) | contract-decided | no |
| D128 | Orchestrator ledger form: option set `--ledger` alone; exactly one form; no active job gives `no_active_job`; on a format1 or format2 ledger `unsupported_ledger_version` (D142); context gains key `live` only in this form (database-free bytes unchanged apart from the action labels flipping). The orchestrator, its ledger form, the capability `execution_read_v1` on it and the label flips exist only in builds that compile the orchestrator, the authoring bundle (the production binary has no authoring path, D148). | design-decided (R15-11, RA-1 "R15 later adds a ledger form"); NEW-engineering for the no-job refusal | new-engineering | yes |
| D129 | `live` content: `{ledger,revision,last_event,activation,roles,runs,blockers}`; `runs` are active runs `{id,role,generation,request_id}` with no token or secret; `blockers` is always `[]` in R15 (R18 defines rows). New mandatory section `orchestrator_state` in the ledger form only. | design-decided (R15-11); NEW-engineering for keys and section | new-engineering | no |
| D130 | Action labels: after R15 the `future` list is `refresh`, `checkpoint`, `release`, `recover`, `check`, `check-recover`, `verdict`; R16 flips the first four (and `preview --since`), R17 flips `check` and `check-recover`, R18 flips `verdict`; `available` still means "in the sealed catalog". The labels exist only where the orchestrator is compiled (D148). | design-decided (RA-7 future list; R16-7, R17-8, R18-8, R15-11) | contract-decided | no |
| D131 | Pointer amendments in the five other contracts are inserted pointer paragraphs only, each labelled "Pointer (R15-A2)" and stating that nothing is shipped until the R15 integrated gate; no current-state row, table or sentence of those contracts is edited, following the RA01-A pattern (the only existing line edited outside execution-contract.md is the `execution-design.md` F2 bullet). Every pointer says it narrows no approved clause. | contract-decided (round record scope; RA01-A pointer pattern) | contract-decided | no |
| D132 | Fixed root help after R15, per build (exact bytes, section 4.7.2), described in an inserted pointer (the current bytes stay in force until the R15 integrated gate). Production: the `bootstrap.cpp` bytes with the history continuation line gaining ` \| --event ID` and five lines (activate with `--expect-job JOB_OR_NONE`, status as one line, preview, claim, takeover) immediately before the final `planar --help\n`; no orchestrator line. Authoring bundle: the production bytes with the six RA01 lines spliced immediately before `planar --help\n` (so after the five R15 lines) and the continuation line `  or --ledger PATH\n` directly after the `planar orchestrator` line. `with-profile` help bytes unchanged. | design-decided (execution-design.md:384-391 requires both contracts amended); NEW-engineering for the exact text | new-engineering | no |
| D133 | Catalog after R15, per build: the production build has ten mandatory rows (the current five plus the five R15 rows) and the authoring bundle has sixteen (the ten plus the six RA01 rows; the RA01 rows and the `orchestrator` verb exist only there, D148). `orchestrator` (authoring bundle only) has descriptor options all optional, presence rule native. Capabilities: activate `execution_commit_v1`; status `execution_read_v1` and `draft_capture_v1`; preview `execution_read_v1`; claim and takeover `execution_commit_v1` and `provenance_read_v1`; orchestrator (authoring bundle only) gains `execution_read_v1`. Handler IDs `shipped.<verb>.v1`; response kinds `activate_v1`, `status_v1`, `preview_v1`, `claim_v1` (shared by claim and takeover). | design-decided for shape (verb table, RA-1 pattern); NEW-engineering for names | new-engineering | no |
| D134 | X-CLI family list under `with-profile`: the closed factory widens from five to ten families (adds `activate`, `status`, `preview`, `claim`, `takeover`; admission rows and the `command_options` variant change are R15-B and R15-C work with an x07-addendum amendment, not made here). The six RA01 verbs stay embedded-only: this is option A of the delegated D134 question, accepted under the standing delegation of 2026-10-31, and it is recorded as a by-reference amendment of the closing paragraph of `authoring-contract.md` RA-9 and of design RA-9 (`execution-design.md:499`), listed in the Pointer amendments next to D144 (neither file is edited beyond the F2 bullet). The sentence "R15 widens the closed factory to admit these verbs under `with-profile` only by its own reviewed amendment" is superseded by: R15 does not admit them; a later round, provisionally named RA02 here (the roadmap assigns the real ID after the R15 integrated gate), admits them under `with-profile` by its own reviewed amendment, decides which reads are exempt, and carries the shape-catalog narrowing-risk row (`remaining-steps-design.md:493-494`) in its invariant table. `orchestrator --ledger` is an exempt embedded read. The consequences for external profiles are stated in the pointer (D145). | design-decided (E3 A: "new shipped families join the closed factory", execution-design.md:281-283); delegated option A of the D134 question (operator, standing delegation 2026-10-31); an authority choice that departs from the RA-9 closing sentence by recorded amendment | operator-decided (delegated 2026-10-31, option A) | yes |
| D135 | Reserved built-in section IDs gain `run` (dispatch packets) and `orchestrator_state` (ledger-form orchestrator). The reservation of both section IDs applies in every build, production included, even though `orchestrator_state` is rendered only in the authoring bundle: `runtime.cpp` reserves the RA01 `orchestrator_*` section IDs in every build today, so R15-D does not guess. A profile registering either is valid today and refused after R15 (same consequence statement as RA-9 item 6). | design-decided (R15-6 adds `run`); NEW-engineering for `orchestrator_state` | new-engineering | no |
| D136 | Load services table, format3 row: services in order `open_workspace`, `capture_load`, `read_execution({})`, `accept_load({bundle,semantic,execution})`; format1 and format2 keep three services and exact keys `{bundle,semantic}`. | NEW (engineering) per R15-3 (service shape), design-decided order | new-engineering | no |
| D137 | `execution_change_v1` and `execution_snapshot_v1` exact key lists (section 4.1.5), expectations as eight-member objects; run secrets never in the snapshot; the snapshot carries the decoded acknowledged W value (`work`) of each running role's dispatch because `status --role` needs it for `changed` and `explain` (Part 1 EX-3 step 2). | design-decided (EX-3 steps 2 and 4); NEW-engineering for keys | new-engineering | no |
| D138 | Part 1 amendments A-1 to A-13 (section 5) are wording, pointer and Messages-cell updates that Part 2 requires. A-3 tightens the token-exposure rule of EX-2 line 540 (no packet carries the token; claim and takeover results carry it only as `run.token`); A-13 widens the refusal-site cell of `state_conflict` in the Messages table to two snapshot-time sites (D149); A-8 widens the scope of the `revision_conflict` Messages cell that R15-A1 I66 recorded as the load compare-and-swap only. Every other amendment re-points a reference or adds a listed item. No other semantics of Part 1 change. | contract-decided (round record: scoped, recorded, reasoned) | contract-decided | no |
| D139 | F2 carried gap: the execution-design line 527 bullet is reworded to say that the RA01 authoring and observe captures are covered by the named tests, and the receipt-reuse claim is limited to what render and load capture paths exercise. | contract-decided (RA01-C row C-15q, `rounds/V5-02-RA01-C.md:102,213`) | contract-decided | no |
| D140 | F3 to F8 stay carried as test-only obligations in the acceptance checklist (named for R15-B to R15-E). | contract-decided (round record "Carried RA01-C coverage gaps") | contract-decided | no |
| D141 | Evidence for this docs round (checker, base snapshot from the recorded base commit `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` via `git show`, never HEAD, red run on a scratch tree built only from the snapshot, prober baseline and per-mutant logs in `probes/` owned by the prober stage (unmutated baseline first; each kill log names the killing check), link check, redaction with control); hashes only from the workflow `identity.py`, pasted verbatim; no record or evidence auditor; no hashes of other records pinned. | contract-decided (round record "Agreed verification") | contract-decided | no |
| D142 | Exempt reads and ledger format. `status` in every form and `orchestrator --ledger` read execution tables that exist only in format3, so on a format1 or format2 ledger they are refused `unsupported_ledger_version` after `usage` (existing code and message; no replay, no binding step except `status --role`, which is bound). `history` and `export` keep every format (`history --event` is valid on all); `init` is not bound and not an exempt read (D108). `preview` (non-exempt) follows D109, with the format refusal second. Refusal order for exempt reads: usage, format, (interpretation binding for `status --role` only), the verb's own refusals. | NEW-delegated (Common rule 1 lists only the mutations; the exempt reads need an explicit outcome) | operator-decided (delegated 2026-10-31, option A) | yes |
| D143 | Format3 history shapes (R15-10): the event summary gains the typed reference columns `job`, `role`, `generation_id`, `run_id` beside payload v2 (`job` and `role` are IDs or the empty string, consistent with payload v2 `active_job`; `generation_id` and `run_id` are decimal strings or `"none"`; `none` is a legal `id` per declarations.md:33 and EX-4 rejects it for a job, so an absent job or role is never spelled `none`); `--at-event` `state` on format3 carries role states and active runs of the active job as of that event, replayed from rows without Lua (EX-1 invariant 19). Format1 and format2 shapes are unchanged. | design-decided (R15-10; ledger-contract.md R14-E2 pins "absent IDs use none" for numeric database IDs only); NEW-engineering for key names | new-engineering | no |
| D144 | By-reference amendments of `authoring-contract.md` are recorded in the Pointer amendments section of the execution contract and the file is not edited: RA-9 closing paragraph (D134: R15 does not admit the six RA01 verbs under `with-profile`; a later round does), RA-1 (the orchestrator options become optional with a native all-or-none rule and the ledger form, authority: RA-1 "R15 later adds a ledger form by its own contract amendment") RA-7 (section list gains `orchestrator_state`; "It states no execution state" applies to the database-free form only) and RA-9 items 1, 4 and 6 (item 1: the embedded probe holds the five X08 families, ten after R15; item 4: the with-profile catalog of five shipped rows, completeness rule five, embedded rule eleven, become ten, ten and sixteen (authoring bundle); item 6: with-profile catalog of five shipped commands, external capacity unchanged, becomes capacity 54), each "valid until the R15 integrated gate". | contract-decided (RA-1 sentence; the contract's own rule that each amendment is recorded) | contract-decided | no |
| D145 | External-profile consequences of the ten-family list are stated as "valid today, refused after R15" in the XP pointer, following RA-9 item 6: the five command and family names, the five shipped handler IDs and the section IDs `run` and `orchestrator_state` (reserved in every build, production included) leave the external namespace; external command capacity falls by five (X-LIMIT counts 64 commands including shipped; the table is unedited); the `history` probe signature gains `--event ID`; the with-profile completeness rule moves from five to ten. | design-decided (E3 A) with RA-9 item 6 as the model; NEW-engineering for the exact statement | new-engineering | no |
| D146 | Provenance input handling (4.4.1): `--provenance` is an explicit single-file capture that follows the `--diagnostic` rows of the authoring Messages table (`io` `Cannot resolve explicit input path` for a missing, unresolvable or unreadable file; `path_denied` `Explicit input file is a symbolic link`; `io` `Explicit input must be a regular file`; `capture_changed` `File changed during read`). Checks run in the order file bytes, JSON grammar, depth, settings count, string bytes, then shape and leaf-rule checks, so a string leaf over 1024 bytes is `resource_limit` with `provenance_string_bytes`, never `schema`. The `schema` field is `provenance` or `provenance.` plus a dotted key path. Provenance refusals are step (9) of the refusal order, after the role-state refusals; an early replay never reads the file. | NEW-engineering (authoring-contract.md Messages `--diagnostic` rows; EX-8 row Provenance file; D109) | new-engineering | yes |
| D147 | The `run` section body is pinned byte-exact (4.2.1): the `## run` framing follows the render rule "Every generated section begins `## <ID>`"; the five purpose strings for `refresh`, `checkpoint`, `check`, `release` and `verdict` are fixed by this contract and `dispatch.lua` reproduces them, because the retained packet bytes, `render_fp` and the claim retry bytes depend on them and the dispatch packet states the legal next actions as normative text. | NEW-engineering (render-contract.md leaves section headings and labels to the projection; this contract fixes the body) | new-engineering | no |
| D148 | Build variants of the R15 bytes and counts (4.6, 4.7.1, 4.7.2). Operator answer D2/D3 option B (2026-10-30): the production binary has no authoring path (`src/cmd/planar/CMakeLists.txt:267`: "The product targets never receive PLANAR_AUTHORING_BUNDLE"). Production: ten catalog rows; root help is the `bootstrap.cpp` bytes with the history continuation changed and the five R15 lines before `planar --help`; no `orchestrator` verb, so no `orchestrator --ledger`, ledger form, `orchestrator_state` section, orchestrator `execution_read_v1` capability or action-label flips. Authoring bundle: sixteen rows; the splice (`authoring::splice_root_help`) inserts the six RA01 lines immediately before `planar --help`, so the relative order is the five R15 lines, the six RA01 lines, the continuation `  or --ledger PATH`, then `planar --help`; the continuation is a seventh spliced line (R15-D work on `root_help_lines`). The section-ID reservation of `run` and `orchestrator_state` is not build-qualified: it applies in every build (D135). | design-decided (basis: operator answer D2/D3 option B, 2026-10-30); NEW-engineering for the R15 bytes and order | operator-decided | yes |
| D149 | Amendment A-13: the `state_conflict` refusal-site cell of the Messages table ("Any other failed precondition at commit") gains the two snapshot-time sites that D111 and D114 use before the lock: the `activate` `--expect-job NONE` mismatch while a job is active (refusal step 5) and a `takeover` `--run-id` that names no run of the job and role. An `activate` `--expect-job JOB` mismatch is not a site: it is `no_active_job`, which EX-7 freezes as "The ledger has no active job, or the named job is not the active job", so a driver that follows EX-7 does not retry it as a conflict. The frozen EX-7 meaning of `state_conflict` ("changed between snapshot and commit ... retry with the same request ID") is not edited, and no code or message is added. Driver-visible limit stated in P2-1: a same-ID retry cannot succeed for these two sites (the refusal is deterministic until the active job changes, or forever for an unknown run id); the code is reused because EX-7 is closed and no existing code fits better for them. | NEW-engineering (derived from D111, D114 and the closed EX-7 set) | new-engineering | yes |
| D150 | Precondition sets of the reserved operations are deferred: `refresh`, `checkpoint`, `release`, the `check` and `check-recover` commits and `verdict` have no request body, facts, response or event in Part 2, so P2-1 defines no member set for them; their sets are defined by the contract round of each operation (R16 for `refresh`, `checkpoint`, `release` and `recover`; R17 for the two check commits; R18 for `verdict`), using only the eight closed members. Part 1 EX-3 (lines 594 and 646) says the remaining members are defined by R15-A2; amendment A-4 re-points those sentences to P2-1 and records this deferral with its reason. Rejected: fields-only sets for reserved operations (they would pin members before the lifecycle and event shapes exist). | design-decided (EX-3 vocabulary; reserved verbs stay reserved, D140); NEW-engineering for the deferral | new-engineering | yes |
| D151 | Native enforcement placement: native validation at EX-3 step 5 requires `expected` to equal exactly the fixed set of the operation in 4.1.3 (same eight-member kinds, no omitted, extra or duplicate member) with every value equal to the snapshot read by the same command; it refuses a `claim` change whose target before-state in the snapshot is not `ready` or `stale`, and a `takeover` change whose named run is not the target's active run. A handler that violates this is refused with code `internal` (its existing message in the Messages table; no new message) because a correct handler has already returned `claim_conflict`, `fenced` or `state_conflict` before building the change. Lua supplies `expected`; native enforces the set. | design-decided (EX-3 step 5: native verifies the given preconditions); NEW-engineering for the exact-set rule | new-engineering | yes |
| D152 | `status --role R` and `preview --role R` reference behaviour: with an active job, a role not declared by the active revision's recorded interpretation for the active job and not listed in `role_state` is `reference` (message `Unknown selected role`, field `selection.role`); a role listed with state `retired` that the active revision no longer declares has no current W: `status --role` returns `role_detail` with `current:"none"`, `changed:[]` and `explain:{work_or_semantic:[],output_vector:[],prerequisite:[]}`, while `preview --role` is `reference` with the same message and field. "Role exists in the job" always means declared by the active revision's recorded interpretation. | design-decided (R15-7, R15-6); NEW-engineering for the retired case | new-engineering | no |

NEW choices that are product or authority decisions without a recommendation: **none**. Items for the plan reviewer to
confirm because they carry the most weight (marked "yes" above): D104, D108, D112, D114, D117, D128, D134, D142, D146, D148, D149, D150, D151. If any
is rejected it becomes an operator question under Q18; the plan keeps the others stable because each is isolated to one
clause.

## 3. Cross-cutting rules for all packages

1. Contract text is normative English with exact literals; tables for field lists. No machine-local paths anywhere (no
   `/Users/`, no repository root). Links are relative and anchors resolve (GitHub slug rules; the Part 2 headings are
   fixed in section 4.0).
2. Do not rewrap, reformat or renumber any existing line outside a listed amendment. Do not touch the SQL block, the
   state-machine tables, EX-7 or EX-8 tables, the existing Messages tables' existing rows (only the listed row edits).
3. Error codes allowed in Part 2 text: `usage`, `schema`, `reference`, `capture_changed`, `resource_limit`, `io`,
   `internal`, `budget_exceeded`, `unsupported_ledger_version`, `request_conflict`, `request_not_found`, `revision_conflict`,
   `event_not_found`, `ledger_corrupt`, `unsupported_interpretation`, `ledger_busy`, `storage_full`, `path_denied`,
   `unsupported_version`, `lua_syntax`, `capability_denied`, `ownership`, `dependency_cycle`, `ledger_not_found`,
   `foreign_ledger` and the five R15 E16 codes `no_active_job`, `claim_conflict`, `state_conflict`, `fenced`,
   `interpretation_mismatch`. The R16 to R18 codes may be named only as reserved. No other code token.
4. No message string beyond the nineteen of EX-7 and the existing strings the contract already names. Part 2 text quotes
   no new message. The statement "No twentieth message exists" appears in P2-1.
5. Digest text form is `xxh3_128:` plus 32 lowercase hex digits (render-contract.md:272-275); the stored form is the 16
   raw bytes. Never write a digest value.
6. Tags: only `contract-decided`, `operator-decided`, `new-engineering` in the Decisions table.
7. Pointers in the five other contracts are inserted paragraphs labelled "Pointer (R15-A2)", each stating that nothing is
   shipped until the R15 integrated gate and that it narrows no approved clause. No row of a current-state table and no
   sentence of a current-state paragraph is edited there (RA01-A pattern). The one existing line edited outside
   `execution-contract.md` is the `execution-design.md` F2 bullet.
8. Hashes in any record or evidence file come only from the workflow `identity.py`, pasted verbatim.

## 4. Normative content (the text packages P2, P3 and P4 write)

### 4.0 Structure of the replaced region of execution-contract.md

Replace lines 892-922 (`## Part 2 placeholders (R15-A2)` through the last placeholder) with exactly these headings, each
once, in this order. The old `### Part 2 (R15-A2): ...` headings must not remain.

```text
## Part 2: execution verbs, projections and reads (R15-A2)
### P2-1 Verbs and services
### P2-2 Dispatch projection and dispatch_manifest_v1
### P2-3 work_exec_v1
### P2-4 Provenance (driver_provenance_v1)
### P2-5 History event selector and request results
### P2-6 Orchestrator live form
### P2-7 Command, render, load, ledger and external-profile pointers
```

Anchors: `#part-2-execution-verbs-projections-and-reads-r15-a2`, `#p2-1-verbs-and-services`,
`#p2-2-dispatch-projection-and-dispatch_manifest_v1`, `#p2-3-work_exec_v1`, `#p2-4-provenance-driver_provenance_v1`,
`#p2-5-history-event-selector-and-request-results`, `#p2-6-orchestrator-live-form`,
`#p2-7-command-render-load-ledger-and-external-profile-pointers`. All older references to
`#part-2-placeholders-r15-a2` (execution-contract.md lines 702, 724, 974 and the Traceability rows) must be re-pointed.

After `### P2-7` and before `## Messages` the contract gains the top-level section `## Part 1 amendments (R15-A2)` (anchor
`#part-1-amendments-r15-a2`), written by package P3, listing A-1 to A-13 of section 5 with location, change and reason. It
is not a `### P2-` heading and is not counted among the seven. Package P2 must not link to it (it does not exist until P3).

Lead-in text (replaces "They are not normative, and they define no behaviour in this part."): states that Part 2 is
normative, that it completes the verbs and reads of round R15, defines no R16 to R19 behaviour (`refresh`, `checkpoint`,
`release`, `recover`, `check`, `check-recover`, `verdict` stay reserved names, `--since` stays R16), that Part 1 is fixed
except the recorded amendments A-1 to A-13 (listed in the section `## Part 1 amendments (R15-A2)`), and that this part adds no code and no message. Also update the document title
to `# Execution contract v1 (parts 1 and 2)` and the Status paragraph (lines 3-9) to say Part 2 is filled by round
R15-A2 (amendment A-1).

### 4.1 P2-1 Verbs and services

#### 4.1.1 Common verb rules

- One command per verb. Reads are separate from mutations. Mutation verbs (`activate`, `claim`, `takeover`) require
  `--request-id` and `--actor` with the load grammar of `load-command-contract.md` and the ledger contract (request keys
  1 to 128 bytes of `[A-Za-z0-9_.:-]`; actor nonempty text). Reads (`status`, `preview`) write nothing and have no
  request framing.
- All options are separate tokens (`--name value`), scalar values nonempty and not starting `--`, no defaults, aliases,
  positionals, repeats or short flags. A flag cannot be required.
- `--ledger`, `--workspace`, `--registry` and `--provenance` are opaque native host-path slots; no host path crosses to
  Lua. `--job`, `--role`, `--request-id`, `--actor`, `--expect-revision`, `--expect-job` are text. `--run-id` is a
  canonical positive decimal 1..9223372036854775807 crossing into Lua as a canonical decimal string.
- Each execution mutation uses the EX-3 commit protocol and `request_v3` (EX-2). Early replay precedes the
  interpretation binding of EX-6.
- No twentieth message: Part 2 adds no code and no message text. Every refusal named below uses a code of the E16 set or an
  existing code with its existing message ([Messages](#messages)).

#### 4.1.2 Option maps (table "Option maps"; `!` required)

| Verb | Kind | Required | Optional | Exclusions and rules | Response kind |
| --- | --- | --- | --- | --- | --- |
| `activate` | mutation | `--ledger`, `--job`, `--expect-revision`, `--expect-job`, `--request-id`, `--actor` | none | `--expect-revision` is `ID_OR_NONE` as load spells it (a canonical positive decimal revision ID or `none`); `--expect-job` is `NONE` or a job ID (D104; its help metavariable is `JOB_OR_NONE`) | `activate_v1` |
| `status` | read | `--ledger` | `--role`, `--request-id`, `--workspace`, `--registry` | `--role` excludes `--request-id` and nothing else (E1); `--request-id` may combine with `--workspace` and `--registry`; `--workspace` and `--registry` both or neither; any violation is `usage` | `status_v1` |
| `preview` | read | `--ledger`, `--role` | none | R16 adds `--since`; not in R15 | `preview_v1` |
| `claim` | mutation | `--ledger`, `--job`, `--role`, `--request-id`, `--actor` | `--provenance` | none | `claim_v1` |
| `takeover` | mutation | `--ledger`, `--job`, `--role`, `--run-id`, `--request-id`, `--actor` | `--provenance` | `--run-id` is the current active run id of the role, not a token | `claim_v1` |

The R16 to R19 verbs keep their E1 rows as reserved names and are unknown commands (`usage`) until their rounds.

Request parameter maps (`request_v3`, EX-2; keys sorted by canonical_v1; paths enter as normalized absolute private
strings; file contents never enter):

| Operation | `parameters` map |
| --- | --- |
| `activate` | `{expect_job, expect_revision, job}`; `expect_job` is the text `NONE` or the job ID, `expect_revision` is the decimal string or `none` |
| `claim` | `{job, role}` plus `provenance_path` only when `--provenance` was supplied |
| `takeover` | `{job, role, run_id}` plus `provenance_path` only when `--provenance` was supplied; `run_id` is a decimal string |

#### 4.1.3 Precondition sets (closed EX-3 vocabulary, D34, D107)

Every set contains `request_unused(id)` and `active_revision == R`. Members by operation:

| Operation | Members (all checked under the lock) |
| --- | --- |
| format3 load and `activate` | exactly as EX-3 "Format3 load and activate preconditions" (unchanged) |
| `claim` | `request_unused`, `active_revision == R`, `active_job == J`, `fact_version(job, role) == v` for every role of the active job, `role_state(target) == (state, drifted, run)`, `generation_current(target, G)` |
| `takeover` | the `claim` set with `role_state(target) == (running, drifted, run)` where `run` is the `--run-id`; no member uses a secret; `run_active` is not used |
| `refresh`, `checkpoint`, `release`, the `check` and `check-recover` commits, `verdict` | not defined in this part (D150): these operations are reserved and have no request body, facts or response here; their member sets are defined by the contract round of each operation (R16, R17, R18), using only the eight closed members |
| `status`, `preview` | none (reads; one read transaction) |

P2-1 defines the member sets of `claim` and `takeover` only, besides the unchanged load and `activate` sets. Part 1 EX-3 says the
remaining members of every other operation are defined by R15-A2 (lines 594 and 646); amendment A-4 re-points those
sentences to this section and records that the sets of `refresh`, `checkpoint`, `release`, the two check commits and
`verdict` are deferred to the R16 to R18 contract rounds, because the operations are reserved and nothing here can pin
members before their lifecycle and event shapes exist.

Enforcement (D151). Lua supplies `expected`; native enforces it. At EX-3 step 5 native validation requires `expected` to equal
exactly the fixed set of the operation in the table above: the same member kinds, with none omitted, added or duplicated
(for `claim` and `takeover`, the six members, with one `fact_version` member per role of the active job), and every value
equal to the snapshot read by the same command. Native refuses a `claim` change whose target before-state in the snapshot is
not `ready` or `stale`, and a `takeover` change whose named run is not the target's active run. A handler that sends any
of these changes is refused with code `internal` (its existing message; a handler response mismatch, not a driver-visible
conflict), because a correct handler returns `claim_conflict`, `fenced` or `state_conflict` before it builds the change. A
permissive handler therefore cannot omit `fact_version` or `role_state`, or claim a `done` role, and have native commit it.

Mismatch mapping (Part 1 EX-3 Conflicts and the closed EX-7 details; D114).

- At the snapshot, `claim` on a role that is not `ready` or `stale` is `claim_conflict` with `{state}`, the role's state,
  which is one of `blocked`, `running`, `done`, `retired`.
- At the snapshot, `takeover` is never `claim_conflict`: a `--run-id` that names no run of that job and role is
  `state_conflict` (a snapshot-time site, before the lock, named in the Messages cell by amendment A-13; a same-ID retry cannot
  succeed because the run id names no run, which is a recorded limit); any other run id that is not the role's current active run (this includes every role that is not
  `running`) is `fenced` with `{run_state}` of the named run.
- Under the lock, for `claim`, a failed `role_state(target)` member is `claim_conflict` with `{state}` of the locked state
  only when the locked role is taken by another run (`running`) or is in another closed-set state (`blocked`, `done`,
  `retired`). When the locked state is `ready` or `stale` (for example a stale role turned ready by a concurrent structural
  load) it is `state_conflict`, as is every other failed member (`request_unused`, `active_revision`, `active_job`,
  `fact_version`, `generation_current`).
- Under the lock, for `takeover`, a failed `role_state(target)` member is `fenced` with the locked `run_state` of the named
  run when that run is no longer the role's current active run, else `state_conflict`; every other failed member is
  `state_conflict`.
- `claim_conflict` never carries `ready` or `stale`, and `fenced` never carries a value outside its five. `reservation_open`
  and `run_active` are not used in R15. Claims of two different roles do not conflict because neither asserts the other's
  `role_state` and claims bump no `fact_version`.

#### 4.1.4 Verb behaviour and response schemas

Refusal order (D109) for a non-exempt verb (`activate`, `claim`, `takeover`, `preview`, `status --role`): (1) `usage`; (2)
`unsupported_ledger_version` for a format1 or format2 ledger; (3) early request replay (equal body returns saved bytes,
unequal `request_conflict`; mutations only; replay never reads `--provenance`); (4) `interpretation_mismatch` (EX-6; only when an active revision exists); (5)
expectations (`activate` only: `--expect-revision` mismatch is `revision_conflict` with `{expected,actual}`; `--expect-job JOB`
naming a job that is not the active job, or any job when none is active, is `no_active_job` (EX-7: "The ledger has no active job, or the named job is not the active job"); `--expect-job NONE` while a job is active is `state_conflict` at the snapshot, before the lock, a site named by amendment A-13); (6) `no_active_job` when no job is active or `--job` is not the active job (claim, takeover,
and `preview` and `status --role` when none is active); (7) `reference` with message `Unknown selected job` and field
`selection.job`, then `Unknown selected role` and field `selection.role` (activate: the job must exist in the active
revision; claim and takeover: the role must exist in the job; `status --role` and `preview`: see below); (8) role-state refusals (`claim_conflict`, `fenced`,
`state_conflict` per 4.1.3); (9) for `claim` and `takeover` with `--provenance`, the provenance input refusals of
[4.4.1](#p2-4-provenance-driver_provenance_v1) (the file is read by `read_provenance` after `read_execution`, so a refusal at steps (1) to (8)
wins and the file is not read); (10) commit preconditions (`state_conflict`).

Driver-visible limit (D149): a same-ID retry cannot succeed after the `state_conflict` of `activate` `--expect-job NONE` while a
job is active, or after the `state_conflict` of a `takeover` `--run-id` that names no run; the first is deterministic until the
active job changes and the second never changes, so the driver must change its expectation or run id, not retry. An
`activate` `--expect-job JOB` mismatch is `no_active_job` for the same reason and carries the same limit.

Role references of the reads (D152). "The role exists in the job" means declared by the active revision's recorded
interpretation for the active job. For `status --role R` and `preview --role R` with an active job: a role that is not
declared and is not a `role_state` row of the job is `reference` (message `Unknown selected role`, field `selection.role`). A
role that `status_v1` lists as `retired` but that the active revision no longer declares has no current W: `status --role R`
returns `role_detail` with `current` the string `none`, `changed` `[]` and `explain` `{work_or_semantic:[],output_vector:[],prerequisite:[]}`
(`acknowledged` and `reasons` as for any role), and `preview --role R` is `reference` with the same message and field.

Exempt reads (D108, D142): `status` without `--role` (bare, `--request-id`, `--workspace` with `--registry`, or both of the
last two), `history`, `export` and `orchestrator --ledger` skip step (4) only. `init` is not an exempt read: it is classified
`not bound (no active revision exists; not an execution verb)`, because it creates a ledger and so is a mutation of the
read/mutation split of E1 with no active revision to bind. Their order is: (1) `usage`; (2)
`unsupported_ledger_version` for a format1 or format2 ledger, which applies to every form of `status` (including `status
--role`) and to `orchestrator --ledger`, because they read execution tables that exist only in format3, and does not apply
to `history` and `export`, which keep every ledger format (`history --event` is valid on all), nor to `init`, which creates a ledger and applies no format, replay or binding step; (3) for `status
--role` only, the interpretation binding of step (4); (4) the verb's own refusals (`request_not_found` for `status
--request-id`, `no_active_job` for `orchestrator --ledger` when no job is active). Exempt reads never replay and never
write. `preview` is bound, not exempt: it follows the non-exempt order, so its format refusal is step (2), before the
binding. On a format1 or format2 ledger no degenerate status shape exists.

`activate`: effects as EX-4 "Activation" and "Re-activation". Response `activate_v1`:
`{version:1,kind:"activate",ledger,request_id,revision,job,previous_job,activation,event,roles}`. `ledger` is the
32-lowercase-hex identity; `revision`, `activation` and `event` are canonical decimal strings; `previous_job` is the job ID
or the empty string; `roles` is an array sorted by role ID of `{role,generation,state}` where `generation` is the new
ordinal as a decimal string and `state` is the after-state (`blocked` or `ready`).

`claim`: role must be `ready` or `stale`, otherwise `claim_conflict` with `{state}` (details of EX-7:
`blocked`, `running`, `done`, `retired`). Lua renders the dispatch packet ([P2-2](#p2-2-dispatch-projection-and-dispatch_manifest_v1)).
One EX-3 commit creates the run (token secret from the OS random source), the dispatch with retained packet, manifest and W
records (`dispatches` row, `kind` `claim`), the `running` transition and the request. Response `claim_v1`:
`{version:1,kind:"claim",ledger,request_id,job,role,revision,run:{id,generation,token},dispatch:{id,work_fp,render_digest,bytes},packet,manifest}`.
`work_fp` and `render_digest` are `xxh3_128:` digests; `bytes` is the packet Markdown byte length; `packet` is the packet
value P and `manifest` is the manifest value M, both assembled natively. `run.id` and `dispatch.id` are decimal strings,
`run.generation` is the generation ordinal as a decimal string, `run.token` follows the [Run token](#run-token-r15-9)
grammar. Key order is canonical (sorted).

`takeover`: requires `--run-id`, the id of the run the driver believes is current; the secret is not required. At step (8) the
named run must be a run of that job and role, otherwise `state_conflict` (no run state exists, so none is invented). If the
named run is not the role's current active run, which includes every role that is not `running` (`ready`, `stale`,
`blocked`, `done`, `retired`: no run is current), the refusal is `fenced` with `{run_state}` of the named run. `claim_conflict`
is never used by takeover (D114). Only a role that is `running` (with or without drift) whose current active run is the
named run proceeds. One commit: old run `fenced`, new generation (reason `takeover`, lineage carried), new run, dispatch
`kind` `takeover`, event `run_taken_over`. Response `claim_v1` with `kind:"claim"` and the same keys plus `previous_run`
(decimal string of the fenced run id) present only for takeover.

`status`: response `status_v1`:
`{version:1,kind:"status",ledger,active_revision,active_job,last_event,roles,drafts?,role_detail?,request?}`.
`active_revision` is a decimal string or `none`; `active_job` is the job ID or the empty string; `last_event` is a decimal
string; `roles` is the array of every `role_state` row of the active job (a retired role that the active revision no longer declares stays listed with state `retired`) sorted by role ID, each
`{role,kind,generation,state,drifted,run}` with `kind` `worker` or `reviewer` taken from the `role_structure_v1` record of the role's current generation (`snapshot.roles[].generation.structure`), never from the interpretation, `generation` the ordinal decimal string,
`drifted` a boolean, `run` either the string `none` or `{id,generation}` (decimal strings, never a token or secret);
`roles` is `[]` when no job is active. `--role R` adds `role_detail`:
`{role,acknowledged,current,changed,explain,reasons}` where `acknowledged` is `none` or `{work,render,dispatch}` (digest
texts and the dispatch id of the active run's latest dispatch), `current` is `{work}` (or the string `none` for a retired role the active revision no longer declares, D152), `changed` is the sorted array of
top-level W field names whose values differ between the acknowledged W value and the current one (empty when equal or when
no acknowledged W exists), `explain` is `{work_or_semantic,output_vector,prerequisite}` separating the three v5 causes
(`work_or_semantic`: array of changed W field names; `output_vector`: array, always `[]` in R15 because no completion
exists; `prerequisite`: array of `{id,phase,status}` of the prerequisites whose status is not `available`), and
`reasons` is the array of `{event,ordinal,reason}` for the role's most recent `transitions` rows, oldest first, at most 16.
`changed` and `explain` are computed from the decoded acknowledged W value `snapshot.roles[].run.dispatch.work` ([4.1.5](#415-services-and-protocol-d113-d137)) and the current W. `--workspace/--registry` adds `drafts`:
`{status,changed,error?}` where `status` is `unchanged`, `changed` or `invalid`, `changed` is the sorted array of logical
portable paths of captured declaration-closure files that differ from, are absent from, or are added to the active
revision's source, and `error` (only for `invalid`) is the typed error object `{code,message,field}` that load would
report; nothing is accepted. `--request-id` adds `request`:
`{request_id,operation,outcome,result,disposition?}` where `result` is the assembled saved result (the claim and takeover
result carries the run token, D118) and `disposition` is present only for claim and takeover with the EX-2 vocabulary;
unknown request is `request_not_found`. `--request-id` may be combined with `--workspace` and `--registry` (`request` and `drafts` are then both present); only `--role` excludes `--request-id`. On a format1 or format2 ledger every form of `status` is `unsupported_ledger_version` (D142).

`preview`: no active job gives `no_active_job`; the role must exist in the job, meaning declared by the active revision's recorded interpretation (`reference`, message `Unknown selected role`, field `selection.role`, also for a `retired` role the revision no longer declares). Response `preview_v1`:
`{version:1,kind:"preview",ledger,revision,job,role,generation,state,work_fp,render_digest,bytes,packet,manifest}`; it
claims and retains nothing, writes no event and no request, and has no token. Its packet has `mode:"dispatch"` with the
preview authority template.

#### 4.1.5 Services and protocol (D113, D137)

Each service is callable at most once, in the stated order; failure poisons further use; no Lua runs under the write lock;
argument maps have exact keys.

| Family | Services in order | Notes |
| --- | --- | --- |
| `activate` | `read_execution({})`; `commit_execution({change:C})` | no capture |
| `status` | `read_execution({})`; then, only with `--request-id`, `read_request({request_id:TEXT})`; then, only with `--workspace` and `--registry`, `open_workspace({workspace:SLOT})` and `capture_drafts({route:R})` | no commit; the optional services combine and keep this order |
| `preview` | `read_execution({})` | no commit |
| `claim` | `read_execution({})`; `read_provenance({provenance:SLOT})` only when `--provenance` is present; `commit_execution({change:C})` | |
| `takeover` | the `claim` services | |
| format3 `load` | `open_workspace`, `capture_load`, `read_execution({})`, `accept_load({bundle:B,semantic:M,execution:CHANGE})` | format1 and format2 keep three services and `{bundle,semantic}` |

Results (V):

- `read_execution({})` returns `{version:1,ledger,last_seq,snapshot}`. `snapshot` is `execution_snapshot_v1` with exactly
  `active_revision` (decimal string or `none`), `active_job` (ID or empty string), `activation` (decimal string or `none`),
  `revision` (`none` or `{id,interpretation,versions,stdlib_digest,registry,source}` with the decoded recorded values),
  `roles` (every `role_state` row, sorted by job then role: `{job,role,kind,generation:{id,ordinal,structure},state,drifted,fact_version,run}` (`structure` is the decoded `role_structure_v1` value and `kind` equals its `kind`; retired roles keep their rows)
  where `run` is `none` or `{id,generation,dispatch:{id,work_fp,render_fp,revision,work}}`, where `work` is the decoded acknowledged `work_exec_v1` W value of that dispatch, the retained record `dispatches.work_id`; Part 1 EX-3 step 2 requires the active run and its acknowledged dispatch, W value and digests) and `publications` (array, `[]` in
  R15). Run secrets and tokens never appear. Bound: `execution_snapshot_bytes` 16777216 (EX-8).
- `read_request({request_id})` returns `{request_id,operation,outcome,result,disposition?}` (the assembled saved result).
- `read_provenance({provenance:SLOT})` returns the normalized `driver_provenance_v1` document after the bounds of
  [P2-4](#p2-4-provenance-driver_provenance_v1); a refusal is one of `io`, `path_denied`, `capture_changed`, `resource_limit` or `schema` as mapped in [P2-4](#p2-4-provenance-driver_provenance_v1).
- `capture_drafts({route:R})` returns `{outcome:"captured",bundle:B,changed:[paths]}` or
  `{outcome:"rejected",error:E,changed:[paths]}`; E is the typed error of the declaration-content classes (`lua_syntax`,
  `capability_denied`, `schema`, `reference`, `ownership`, `dependency_cycle`, `path_denied`, `unsupported_version`);
  `capture_changed`, `io`, `resource_limit`, `ledger_busy` and `internal` fail the command. Lua runs `interpret_bundle` on a
  captured bundle to separate `unchanged`/`changed` from `invalid`.
- `commit_execution({change:C})` returns the decoded saved response value after the lock is released. `C` is
  `execution_change_v1` with exactly `version` (1), `operation` (`load`, `activate`, `claim` or `takeover`),
  `snapshot_seq` (decimal string), `facts` (operation facts: activate `{job,generations}`; claim `{role,provenance?}`;
  takeover `{role,previous_run,provenance?}`; load `{generations,fenced_runs}`), `vector` (the complete next role-state
  vector of the active job, validated and never written wholesale), `transitions`, `packet` (claim, takeover only:
  `{markdown,manifest,work}`) and `expected`. `expected` is an array of objects with `kind` one of the eight closed
  members (`active_revision`, `active_job`, `fact_version`, `role_state`, `generation_current`, `run_active`,
  `reservation_open`, `request_unused`). Native validates `expected` as an exact fixed set and refuses a non-claimable claim target or a takeover whose run is not the active run (D151, [4.1.3](#p2-1-verbs-and-services)), recomputes the render digest from the packet bytes and the W digest from
  the W value, allocates every id and the run secret, and the handler's response must equal the retained native result.
- Format3 `accept_load` result is the existing complete load success envelope; the extra key `execution` is `CHANGE` of the
  same shape with `operation:"load"`. This amends `load-command-contract.md` services table for format3 only.

The exempt reads of D108 skip step (4) only; their full order and their format1/format2 outcome are in 4.1.4 (D142).
`status` without `--role` and `orchestrator --ledger` run with whatever embedded Lua the binary carries.

### 4.2 P2-2 Dispatch projection and dispatch_manifest_v1

#### 4.2.1 Packet mode and sections (D117)

- Packet value P keeps the render keys `version`, `mode`, `workspace`, `spec`, `job`, `role`, `kind`, `work`, `markdown`;
  `mode` is `"dispatch"` for claim, takeover and `preview`. Render's `mode:"preview"` is unchanged and render stays
  database-free.
- Markdown follows the render presentation convention (LF, no BOM, final LF). Digest `render_fp` is
  `H("fragment_v1", exact_markdown_UTF8_bytes)`.
- Section order for dispatch: `authority`, `run`, `goal`, `acceptance`, `requirements`, `contracts`, `boundary`,
  `dependencies`, `shape_policy`, `checks`, `outputs`, `observations`, `preferences`, `rationale`, `workspace_notes`,
  `job_notes`, `checkpoint`, `omissions`, `recorded_history`, as the shipped worker and reviewer projections order them
  with `run` inserted second. `run` is mandatory and is never omitted by the budget algorithm.
- Fixed authority statements (the `authority` section body, byte-exact templates; `{x}` marks substitution):
  - dispatched (claim and takeover): `This packet was dispatched by Planar for run request {request_id}, generation {generation} of role {role}, under revision {revision} of job {job}. Checks have not run in this dispatch. Upstream facet content comes from recorded publications, and prerequisite status comes from ledger qualities, not from a checkout.`
  - preview: `This packet is a preview of a dispatch for generation {generation} of role {role}, under revision {revision} of job {job}. No run exists and nothing was retained. Checks have not run in this dispatch. Upstream facet content comes from recorded publications, and prerequisite status comes from ledger qualities, not from a checkout.`
  `{generation}` is the generation ordinal; `{revision}` the revision id; both decimal.
- The `run` section is framed as every generated section (`## run`, a blank line, the body, one structural LF; the render
  Markdown projection rule). Its body is these lines joined by single LFs with no blank line: first `run request: {request_id}`
  (dispatched) or `run request: none` (preview); then the legal next actions of the role kind in E1 table order, worker
  `refresh`, `checkpoint`, `check`, `release`, reviewer `refresh`, `checkpoint`, `verdict`, `release`, one line
  `- {verb}: {purpose}` each; then the line `Actions not shipped in this build are unavailable.` The purposes are fixed
  byte-exact by this contract (D147), each at most 200 bytes, and `dispatch.lua` reproduces them:

  | verb | purpose (the text after `- {verb}: `) |
  | --- | --- |
  | `refresh` | `Re-render this work and acknowledge the current packet.` |
  | `checkpoint` | `Record observations of progress on this run.` |
  | `check` | `Run the checks of the job against the output of this role.` |
  | `release` | `End this run as released or interrupted.` |
  | `verdict` | `Record the verdict of the reviewer for this run.` |

  In R15 these verbs are reserved. The run section never contains a token or secret.
- Projection IDs: the shipped-owned `worker` and `reviewer` projections produce dispatch packets from the internal input
  `dispatch_context_v1` (not an admissible external projection input). No new projection ID exists, and external
  projections do not apply to dispatch.

#### 4.2.2 Mandatory run section identifier

The section ID `run` is a reserved built-in section ID (D135; pointer in [P2-7](#p2-7-command-render-load-ledger-and-external-profile-pointers)).

#### 4.2.3 dispatch_manifest_v1 field table

Canonical_v1 value M, stored as a value record with codec ID `dispatch_manifest_v1` (`dispatches.manifest_id`). Bound
`manifest_bytes` 16777216 (EX-8). Fields, exactly:

| Key | Value |
| --- | --- |
| `version` | 1 |
| `revision` | revisions id, decimal string |
| `job`, `role` | IDs |
| `generation` | the generation ordinal, decimal string |
| `run` | `{request,kind}`: `request` is the claim or takeover request ID, `kind` is `claim` or `takeover`; takeover adds `previous` (decimal string of the fenced run id) |
| `versions` | the seven `work_exec_v1` versions then `dispatch_v1` |
| `work` | `{digest}` (the `xxh3_128:` work fingerprint; the W value is the record `dispatches.work_id`) |
| `render` | `{digest,bytes,budget_bytes}` as render M |
| `sections` | ordered array of `{id,mandatory,included,bytes}`, includes `run` with `mandatory:true` |
| `omitted` | ordered array of optional section IDs |
| `prerequisites` | sorted array as render M, statuses from ledger qualities (`available`, `unavailable`, `unverified`, `answered`; `not_captured` never) |
| `registry` | `{id,revision,authority:"driver_selected_trusted_configuration"}` |
| `inputs` | `{source,interpretation,publications,repair}`: `source` and `interpretation` are record ids (decimal strings of `revisions.source_id` and `revisions.interpretation_id`), `publications` is a sorted array of `{work,facet,publication,semantic}` (publication id decimal string, semantic digest text; `[]` in R15), `repair` is the repair revision as a decimal string (`"0"` in R15) |

There is no `files` array, no `bytes_hex`, no `base`, `observation`, `outputs`, `role_output`, `targets`, `assertions`,
token or secret key. The W value and the packet Markdown are separate records (`dispatches.work_id`, file record mode 420
for `dispatches.packet_id`). Render-contract pointer: [P2-7](#p2-7-command-render-load-ledger-and-external-profile-pointers).

### 4.3 P2-3 work_exec_v1 (D120)

`work_exec_v1` is computed from the ledger alone. It is the execution-design EX-5 bullet "Execution W is a new versioned
recipe `work_exec_v1`" made exact, and it cites that bullet rather than re-deciding it. The W value has exactly these 24
fields (listed in W's presentation order; canonical_v1 sorts the keys when W is encoded): `versions`, `workspace`, `spec`,
`job`, `role`, `root`, `base`, `goal`, `units`, `milestones`, `constraints`, `prescriptions`, `contracts`, `dependencies`,
`facets`, `pipeline_predecessors`, `questions`, `ownership`, `effective_checks`, `checks`, `toolchains`, `shape`,
`start_facets`, `repair`.

- Fields other than `versions`, `base`, `start_facets` and `repair` have exactly the definitions of `render-contract.md` for
  W (units, milestones, constraints, prescriptions, contracts, dependencies, facets, pipeline predecessors, questions,
  ownership, effective checks, checks, toolchains, shape), taken from the active revision's recorded interpretation.
- `versions`: `["declarations_v1","selection_v1","dependencies_v1","exact_bytes_v1","canonical_v1","xxh3_128_v1","work_exec_v1"]`.
- `base`: always the declared form `{status:"declared",root:<source ID>,label:<base label>}`, as execution-design EX-5
  fixes. Base content enters output identity, not W (E5 A).
- `start_facets`: array of `{dependency,semantic}` for start and facet dependencies consumed by selected units: `semantic`
  is the `xxh3_128:` text of the `publications.semantic` of the current publication of the facet's producer ("current" is
  defined by R17), or `{dependency,unavailable:true}` when no publication exists (always, in R15). Complete-only facet
  values, producer quality and outputs are excluded; units whose producer is selected are excluded as in render.
- `repair`: the role's repair revision in its structural lineage (Part 1 EX-5 Lineage) as a decimal string; `"0"` before
  any repair, which is always the case in R15 because R18 admits repairs. `repair` enters W: `render-contract.md` says "no
  repair revision or generation is fabricated. Later execution must add those through a versioned recipe", and
  `work_exec_v1` is that recipe for the repair revision.
- Generation is not a W field: it is carried by `dispatch_v1` and `dispatch_manifest_v1` (`generation`), not by W, so a takeover generation never moves W (execution-design EX-5 bullet, operator E5 A; the rule that takeover must not move W).
- Digest: `work_fp = H("work_exec_v1", W)` with `H` of `render-contract.md` (canonical_v1, XXH3-128, `xxh3_128:` text,
  16 raw bytes in `dispatches.work_fp`). The W value is retained as a value record with codec ID `work_exec_v1`.
- Render emits `work_v1` unchanged. The render-contract pointer adds `work_exec_v1` and `dispatch_v1` to the interpretation
  identifier list and states that render never emits them.

### 4.4 P2-4 Provenance (D121 to D123)

`driver_provenance_v1` document (UTF-8 JSON file, Glaze strict parse, unknown or duplicate keys and trailing bytes are
`schema`):
`{version:1,driver:{name,version},trial:{id},view:{id},model:{requested,actual},session:{requested,actual},settings:[{name,value}]}`.
Leaf rule: every leaf (`driver.name`, `driver.version`, `trial.id`, `view.id`, `model.requested`, `model.actual`,
`session.requested`, `session.actual`, and each settings `value`) is a UTF-8 string of 1 to 1024 bytes without NUL, or
exactly the object `{"status":"unknown"}` (one key `status`, value the string `unknown`). A settings `name` is a string leaf
(never unknown), names are unique, normalized sorted by unsigned bytes. Every key of the shape is required; a driver that
knows nothing writes the unknown leaf. Bounds (EX-8 row "Provenance file"): at most 65536 bytes (`provenance_bytes`),
depth 4 (`provenance_depth`), at most 32 settings (`provenance_settings`), strings at most 1024 bytes
(`provenance_string_bytes`); a violation is `resource_limit` with `details:{limit,maximum}` and the message
`Execution resource limit exceeded`. The reader applies the same bounds to a stored record; a stored value that violates
them is `ledger_corrupt`.

Stored record: value record, codec ID `driver_provenance_v1`, canonical_v1 value
`{version:1,authority:"driver_assertion_unverified",document:<normalized document>}`; referenced by `runs.provenance_id`
and the event reference key `provenance` (EX-4 table). Absence of the file stores no record: `runs.provenance_id` is NULL
and every field reads as unknown; the history event expansion shows such a run with the all-unknown driver document ([P2-5](#p2-5-history-event-selector-and-request-results)). Attachment points: `claim` and `takeover` (live in R15, option `--provenance`); `release`
and `check` are specified as fields only (their rounds R16 and R17 pin the event reference key); `check-recover` and
`verdict` attach none. Planar's native per-run provenance (ledger identity, revision, source digest, job, role,
generation, base label, W and render digests, requested preferences `model` and `session` from the job) is derived from rows
and reported by the [history event expansion](#p2-5-history-event-selector-and-request-results); it is never taken from the
driver file. The token is never part of any provenance value.

#### 4.4.1 Provenance input handling (D146)

`--provenance FILE` is an opaque native slot and an explicit single-file capture that uses the same refusals as the authoring
`--diagnostic` rows (`authoring-contract.md` Messages table). The `read_provenance` service reads the file once. The request
body holds only `provenance_path`, never contents, so an early replay never reads the file. Existing codes and messages only;
no new message:

| Condition | Code | Message |
| --- | --- | --- |
| path missing, unresolvable, or the file cannot be opened | `io` | `Cannot resolve explicit input path` |
| the final path component is a symbolic link | `path_denied` | `Explicit input file is a symbolic link` |
| not a regular file (directory, FIFO, socket, device) | `io` | `Explicit input must be a regular file` |
| the file changed between the capture gate and the end of the read | `capture_changed` | `File changed during read` |
| a bound of EX-8 is exceeded | `resource_limit` | `Execution resource limit exceeded` with `details:{limit,maximum}` |
| JSON grammar or shape violation | `schema` | the existing message of the shared JSON parser or schema normalizer |

Path normalization refusals keep their existing messages. Check order, first failure wins: (1) the captured size exceeds 65536
bytes, `provenance_bytes`, before any parse; (2) JSON grammar (invalid JSON, duplicate keys, trailing bytes), `schema` with
field `provenance`; (3) nesting depth above 4 (the root object counts 1, so a `settings[].value` object sits at depth 4),
`provenance_depth`; (4) more than 32 `settings` entries, `provenance_settings`; (5) any string longer than 1024 bytes,
`provenance_string_bytes`, so a 1025-byte leaf is `resource_limit` and never `schema`; (6) shape and leaf-rule violations
(missing or unknown key, wrong type, empty string, NUL byte, an object other than `{"status":"unknown"}`, duplicate settings
names), `schema`. The `schema` field is `provenance` alone for a grammar violation, otherwise `provenance.` followed by the
keys from the document root joined by `.`, array indexes as canonical decimals (for example `provenance.model.requested` and
`provenance.settings.0.value`); a missing key names its own path and an unknown key names the unknown key. The provenance
refusals are step (9) of the refusal order in [P2-1](#p2-1-verbs-and-services), after the role-state refusals and before the
commit preconditions.

### 4.5 P2-5 History event selector and request results (D124 to D127, D143)

`planar history --ledger PATH --event ID`: `--event` is a canonical positive decimal 1..9223372036854775807; it excludes
`--revision`, `--request-id`, `--at-event`, `--after-event` and `--limit` (any combination is `usage`); it works on format1,
format2 and format3 ledgers; an unknown event is `event_not_found`. Services:
`read_history({selector:"event",id:ID,request_id:"",limit:50})`. `limit` carries the existing absent-limit value 50 that
`commands.lua` passes whenever `--limit` is absent (`present.limit and options.limit or 50`), and the event selector ignores
it; `id` crosses into Lua as a canonical decimal string like `at-event`. The selector set of the history family becomes
`page`, `revision`, `request`, `state`, `event`.

Format3 event summary (R15-10, D143). Format1 and format2 event summaries are unchanged. A format3 event summary, wherever it
appears (`page` data, `state` data, `event` data), is the existing `{sequence,type,version,revision,request_id,actor,occurred_at,payload}`
plus the four typed reference keys `job`, `role`, `generation_id` and `run_id`, being the `events` columns of the same
names: `job` and `role` are IDs, and each is the empty string when the column is NULL (consistent with payload v2
`active_job`, because `none` is a legal `id` per `declarations.md:33` and EX-4 rejects `none` for a job, so a job or role
named `none` would be indistinguishable from an absent one); `generation_id` and `run_id` are canonical decimal strings, and
each is `"none"` when the column is NULL (R14-E2: absent database IDs use `none`; that rule covers numeric database IDs only). `version` is 1 for `ledger_initialized` and 2 for every other format3 event,
and `payload` is the structured public payload, version 2 for format3 as EX-4 pins it
(`{version:2,operation,before:{active_revision,active_job},after:{active_revision,active_job},reason,references}`; the
`ledger_initialized` payload keeps version 1).

Format3 at-event state (R15-10, D143). `history --at-event ID` on a format3 ledger returns `{selector:"state",event,state,revision}`
where `event` is the format3 event summary and `revision` is the revision metadata or `"none"`, as before. `state` is
`{active_revision,active_job,activation,roles,runs}` replayed from rows with seq at most ID, without Lua (EX-1 invariant
19): `active_revision` as before; `active_job` is the job ID or the empty string; `activation` is the decimal id of the
latest `job_activations` row with `event_seq` at most ID, or `"none"`; `roles` are the roles of the active job at that
event, sorted by role ID, each `{role,generation,state,drifted,run}` taken from the last `transitions` row of that (job,
role) with `event_seq` at most ID (`generation` is the generations ordinal as a decimal string; `state` and `drifted` are
the row's after-values; `run` is `"none"` or `{id,generation}` of the run claimed or taken over at the event of the
role's last transition into `running`); `runs` are the runs active at that event, sorted by id, each `{id,role,generation}`
(decimal strings). Role `kind` is not part of the replay (it needs the interpretation), and no token or secret appears.
Roles of other jobs are retired (EX-1 invariant 17) and omitted. Format1 and format2 `state` data stays exactly
`{active_revision}`.

Response of `--event`: the existing history envelope `{version:1,kind:"history",ledger,active_revision,last_event,data}` with
`data` = `{selector:"event",event,transitions,execution?}`:
`event` is the event summary (format3 extension above for format3); `transitions` is the array of the event's
`transitions` rows ordered by ordinal, each
`{ordinal,job,role,generation,before,after,reason}` (`generation` the ordinal as a decimal string; `before` is `none` or
`{state,drifted}`; `after` is `{state,drifted}`), and is `[]` for format1 and format2 events and for events without
transitions; `execution` is present only for `job_activated`, `run_claimed` and `run_taken_over` events:
- `job_activated`: `{activation:{id,job,revision}}`.
- `run_claimed`, `run_taken_over`: `{run:{id,generation,request_id},dispatch:{id,kind,work_fp,render_fp,revision},packet:{id,bytes,markdown},manifest:{id,value},work:{id,value},provenance:{native,driver}}`;
  `packet.markdown` is the exact stored UTF-8; `manifest.value` and `work.value` are the decoded canonical values;
  takeover adds `previous_run` (decimal string). `provenance` is always present for these two event types.
  `native` is `{ledger,revision,source,job,role,generation,base_label,work_fp,render_fp,preferences}`, derived from rows and
  the recorded interpretation and never from the driver file: `ledger` the 32-lowercase-hex identity, `revision` the
  decimal id, `source` `{id,digest}` of the revision's source record (`digest` its `xxh3_128:` text), `generation` the
  ordinal as a decimal string, `base_label` the label of the W `base`, `work_fp` and `render_fp` the digest texts, and
  `preferences` `{model,session}` the requested preferences of the job as strings, each `{"status":"unknown"}` when the job
  declares none. `driver` is `{id,authority,document}`: with a stored record (`runs.provenance_id` not NULL) `id` is the
  record id, `authority` is `driver_assertion_unverified` and `document` is the normalized `driver_provenance_v1`
  document; without one, `id` is `"none"`, `authority` is `"none"` and `document` is the all-unknown form
  `{version:1,driver:{name:U,version:U},trial:{id:U},view:{id:U},model:{requested:U,actual:U},session:{requested:U,actual:U},settings:[]}`
  with `U` the object `{"status":"unknown"}`, so the rule "a missing driver file records every field unknown" is
  observable.
The expansion never contains the run token or `runs.secret`; the reader does not select the secret column. Records are
decoded natively with no Lua, as `--at-event` is. The whole stdout including the final LF is at most 16777216 bytes;
the existing refusal `resource_limit` with `details:{limit:"output_bytes",maximum:16777216}` and empty stdout applies and
nothing is truncated. A referenced record that is missing is `ledger_corrupt`; an unsupported codec is
`unsupported_interpretation`.

`history --request-id` on format3 (OP2, D126): the data keeps `{selector:"request",result}`, no `disposition`. `result` is the
decoded saved result of the request: for `load` the existing result; for `activate` the `activate_v1` value of
[P2-1](#p2-1-verbs-and-services); for `claim` and `takeover` the `claim_v1` value including `run.token`, with `packet` and
`manifest` assembled natively from the request's `attachment_id` record and the dispatch's `manifest_id` record (the saved
`result` column holds the value without `packet` and `manifest`, at most 1 MiB). Format1 and format2 responses are
unchanged. The whole stdout including the final LF is at most 16777216 bytes: a format3 `claim` or `takeover` result assembles the packet (up to 1 MiB) and the manifest (up to 16 MiB), so it can exceed the ceiling, and the existing refusal `resource_limit` with `details:{limit:"output_bytes",maximum:16777216}` and empty stdout applies with no truncation. The records are decoded natively with no Lua. History pages (`--after-event`) and `--at-event` never contain the token.

### 4.6 P2-6 Orchestrator live form (D128 to D130)

`planar orchestrator --ledger PATH` exists only where the orchestrator is compiled, which is the authoring bundle (D148); the production binary has no `orchestrator` verb and nothing in this section applies to it, except that the reservation of the section IDs `run` and `orchestrator_state` applies in every build, production included (D135, [P2-7](#p2-7-command-render-load-ledger-and-external-profile-pointers)), even though `orchestrator_state` is rendered only in the authoring bundle. Exactly one form is accepted. Database-free form: `--workspace`, `--registry`, `--job`
together (RA-1), unchanged. Ledger form: `--ledger` alone; any other combination is `usage`. The descriptor's four options
are all `required:false` and the presence rule is native. Services (ledger form): `read_execution({})`. Response kind
`orchestrator_v1`, envelope kind `prompt` (`kind:"orchestrator"`), as RA-3.

On a format1 or format2 ledger the ledger form is `unsupported_ledger_version` (D142). No active job: `no_active_job`. Otherwise the context is `orchestrator_context_v1` of RA-7 with workspace, spec, job, shape,
roles, graph and prescriptions taken from the active revision's recorded interpretation for the active job, plus one extra
key `live` (absent in the database-free form):
`live` = `{ledger,revision,last_event,activation,roles,runs,blockers}` where `roles` is the status role array of
[P2-1](#p2-1-verbs-and-services), `runs` is the array of active runs sorted by role `{id,role,generation,request_id}`
(decimal strings; no token, no secret), `blockers` is `[]` in R15, and `activation` is a decimal string. An additional
mandatory section `orchestrator_state` follows `orchestrator_actions` in the ledger form only; it lists role states, runs,
blockers and the legal next action per role state (`ready` or `stale`: `claim`; `running`: `status`, or `takeover` when the
driver lost the session; `running` with drift: `status` and, from R16, `refresh`; `blocked`: wait for prerequisites;
`done`: none; `retired`: none). RA-7's sentence "It states no execution state" applies to the database-free form only.

Where the orchestrator is compiled, `actions` labels: `available` exactly when the verb is in the invocation's sealed catalog. After R15 the future list is
`refresh`, `checkpoint`, `release`, `recover`, `check`, `check-recover`, `verdict`; `activate`, `status`, `preview`, `claim`
and `takeover` are `available` (they join the catalog at the R15 integrated gate); R16 flips `refresh`, `checkpoint`,
`release`, `recover` (and `preview --since`), R17 flips `check` and `check-recover`, R18 flips `verdict`. `with-profile` is
never an action. Pointer: `authoring-contract.md` RA-7 is the base definition; this contract extends it by insertion and
`authoring-contract.md` is not edited by this round. `orchestrator --ledger` is an exempt read (D108).

### 4.7 P2-7 Pointers (D131)

Each pointer below is an inserted paragraph labelled `Pointer (R15-A2):` in the named contract. Each says that nothing is
shipped until the R15 integrated gate, that the rows, tables and bytes beside it stay current until that gate, and that it
narrows no approved clause. No existing row, table or sentence of those contracts is edited. P2-7 repeats the constants
(help lines, handler IDs, response kinds, capability names, option names, the ten-family list, section IDs) exactly as the
pointers state them, and links to the pointers.

#### 4.7.1 command-contract.md (one inserted pointer paragraph after the RA01-A pointer, line 29)

States, as gate-time content that is not shipped now:

- Catalog counts at the R15 integrated gate, per build (D148): ten mandatory rows in the production build (the current five plus
  the five R15 rows) and sixteen in a build that compiles the authoring bundle (the ten plus the six RA01 rows; the RA01 rows
  and the `orchestrator` verb exist only there). The history family gains `--event` in both; the orchestrator family gains the
  ledger form only where the orchestrator is compiled.
- ABI rows:

  | name / family | handler_id | response_kind | capabilities |
  | --- | --- | --- | --- |
  | activate / activate | `shipped.activate.v1` | `activate_v1` | `execution_commit_v1` |
  | status / status | `shipped.status.v1` | `status_v1` | `execution_read_v1`, `draft_capture_v1` |
  | preview / preview | `shipped.preview.v1` | `preview_v1` | `execution_read_v1` |
  | claim / claim | `shipped.claim.v1` | `claim_v1` | `execution_commit_v1`, `provenance_read_v1` |
  | takeover / takeover | `shipped.takeover.v1` | `claim_v1` | `execution_commit_v1`, `provenance_read_v1` |

  and, only where the orchestrator is compiled, `orchestrator` gains capability `execution_read_v1` beside
  `orchestrator_capture_v1`; `orchestrator --ledger` exists only there.
- Option signatures: activate text `--ledger!`, `--job!`, `--expect-revision!`, `--expect-job!`, `--request-id!`,
  `--actor!`; status text `--ledger!`, `--role`, `--request-id`, `--workspace`, `--registry` (`--role` excludes
  `--request-id` and nothing else; `--workspace` and `--registry` both or neither); preview text `--ledger!`, `--role!`;
  claim text `--ledger!`, `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance`; takeover text `--ledger!`,
  `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance` and decimal `--run-id!`; history gains decimal `--event`
  (excluding `--revision`, `--request-id`, `--at-event`, `--after-event`, `--limit`); orchestrator text `--ledger`,
  `--workspace`, `--registry`, `--job` all optional with the native both-or-exactly-one-form rule.
- Lua boundary: the rule "Database IDs/cursors (`revision`, `at-event`, `after-event`) cross into Lua only as canonical
  decimal strings, absent `"0"`" extends to `run-id` and `event`.
- Opaque slots gain `provenance`.
- Services: the five families have the service lists of [P2-1](#p2-1-verbs-and-services); `orchestrator` gains
  `read_execution({})`; `read_history` gains the selector `event` and is called as
  `read_history({selector:"event",id:ID,request_id:"",limit:50})`, `limit` being the existing absent-limit value 50
  that `commands.lua` passes (the event selector ignores it, no `limit:1`).
- The root help bytes of 4.7.2.

#### 4.7.2 Fixed root help after R15, per build (exact; stated in the pointers, current bytes unchanged until the gate)

Production build (no authoring path, D148; ten rows). Starting from the bytes in `src/cmd/planar/bootstrap.cpp`
(`default_root_help`):

1. The second line of the history block `  or --revision ID | --request-id ID | --at-event ID\n` becomes
   `  or --revision ID | --request-id ID | --at-event ID | --event ID\n`.
2. Immediately before the final line `planar --help\n` come, in this order, five lines:

```text
planar activate --ledger PATH --job ID --expect-revision ID_OR_NONE --expect-job JOB_OR_NONE --request-id ID --actor TEXT
planar status --ledger PATH [--role ID | --request-id ID] [--workspace FILE_OR_NAME --registry FILE]
planar preview --ledger PATH --role ID
planar claim --ledger PATH --job ID --role ID --request-id ID --actor TEXT [--provenance FILE]
planar takeover --ledger PATH --job ID --role ID --run-id ID --request-id ID --actor TEXT [--provenance FILE]
```

No RA01 line and no orchestrator line exists in this build.

Authoring bundle (sixteen rows). The production bytes above, then the splice of `authoring::splice_root_help`, which inserts
the RA01 lines immediately before the final `planar --help\n`. The relative order is therefore fixed: the history block, the
export line, the load line, the five R15 lines above (activate, status, preview, claim, takeover), then these seven spliced
lines, then `planar --help\n`:

```text
planar spec-prompt --workspace FILE_OR_NAME --registry FILE
planar critique --workspace FILE_OR_NAME --registry FILE
planar ingest --workspace FILE_OR_NAME --registry FILE
planar explain-rejection --diagnostic FILE
planar observe --from DIRECTORY
planar orchestrator --workspace FILE_OR_NAME --registry FILE --job ID
  or --ledger PATH
```

The last line is a seventh spliced line that directly follows the orchestrator line (R15-D work on `root_help_lines`; the
RA01-B splice has six lines today). Every other help byte, the `with-profile` help bytes and parse and usage precedence are
unchanged. The production lines (the history continuation and the five lines) are identical in the command-contract,
load-command-contract and external-profile-contract pointers and in P2-7; the authoring-bundle ordering is stated in the
command-contract pointer and in P2-7. The external-profile pointer states that default root help stays byte-for-byte old
apart from the amendments listed in command-contract.

#### 4.7.3 render-contract.md (inserted pointer paragraphs only)

After the `**P** has exactly ...` paragraph: a pointer that execution adds packet mode `dispatch` (all other P keys
unchanged), defined by P2-2; the dispatch manifest `dispatch_manifest_v1` is the execution analogue of M with record IDs for
files. After the work fingerprint paragraph (lines 402-412): a pointer that `work_exec_v1` is the versioned recipe that "later
execution must add", defined by P2-3, with the explicit statement that `repair` enters W (render-contract.md: "no repair revision or generation is fabricated. Later execution must add those through a versioned recipe") and that the generation is carried by `dispatch_v1` and `dispatch_manifest_v1` (`generation`), not by W, citing the execution-design EX-5 bullet (operator E5 A) and the rule that takeover must not move W; render emits only `work_v1` and stays database-free (no `--claim`,
`--refresh` or `--since`). After the identifiers paragraph: a pointer that execution adds the identifiers `work_exec_v1` and
`dispatch_v1`, which render never emits.

#### 4.7.4 load-command-contract.md (one inserted pointer paragraph after the RA01-A pointer)

States: on format3 only, the services are `open_workspace`, `capture_load`, `read_execution({})`,
`accept_load({bundle,semantic,execution})`, with a `read_execution({})` row `{version:1,ledger,last_seq,snapshot}`
described in the pointer (not inserted into the services table), and `accept_load` takes the extra key `execution` (`CHANGE`
of `operation:"load"`); format1 and format2 loads keep three services and `{bundle,semantic}`, and the sentence "It receives
exactly three context methods" stays current for them (format3 receives four). Capability: the load family's capability set
stays `accept_load_v1`; it covers `read_execution({})` on format3 (no new capability for load; the load handler cannot call
`commit_execution`, which belongs to the execution families). The pointer also states the production help lines of 4.7.2, byte-identical to the command-contract pointer: the history continuation `  or --revision ID | --request-id ID | --at-event ID | --event ID` and the five lines for `activate` (with `--expect-job JOB_OR_NONE`), `status`, `preview`, `claim` and `takeover`.

#### 4.7.5 ledger-contract.md (one inserted pointer paragraph after the format3 request-lookup paragraph at line 451)

States: the command-surface row text `planar history --ledger <path> --event <id>`: return the event with its transitions and,
for execution events, the expanded records ([P2-5](#p2-5-history-event-selector-and-request-results)), mutually exclusive
with all other selectors and paging; the data selector `event` joins `page`, `revision`, `request`, `state` in the `data`
sentence; the format3 event summary and `--at-event` state extensions of P2-5. Format1 and format2 text is unchanged. The
command-surface table and the data-selector paragraph are not edited.

#### 4.7.6 external-profile-contract.md (one inserted pointer paragraph after the RA01-A pointer)

States: (a) X-CLI family list: the shipped families admitted under `with-profile` are the five of X08 plus `activate`,
`status`, `preview`, `claim`, `takeover` (ten); their known slots are `ledger` and, per verb, `workspace`, `registry`,
`provenance`; the admission rows, the `command_options` variant change and the x07-addendum amendment are R15-B and R15-C
work; the six RA01 verbs stay embedded-only (RA-9 item 5). (b) Reserved section IDs gain `run` and `orchestrator_state` in
the X-PROJ union, in every build including production (the reservation is not build-qualified, as `runtime.cpp` already reserves the RA01 `orchestrator_*` IDs in every build; `orchestrator_state` is rendered only in the authoring bundle). (c) Root help follows command-contract. (d) Consequence statement, modelled on RA-9 item 6, "valid today,
refused after R15": a profile that registers a command or family named `activate`, `status`, `preview`, `claim` or
`takeover`, a handler ID `shipped.activate.v1`, `shipped.status.v1`, `shipped.preview.v1`, `shipped.claim.v1` or
`shipped.takeover.v1`, or a section ID `run` or `orchestrator_state`, is valid today and refused after R15 by the existing
registration collision error (`reference`). X-LIMIT counts 64 commands including shipped, so admitting five more shipped
families under `with-profile` cuts external command capacity by five (59 to 54); projections are unchanged (no new
projection ID). The embedded probe gains the five families and the `history` probe signature gains `--event ID` (`with-profile
P history --event ID` is `usage` today and accepted after R15); the five verbs under `with-profile` stop being `usage`
after admission; the with-profile completeness rule moves from five to ten. (e) The message tables are untouched and the
X-LIMIT table is unedited.

### 4.8 Messages, pointer amendments, traceability, checklist, decisions (execution-contract.md)

- Messages (amendments A-8 to A-10 and A-13, each reasoned in `## Part 1 amendments (R15-A2)`): the `revision_conflict` row's
  "Execution sites" cell, which R15-A1 I66 recorded as "The format3 load compare-and-swap only", becomes "The format3 load
  compare-and-swap and the `--expect-revision` check of `activate`"; the `reference` rows' cells name the field values
  `selection.job` and `selection.role`; the `schema` row names the provenance file field; the `state_conflict` refusal-site cell "Any other failed precondition at commit"
  becomes "Any other failed precondition at commit; also, at the snapshot before the lock, an `activate` `--expect-job NONE` mismatch (a job is active) and a
  `takeover` `--run-id` that names no run of the job and role" (A-13, D149; the frozen EX-7 meaning is not edited). No row
  respelled, no new message.
- Pointer amendments (A-11 and new items): item 7 ("Not made by this round") is replaced by an item stating that the command,
  render, load, ledger and external-profile pointers are made by R15-A2 as inserted paragraphs and recorded in P2-7; new items
  for `execution-design.md` (F2 bullet only); for the by-reference amendments of `authoring-contract.md` (RA-1: the
  orchestrator options `--workspace`, `--registry`, `--job` become optional with a native all-or-none rule and the ledger form,
  authority RA-1 "R15 later adds a ledger form by its own contract amendment"; RA-7: the section list gains
  `orchestrator_state`, and "It states no execution state" applies to the database-free form only; RA-9: the closing paragraph "R15 widens the closed factory to admit these verbs under
  `with-profile` only by its own reviewed amendment" is superseded by D134 option A, namely R15 does not admit them and a later
  round, provisionally named RA02 here, admits them under `with-profile`, decides which reads are exempt and carries the
  shape-catalog narrowing-risk row in its invariant table, and the same sentence of design RA-9 at `execution-design.md:499` is
  amended by reference; RA-9 items 1, 4 and 6 are also amended by reference, each "valid until the R15 integrated gate": item 1 (the
  embedded probe holds only the five X08 families) reads ten families, item 4 (the with-profile catalog is five shipped rows, its
  completeness rule stays five and the embedded rule is eleven) reads ten, ten and sixteen (authoring bundle), and item 6 (the
  with-profile catalog holds five shipped commands, so external capacity is unchanged) reads capacity 54); and a statement that
  `authoring-contract.md`, `acceptance-matrix.md` and `v5-alignment.md` are not edited by this round.
- Traceability. Existing rows are extended in place and never duplicated: R15-2 (append P2-1, D104, D111), R15-3 (append
  P2-1, P2-7, D136), R15-9 (append A-3 at `#part-1-amendments-r15-a2`, D118 and the exposure list of P2-1), E1 (append P2-1,
  D105, D106), E2 (append P2-1 and the D104 spelling), E3 (append P2-1, P2-6, D108, D142), E5 (append
  [P2-3](#p2-3-work_exec_v1) and D120), E11 (append P2-1, D118) and E16 (append P2-1, D114). R15-1 is unchanged. New rows,
  each once: R15-4 and R15-5 (P2-1, D107, D114), R15-6 (P2-2, P2-3), R15-7 (P2-1, D115, D116), R15-8 (P2-4, D121 to D123,
  D146), R15-10 (names the format3 event summary with payload v2 and typed reference columns, the format3 `--at-event`
  state and the `--event` selector, linking [P2-5](#p2-5-history-event-selector-and-request-results); INVARIANTS I49 to I54,
  I87 to I89), R15-11 (P2-6, D128 to D130, D148), A29, A34, E6 (P2-4) and a Part 1 amendments row
  (`#part-1-amendments-r15-a2`). Any existing row that links `#part-2-placeholders-r15-a2` is re-pointed (A-12).
- Acceptance checklist: add items for status explain separation (A20/A22), `claim` retry bytes (A23), history `--event`
  (A29), provenance unknown rule (A34), and the carried obligations F3 (render-level insertion-order byte test), F4
  (mandatory-file identity test), F5 (change between listing and first read), F6 (probe set stays five), F7 (envelope
  comparisons to literals), F8 (no-ledger snapshot beyond the scratch root), each named as an obligation of R15-B to R15-E.
- Decisions table: append D104 to D152 as in section 2 with their tags; no existing row is edited.

## 5. Part 1 amendments (each listed, reasoned)

Listed in the contract section `## Part 1 amendments (R15-A2)` (package P3) with location, change and reason.

| ID | Location (HEAD lines) | Change | Reason |
| --- | --- | --- | --- |
| A-1 | 1 and 3-9 | Title becomes `(parts 1 and 2)`; Status paragraph says Part 2 is filled by R15-A2 | the file now holds both parts |
| A-2 | 518, 527-528 | forward references to Part 2 re-pointed to P2-1 and P2-5 | placeholders are replaced |
| A-3 | 540 | "in any packet other than the claim or takeover packet" becomes "in any packet, and in the claim and takeover results only in the field `run.token`" | F-B: packets are retained records and are expanded by `--event`; this tightens the token-exposure rule |
| A-4 | 594, 646 | "defined by R15-A2 in Part 2" and "belong to R15-A2" re-pointed to P2-1, with the added statement that P2-1 defines the sets of `claim` and `takeover` and that the sets of `refresh`, `checkpoint`, `release`, the two check commits and `verdict` are defined by the R16 to R18 contract rounds | placeholders are replaced; the other operations are reserved and have no request body, facts or response in this part, so pinning members now would precede their lifecycle and event shapes (D150) |
| A-5 | 702 | Explain pointer re-pointed to P2-1 (`role_detail.explain`) | placeholders are replaced |
| A-6 | 724 | work_exec_v1 forward reference re-pointed to P2-3 | placeholders are replaced |
| A-7 | 730 | "Which verbs are exempt reads is a Part 2 clause" re-pointed to P2-1 (D108) | placeholders are replaced |
| A-8 | Messages, `revision_conflict` row, Execution sites cell | "The format3 load compare-and-swap only" becomes "The format3 load compare-and-swap and the `--expect-revision` check of `activate`" | R15-A1 I66 scoped the cell deliberately because `activate` was not yet specified; P2-1 now specifies it and reuses the code and message (D111) |
| A-9 | Messages, the two `reference` rows, Execution sites cells | name the field values `selection.job` and `selection.role` | P2-1 reuses the existing sites at the execution verbs (D110); the messages are unchanged |
| A-10 | Messages, `schema` row, Execution sites cell | names the provenance file field `provenance` | P2-4 adds the provenance file as a `schema` site (D110) |
| A-11 | Pointer amendments, item 7 | "Not made by this round ..." is replaced by an item stating that the pointers are made by R15-A2 | the pointers are made by this round |
| A-12 | Traceability and Pointer amendments rows linking `#part-2-placeholders-r15-a2` | re-pointed to the P2 anchors | the placeholder heading no longer exists |
| A-13 | Messages, `state_conflict` row of the execution refusal sites, cell "Any other failed precondition at commit" | becomes "Any other failed precondition at commit; also, at the snapshot before the lock, an `activate` `--expect-job NONE` mismatch (a job is active) and a `takeover` `--run-id` that names no run of the job and role" | P2-1 uses `state_conflict` at two sites before the lock (D111, D114); the frozen EX-7 meaning ("changed between snapshot and commit ... retry with the same request ID") and this cell cover neither; an `--expect-job JOB` mismatch is `no_active_job` (EX-7 fits it exactly), not a site; EX-7 stays byte-identical and gains no code or message; limit: a same-ID retry cannot succeed for either site (D149) |

Nothing else in Part 1 changes. The SQL block, tables M and N, EX-7 and EX-8 tables stay byte-identical. The earlier
revision's A-8 (a correction of the placeholder wording) is withdrawn: the placeholder at line 898 is true; the false wording
is only in the round record, which package P5 corrects.

## 6. Test strategy, hard-to-observe properties and platform

This is a documentation round: no product build, test run or Docker run; the observers are the contract checker
(`tests/native/v5-02-r15-a2-evidence/checks/contract_checks.py`), the link check and the redaction check. "Hard-to-observe"
properties are text properties; they are observed by literal-anchored checks (each mutant changes exactly one literal or
removes one clause) and by diff-against-snapshot checks for "unchanged" claims. The mutation prober (a separate stage, scratch
copy outside the checkout) applies each mutant in INVARIANTS.md. Coders never mutate the checkout. Platform: no platform
branch (Markdown and Python 3 only). Enforcement placement: the checker runs from a scratchpad copy against in-repo frozen
inputs, not in place. Budget composition, threat model and diagnostic mapping are the contract content itself (sections 4.1
to 4.6), checked by rows I20 to I47, I84 to I91, I101 to I105 and I107 to I111.

expectRed rule (G12a). A check is `expectRed` true only when it requires new content that the pre-edit snapshot lacks, so it
FAILs on the snapshot. A check that passes on the snapshot (a pure diff of unchanged text, a tool self-test, the redaction
exit codes, the base snapshot completeness) is a guard and is marked `false`. The red-run FAIL count equals the number of
`CHK-A2-*` rows with expectRed true (INVARIANTS.md footer states both numbers); `LINK-A2-001` is red too but is counted
separately because it is a different script; `RED-A2-*` rows are workflow rows, not checker functions.

## 7. Evidence layout and order (G12b to G12d)

Folder `tests/native/v5-02-r15-a2-evidence/`:

| Directory | Content | Owner and order |
| --- | --- | --- |
| `plan/` | DECISIONS.md, INVARIANTS.md, `superseded-<n>/` | search stage (this stage) |
| `base/` | byte-exact copies of exactly ten files: the seven edited docs (`execution-contract.md`, `command-contract.md`, `render-contract.md`, `load-command-contract.md`, `ledger-contract.md`, `external-profile-contract.md`, `execution-design.md`) plus the three guarded docs `authoring-contract.md`, `acceptance-matrix.md`, `v5-alignment.md`, taken from the recorded base commit `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` with `git show cdcfe829c466bf6338815ff6f5b6ea90ffddc570:docs/vernacular/<file>` (never from the working tree and never from a moving HEAD, so concurrent shared-checkout work and this round's own commit cannot change the snapshot) | P1, wave 1, before anything is edited |
| `checks/` | `contract_checks.py`, `link_check.py`, `fixtures/` | P1, wave 1 |
| `red/` | `snapshot-identity.txt`, `tree-verify.txt`, `red-run.txt`, `red-run.exit`, `link-red.txt`, `link-red.exit`, `RED.md` | P1, wave 1 |
| `probes/` | `baseline.attempt-<n>.log` (the unmutated positive control, written before any mutant), `<id>.attempt-<n>.log` per mutant plus `PROBES.md` | the mutation prober stage (not a package), after wave 4 and before P6; it works in a scratch copy and writes only here |
| `green/` | `checks-green.txt`, `selftest.txt`, `linkcheck.txt`, `FIN.md`, `identity.txt` | P6, wave 5, after the prober stage, in the order of the P6 sequence below |
| `raw/` | diffstat and guard confirmations (before FIN.md); redaction exits and control (the only files written after the redaction run); any re-run logs | P6, wave 5 |
| `README.md` | layout description | P5, wave 4 |

Hashes. The snapshot hash for the red run and every identity table in the round record come from the workflow
`identity.py` and are pasted verbatim, never hand-computed. P1 runs `identity.py` on `tests/native/v5-02-r15-a2-evidence/base`
(after taking the snapshot, before any edit) into `red/snapshot-identity.txt`, followed by the line `base commit: cdcfe829c466bf6338815ff6f5b6ea90ffddc570` and the verbatim output of `git rev-parse --verify cdcfe829c466bf6338815ff6f5b6ea90ffddc570^{commit}` (never HEAD). P5 runs it after all contract edits, over the
seven edited docs, `checks`, `base` and `plan` only (not over the round record, README, `red/`, `probes/`, `green/` or
`raw/`, which change after P5), and pastes the output into the round record. P6 runs it over the seven edited docs, the three guarded docs (`authoring-contract.md`, `acceptance-matrix.md`, `v5-alignment.md`) and this round's evidence data files only (`plan`, `base`, `checks`, `red`, `probes`, `green` without `identity.txt`), never over `docs/vernacular` as a whole, so the committed `green/identity.txt` pins no hash of another round record, review or the roadmap; the output is produced into the scratchpad first and copied to `green/identity.txt` so that the file does not hash itself (not pasted into the record). Raw logs are never deleted; a re-run keeps the earlier log as
`<id>.attempt-<n>.log`.

Red-run tree (standing lesson 5). The red run does not read the live shared checkout. P1 builds a scratch tree outside the
checkout containing only the bytes of `base/`: `<scratch>/redtree/docs/vernacular/<file>` and `<scratch>/redtree/base/<file>`
for the ten files, each verified byte-equal to `base/<file>` with `cmp` (exit statuses in `red/tree-verify.txt`). The checker is
run as `python3 -I <scratch>/checks/contract_checks.py --repo <scratch>/redtree --base <scratch>/redtree/base --git-repo <repo root>`;
`--git-repo` and `--base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570` are used only by the guard that compares `base/` with `git show <base-commit>:docs/vernacular/<file>` (row I100); HEAD is never read, because it moves when this round is committed or when another session commits in the shared checkout. The link check is run as
`python3 -I <scratch>/checks/link_check.py --repo <scratch>/redtree --git-repo <repo root> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570`; a link target outside the ten files is
resolved from `git show <base-commit>:<path>` of the same repository. `red/snapshot-identity.txt` is the verbatim `identity.py` output over
`base/` followed by the base commit line, so the identity covers exactly the bytes the red run read (the
ten files, plus the base-commit blobs the link check names). The commands in `red/RED.md` write scratch paths as `<scratch>`. The expected red-run result is `SUMMARY pass=7 fail=99`. On the final
text (the green run) the same command uses `--repo <repo root>`, `--base <repo>/tests/native/v5-02-r15-a2-evidence/base`, `--git-repo <repo root>` and the same `--base-commit`.

Probe positive control (row I106). Before any mutant, the prober stage copies the final tree to a scratch directory outside the
checkout and runs the unmutated copy: `baseline.attempt-<n>.log` must show every check `PASS`, `SUMMARY pass=106 fail=0`, and a
passing `--selftest`. Each mutant then runs against its own fresh copy of the final tree; its log `<id>.attempt-<n>.log` records
the mutation as a non-empty diff (a replace that changed no byte is invalid, not a survivor), the `FAIL <expected check>` line
that killed it, and a `SUMMARY` whose total equals the baseline total (106), so a filter miss or a missing runner cannot read as
a kill. `probes/PROBES.md` lists, per mutant id, the check that killed it and its attempt log.

P6 sequence (fixed, standing lesson 6 keeps every log): (1) green logs `checks-green.txt`, `selftest.txt`, `linkcheck.txt` with their `.exit` files; (2) `raw/diffstat.txt` and `raw/guard-unchanged.txt`; (3) `green/FIN.md`, which lists each log and figure and states that the redaction outcome is in `raw/redact-exits.txt` and `raw/redact-control.txt`, written after it; (4) `identity.py` over the pathspec above, written to the scratchpad and copied to `green/identity.txt`; (5) redaction check, one invocation per staged file (the seven edited docs, the round record, `README.md` and every text file under `plan/`, `checks/`, `red/`, `probes/` and `green/`, `FIN.md` and `identity.txt` included), exit codes into `raw/redact-exits.txt`, then the positive control into `raw/redact-control.txt`. Those two raw files are the only files written after the redaction run; if any file fails redaction nothing is edited by P6: it reports the file and the coordinator re-runs from step (3).

Wave order: W1 P1; W2 P2 and P4 (disjoint files, parallel); W3 P3; W4 P5; prober stage; W5 P6 (final logs written last,
nothing edited afterwards). The roadmap update is the coordinator's after approval and is in no package.
