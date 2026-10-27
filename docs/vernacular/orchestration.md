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
| Coder | Haiku 5.5 | high | Implement only the assigned package's files (test-first or implementation role) |
| Red-check runner | Sonnet 5.5 | low | Prove test-first tests compile and fail on assertions before implementation |
| Gate runner | Sonnet 5.5 | low | Run the agreed build/test commands serially and report raw results |
| Mutation prober | Sonnet 5.5 | medium | Break each acceptance invariant in a scratch copy and report surviving mutants |
| Plan reviewer | Opus 5.5 | high | Check the plan and invariant table against the contracts before any code |
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
2. Search maps the scope and writes the round's **invariant table**: one row per
   acceptance criterion and closed finding, each naming the exact mutant that
   breaks it and the test that must fail under that mutant. Tests are designed
   against their mutant; a row that cannot be observed without a new public
   field or fault hook says so instead of getting a hollow test. Search then
   splits the work into packages with exclusive file ownership per wave. Its notes
   open with a **decisions table**: every design choice the round implies, marked
   contract-decided (with citation), decided in
   [remaining-steps design](remaining-steps-design.md), or new (operator decision
   Q18, 2026-10-27).
   **Plan review** (operator decision, 2026-10-27): before any code, an Opus plan
   reviewer checks the plan against the governing clauses: no clause narrowed,
   every clause and finding has a row, every design choice is decided by a cited
   source. Gaps go back to search for one revision. A new product or authority
   choice stops the run with an operator question. During the day the operator may
   ask for a **checkpoint**: the run stops after plan review and resumes, with the
   operator's notes passed to every coder, once the operator approves the plan.
3. Rounds that add new behaviour run **test-first**: test coders write the tests
   from the contract and the table, plus compiling stubs (real declarations,
   not-implemented bodies), and a red check proves each new-behaviour test
   compiles and fails on an assertion (`red/RED.md`, with one raw log per test).
   Implementation coders then replace the stubs with the test files read-only;
   a test that looks wrong is reported, never edited. Rounds that only add tests
   to existing code, or repair, skip the red check. Within a wave, coders run in
   parallel only when their files are disjoint; coders build only when no other
   coder is running.
4. The gate runner builds and tests once all packages finish. On failure, one
   repair coder (Sonnet 5.5, medium) that owns the round's files may build and
   iterate on the debug tree alone, then the gate reruns; a second failure stops
   the run. A round whose changes are applied but whose gate failed resumes as a
   `repair` run: no new mapping, just the repair coder and the gate.
5. A mutation prober (Sonnet 5.5, medium) applies each invariant-table mutant,
   plus any invariant the table missed, in a scratch copy outside the checkout,
   names the mutant that fails each new test (red-first), and records the
   results with one raw log per mutant
   (`tests/native/<round>-evidence/probes/PROBES.md` and `probes/raw/`). The prober
   keeps one persistent warm scratch build and rebuilds only the test targets each
   mutant needs. Corrective rounds carry forward earlier rows whose files are
   byte-identical and re-run only changed, new and finding rows; the re-probe after a
   survivor repair re-runs only the survivors and rows the repair touched.
   Exploratory mutants beyond the table are capped at 20, with skipped ones listed.
   Earlier
   probe generations move to `probes/superseded-<n>/`; raw logs are never deleted.
   Survivors go back to the repair coder, then the gate and prober rerun once.
   Only the prober applies mutants: coders and repair coders never mutate, stash
   or revert files in the checkout or `build/rewrite-debug`, and do not add public
   product fields only for observability unless the round record allows it.
6. Before review, the coordinator syncs the round record to the final candidate
   (identity hashes, test counts, probe results, repairs and amendments), so no
   section describes an earlier candidate. Identity tables come from the driver's
   identity tool (HEAD, diff hash and per-file hashes including untracked files,
   each naming its pathspec); agents never hand-write hashes.
7. The reviewer checks the invariant table (every criterion has a row, each
   mutant breaks its invariant and is killed by its test, test-first rows were
   red) and returns `approve`, `request-changes` or `blocked`, writing
   `rounds/<id>-review.md`.
8. The coordinator records the outcome, updates the roadmap's current status and,
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

Records-only fast path (operator decision, 2026-10-25): when the reviewer marks a
`request-changes` as records-only (every blocking finding is fixed by editing round
records or evidence READMEs, with no change to source, tests or evidence data), the
coordinator fixes the records in the same run and the reviewer re-checks only those
edits; an approval then commits as usual. Anything else still starts a corrective
round. On approval, the coordinator also applies records-only advisories before
committing and carries code advisories into the next round's brief.

## Commits

Operator authorization, 2026-10-24. After an `approve` review and the
current-candidate check, the coordinator commits the round on
`dev/planar-v2-vernacular`: explicit paths only (never `git add -A`),
`git commit -s`, no Claude co-author trailer, and a subject naming the round (for
example `V5-02-X06-A4: profile preparation architecture`). The baseline commit
(`897617fc`) holds everything through X06-A4+C01.

On 2026-10-24 local history was rewritten once to redact machine-local paths
(nothing had been pushed). Round records written before then cite the original
SHAs: `aa3f95d1` → `897617fc` (baseline), `6c30ecdb` → `260d2fe1`,
`ff672595` → `b5174145` (X06-B1+C01). Trees differ only by the redaction recorded
in `tests/native/path-redaction.json`.

Evidence folders (`tests/native/*-evidence/`) are gitignored. Commit only each
folder's `REPORT.md`/`README.md` and top-level `.md`/`.json` manifests of 100 KB or
less, staged by explicit path with `git add -f`; bulky artifacts stay local and
the committed manifests record their hashes. Support folders and frozen snapshots
are committed normally.

Committed files must not contain machine-local absolute paths. Before staging,
run `python3 scripts/redact-local-paths.py redact --repo-path "$PWD" --home "$HOME"`
on the files to be committed, then the same command with `check` on the staged
files; it must exit 0. Redaction replaces the repository root with `<repo>` and the
home directory with `<home>` and records original/redacted SHA-256 values in
`tests/native/path-redaction.json`, so `verify` can recheck hashes pinned before
redaction. Never commit unreviewed or `request-changes` candidates.
Pushing, merging and installing remain unauthorized.

## Status records

When a round changes status, replace the roadmap's current-status section and
append to the round record. Keep status short: what was approved, what is pending
review, the latest full gate and what remains open. Status never goes in
`CLAUDE.md` or reference docs.
