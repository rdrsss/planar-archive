# Rewrite orchestration

How work on the `dev/planar-v2-vernacular` rewrite is coordinated. This is process,
not codebase guidance: coding agents receive only the parts their brief needs.
Current status is the [roadmap opening](roadmap.md#current-status); round records
live in [rounds](rounds/).

The driver is a Claude Code workflow (`vernacular-round`) that advances exactly one
round per run. Old Planar tools, queues, skills and agent roles are not used.

## Roles

Operator decision, 2026-10-24. Every round uses fresh agents.

| Role | Model | Reasoning | Responsibility |
| --- | --- | --- | --- |
| Coordinator | Sonnet 5.5 | medium | Select the round, write its record, scope ownership, record outcomes and commit |
| Search | Sonnet 5.5 | medium | Read-only map of the exact scope with source citations, split into work packages |
| Coder | Haiku 5.5 | high | Implement only the assigned package's files |
| Gate runner | Sonnet 5.5 | low | Run the agreed build/test commands serially and report raw results |
| Reviewer | Opus 5.5 | high | Independently review candidate and raw evidence |

Escalation: a round's coders use Sonnet 5.5 (medium) instead of Haiku when the
round has already received two `request-changes` reviews, or when any package
touches `src/engine/runtime/` or `src/engine/ledger/acceptance*`. Do not otherwise
substitute models. Rounds before this decision used `gpt-6-luna` search and
`gpt-6.1-sol` coder/reviewer agents; their records keep that history.

## Round sequence

1. The coordinator records the round's objective, dependencies, owned files,
   acceptance criteria, starting revision, pre-existing changes and verification
   in `rounds/<id>.md`.
2. Search maps the scope and splits it into packages with exclusive file ownership.
3. Coders implement their packages. They run in parallel only when owned files are
   disjoint (search's split is the documented reason); otherwise sequentially.
   Coders do not build.
4. The gate runner builds and tests once all packages finish. One repair pass is
   allowed; a second failure stops the run.
5. The reviewer returns `approve`, `request-changes` or `blocked`, writing
   `rounds/<id>-review.md`.
6. The coordinator records the outcome, updates the roadmap's current status and,
   on approval, commits.

All agents work in the main checkout on `dev/planar-v2-vernacular`; no separate
worktrees. Every brief states that the checkout is shared, assigns explicit file
ownership and forbids reverting others' work.

## Review rules

Coders never approve their own work. Requested changes start a fresh corrective
round (`-C01`, `-C02`, …) with new agents, preserving existing findings and the
candidate. Record an exact candidate snapshot or diff identity before review.
Approval lets the coordinator advance ordinary rounds without repeated operator
permission after checking that the reviewed candidate is still current. Edits
affecting approved scope need renewed review. Unresolved product choices return
to the operator.

## Commits

Operator authorization, 2026-10-24. After an `approve` review and the
current-candidate check, the coordinator commits the round on
`dev/planar-v2-vernacular`: explicit paths only (never `git add -A`),
`git commit -s`, no Claude co-author trailer, and a subject naming the round (for
example `V5-02-X06-A4: profile preparation architecture`). The baseline commit
(`aa3f95d1`) holds everything through X06-A4+C01.

Evidence folders (`tests/native/*-evidence/`) are gitignored. Commit only each
folder's `REPORT.md`/`README.md` and top-level `.md`/`.json` manifests of 100 KB or
less, staged by explicit path with `git add -f`; bulky artifacts stay local and
the committed manifests record their hashes. Support folders and frozen snapshots
are committed normally. Never commit unreviewed or `request-changes` candidates.
Pushing, merging and installing remain unauthorized.

## Status records

When a round changes status, replace the roadmap's current-status section and
append to the round record. Keep status short: what was approved, what is pending
review, the latest full gate and what remains open. Status never goes in
`CLAUDE.md` or reference docs.
