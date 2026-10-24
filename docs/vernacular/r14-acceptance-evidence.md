# R14-D8 acceptance evidence candidate

D8 is verification only and pending independent review. This table maps the R14
portions of the v5 rows; it does not close execution, authoring, Linux or external
profiles. The mandatory external-profile gate still follows full R14 acceptance.
Raw contemporaneous baseline, command records, JUnit and LastTest logs reside in
[the D8 evidence directory](../../tests/native/r14-d8-evidence/).

## Crash recovery boundary

The two early native process-death boundaries (before the first journal write,
and before the first journal sync) leave a non-hot journal remnant. The original
main database is byte-identical. Ordinary open and a same-key acceptance retry
refuse `recovery_required`; explicit `recover_quiescent` refuses `corrupt` with
`Recovery requires a valid hot journal header`. Each refusal preserves every
scratch file's bytes. There is **no supported automated recovery/retry demonstrated
for these non-hot remnants**. Tests neither delete a sidecar nor open the original
with raw SQLite to manufacture recovery. This is the conservative boundary in
[the ledger contract](ledger-contract.md#initialization-failure-safety-export-and-format-evolution)
and [R12-C](rounds/R12-C.md), not a claim of universal crash recovery.

Later real database-write and database-sync interruptions recover explicitly from
a recognized hot journal to the old whole acceptance. Death after returned
completion retains the whole committed acceptance and exact saved-byte replay.
The new native tests exercise `complete_acceptance`, isolated subprocesses and a
scoped SQLite VFS; they do not prove public CLI VFS faults or hardware power loss.

| Row | Concrete R14 evidence | Limits / future owner |
| --- | --- | --- |
| A01 | Full retained vector/determinism suites; D5 `load capture prepares complete multi-job closure and exact trusted provenance without source trees`; public `replacement.load_cli` initial/identical/restore preserve exact source and registry/stdlib references. | R19 execution determinism remains open. |
| A02 | Existing scoped renderer material mutations; D5 complete closure and retained receipts; D8 public semantic edit changes W and source while comment/prose leave W equal. | Accepted-role selection/reconciliation is R15/R16. |
| A03 | D8 public prose-only edit changes render bytes with W stable; restore returns original render and W. Existing preview rationale/checkpoint tests remain. | Retained execution refresh/recovery is R16. |
| A04 | Existing renderer/schema materiality and illegal-wrapper refusals; D5 `load capture trusted all-job validation preserves unselected and duplicate logical diagnostics`; same retained interpretation is composed by public load. | Authoring wrapper is RA01. |
| A05 | Existing renderer own-output/W separation preserved by the full suite. Load does not capture implementation/source trees. | Check applicability is R17; load adds no execution. |
| A06 | Existing exact preview review targets remain preserved; load grants no approval. | R18 exact-target verdicts and approvals remain open. |
| A07 | Existing mandatory render-budget refusals and D6 cumulative conversion/resource tests retained. | RA01 and execution packet budgets remain open. |
| A08 | Existing sandbox, frozen retained catalog, capability/service sequencing, caught exhaustion and C01 provenance suites; D7 parser/replay tests and complete exact-source variant audits. | Mandatory external-profile X still unexecuted; no execution authority. |
| A20 | Public invalid unselected job, missing reference and wrong constructor leave zero revision/request and only initialization event; native CAS/final receipt tests refuse without changing head/history. | Draft-status/active-run reads are R15/R16. |
| A23 | New D8 `complete acceptance SQLITE_FULL enumerates journal and database writes with reusable request` compares every table count after rollback and retries same key; five process boundaries above; existing busy/commit-hook/prefix/capture tests; public lost-response and stdout recovery. | Early non-hot remnants have the explicit limit above. Native injection is not a public CLI disk-full proof. Retained packets/cache rebuilding remain R15+. |
| A25 | Public initial → identical noop → comment → prose → semantic change → restore gives revisions 1,1,2,3,4,5; parents none,none,1,2,3,4; events 2–7; each stdout equals exact stored request bytes. | R19 trajectory preservation remains open. |
| A26 | New native process boundaries inspect whole head/revision/event/request state, plus full-table disk-full rollback; existing locked-CAS, same-ID equality/conflict, competing-head, late replay, busy and capture-race tests retained. | No universal recovery claim for non-hot remnants; no hardware power-loss proof. Role activation/reconciliation remains R15. |
| A27 | Existing C3-B2 `atomic targeted collisions refuse file directory interpretation and request without prefixes` already drives all four kinds through real `complete_acceptance`, unchanged head and no partial rows; exact-source audit retained. | No duplicate artificial public collision test added; future execution record paths require own proof. |
| A28 | Public authored comment/prose bytes create distinct source revisions with stable W, semantic edit changes W, exact restore reuses original source reference. Closure export excludes an unrelated sibling and preserves an executable declaration's mode. Existing tree tests retain optional/deletion/rename/mtime semantics. | Accepted-input reconciliation and evidence staleness are R15–R17. |
| A29 | Public original revision/event/request data and exact original source export still succeed after the entire workspace/registry tree is removed, with both public binary and changed-current-policy probe; saved initial load response replays byte-exactly at advanced head. | Execution state/evidence references and full recovery trajectory are R15–R19. |
| A31 | Existing load all-job invalid/duplicate ID diagnostics, malformed route and native membership tests; public invalid unselected job refuses without activation. | Accepted role mapping and scheduling remain R15+. |
| A36 | Existing typed logical-field-path tests in D5, authoritative native diagnostics and D6 failure propagation; public load refusal preserved. | Raw diagnostics are not an authoring diagnostic prompt. RA01 wrapper remains open. |

Exact execution counts, inventory preservation, artifact identities and verification
limits are recorded in the D8 author report. Independent review determines whether
this contract-conformant safety evidence closes the applicable R14 portions.

Final candidate verification executed **402/402 tests in both debug and release,
with zero failures or skips**, preserving all 400 prior independently reviewed
names. Both configurations passed formatting/Doxygen, canonical and record
vectors, every historical/fixed-export/D6/C01/D7 exact-source audit and a final
zero-work build. Debug's first no-work attempt performed a timestamp-triggered
rebuild after a byte-identical source restoration; the repeated check performed
no work. The raw attempts and this limit remain in the evidence report.
