# Integrated R15–R18 code review

Requested by the operator after the implementation checkpoint. Candidate:
`ae02a12bf77c6b72d3060253dd47800c645796a7`; comparison base `1338637c`.
The checkout was clean when review began. Reviewers may inspect surrounding
code needed to evaluate changes against the exact execution contracts.

Three fresh independent reviewers cover (1) ledger/history/transactions,
(2) native runtime, CLI, capture and process integration, and (3) trusted Lua
policy and projection. Each owns only its report. Product code is read-only;
no fixes, installation, publishing or upstream merge are part of this review.
The Claude-specific model listed in the historical orchestration instructions
is unavailable in this environment; fresh available reviewers are used instead.

This is a code review, not the full validation gate. Existing source-built binaries
and isolated scratch reproductions may be used. Reviewers do not build concurrently
in the shared build directory or access installed Planar/operator databases.
The prior focused development checks are context, not proof of full acceptance.

Reports:

- `V5-02-R15-R18-review-ledger.md`
- `V5-02-R15-R18-review-runtime.md`
- `V5-02-R15-R18-review-policy.md`

## Outcome: request changes

Eight actionable findings: two P1 and six P2. All three independent reviewers
returned request-changes. Product source remains unchanged at the reviewed HEAD.

| Priority | Finding | Report |
| --- | --- | --- |
| P1 | Old approvals can reattach after reviewer recreation because verdict snapshots lose lineage. | policy F1 |
| P1 | Consumer completion can use a captured producer vector that became obsolete before commit evaluation. | policy F2 |
| P2 | Reviewer state does not consistently follow verdict applicability during load/check reconciliation. | policy F3 |
| P2 | Check blocker clearing evaluates pre-check facts, retaining an obsolete rejection blocker. | policy F4 |
| P2 | Recovery adapter discards retained evidence and findings. | runtime RT1 |
| P2 | Status request lookup misses open check reservations. | runtime RT2 |
| P2 | Check capture skips .git files/symlinks before inspecting their type. | runtime RT3 |
| P2 | Refresh/release losing to another release reports state_conflict instead of fenced with terminal state. | ledger L1 |

Policy findings were reproduced with the candidate Lua modules and synthetic
snapshots; native end-to-end sequences for those cases remain untested. Runtime
findings were reproduced with isolated CLI scenarios, with each report separating
tested behavior from source-proven consequences. The ledger race was reproduced
in a scratch test executable linked to existing candidate development archives.
Existing focused native tests passed but do not cover these failures. Artifact-to-HEAD
identity was not independently rebuilt by reviewers; source inspection supports
the cited failures. See each report for exact source locations and evidence limits.

No fixes were applied. The next step is a corrective implementation round addressing
these findings, followed by independent re-review and the remaining full validation
work. The unselected R19 real-project exercise remains outstanding.
