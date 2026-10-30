# V5-02-RA01-A gate report (docs-only round)

Gate runner, 2026-10-30. No source file was edited. Raw logs are under `raw/` (files prefixed `gate-` are this run; the earlier `check.log`, `diffstat.log`, `identity.txt`, `redact.log`, `status.txt` were left untouched).

## Candidate identity

Output of identity.py (pathspec `docs CLAUDE.md`), verbatim from `raw/gate-identity.txt`:

| identity | value |
|---|---|
| HEAD | `a3364b8afae5ec7a75f2b19474df3fc89de9b078` |
| sha256(git diff --binary HEAD -- docs CLAUDE.md), tracked only | `08f488e5dc385cc98e1949cf0a85510f4f8965c0d30801d25fdc4eaeda0f937a` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs CLAUDE.md, evidence excluded | `5e0b55ce61117dabe0fea8d9ea8546b35aef37efb933ca787fe2671d4219e14a` |

Per-file sha256: CLAUDE.md `f9cce397...195e`; acceptance-matrix `18c850dc...4965`; command-contract `ffdb020a...a62a0ede`; external-profile-contract `1377d5de...fcb39b35d`; load-command-contract `d72870d6...a30d3`; render-contract `5695452c...d629b`; v5-alignment `837688c0...a634543`; authoring-contract (untracked) `283510a3...9e53`; round record (untracked) `24c81075...aa176eaa22`. Full hashes are in `raw/gate-identity.txt`.

## Commands

| command | exit | result |
|---|---|---|
| `scripts/redact-local-paths.py redact` on the nine explicit files | 0 | "redacted 0 files" |
| `scripts/redact-local-paths.py check` on the same nine explicit files | 0 | no output (clean) |
| `test -L AGENTS.md` | 0 | symlink, target `CLAUDE.md` |
| `git diff --stat` | 0 | 7 tracked files, 29 insertions, 4 deletions: CLAUDE.md, acceptance-matrix, command-contract, external-profile-contract, load-command-contract, render-contract, v5-alignment. Untracked: authoring-contract.md and the round record. Only owned files appear. |
| `ctest --test-dir build/rewrite-debug -N` (inventory only, no build) | 0 | Total Tests: 1273 |

Matched tests run: none, because the round changes only Markdown and `CLAUDE.md`; the round record and CLAUDE.md state that a documentation-only round needs no product build, so no tests were executed (0 run, 0 skips, 0 failures). The zero-work (no-change build) check was not run: no build was performed in this round, so it is not meaningful. This is a stated limitation, not a pass claim. I did not rebuild, so the inventory is that of the existing debug build tree, which pre-dates and is unaffected by this docs-only diff (`git diff --stat -- src tests CMakeLists.txt` is empty).

## Test inventory

`inventory.txt` lists all 1273 registered test names (sorted). Compared with `tests/native/v5-02-x07-c02-evidence/inventory.txt` (the most recent committed inventory, a3364b8a, 1273 names, sorted for comparison): identical. New names: none. Removed names: none.

## (a) Acceptance criteria mapping

This round has no code, so no tests map to its criteria; each criterion maps to a document location and the check that covers it. No test names or test file:line exist for this round.

| criterion | evidence |
|---|---|
| 1 RA-1 to RA-9 exact text | `docs/vernacular/authoring-contract.md` sections RA-1 (line 46), RA-2 (104), RA-3 (136), RA-4 (161), RA-5 (229), RA-6 (262), RA-7 (312), RA-8 (353), RA-9 (361), Messages (371). Whether every design-section-4 test is writable without a design choice is a reviewer judgement; not machine-checked. |
| 2 traceability and decisions tables | authoring-contract.md Traceability (408), Decisions table (451); A35 and A36 rows in acceptance-matrix.md lines 57-58 |
| 3 pointer amendments consistent | authoring-contract.md Pointer amendments (392); diffstat shows additive edits in command-contract (+7), render-contract (+6), load-command-contract (+4), external-profile-contract (+7). Cross-document consistency is a reviewer check. |
| 4 CLAUDE.md names contract; AGENTS.md symlink | CLAUDE.md +1 line; `test -L AGENTS.md` exit 0 |
| 5 no narrowing, no changed diagnostic, no R15-R19 | diffstat: 29 insertions, 4 deletions across 7 files (deletions are the A35/A36 evidence cells and the v5-alignment lines); reviewer check of the deleted lines |
| 6 citations and links | not machine-checked in this gate; the author record claims a manual link check. Reviewer must verify. |
| 7 redaction check exit 0 | redact exit 0, check exit 0 on the nine explicit files |

No corrective findings are closed by this round.

## (b) Red-first

None. No regression tests were added (documentation only). No red snapshot, command or hash exists, and none is invented.

## (c) Deliberate behaviour differences

No product behaviour changes. The round specifies, but does not implement, the eleven-row catalog, the amended root help and two new opaque path slots; these are contract amendments recorded in authoring-contract.md "Pointer amendments" and "Messages" (E16 amendments per the round record). The author record lists NEW engineering choices for the reviewer: handler IDs with underscores, per-verb `include` mapping, job-reading orchestrator capture service, observe bounds, critique rule set, envelope `inputs` shape.

## Limitations

- No build, test run or no-change build check (docs-only round). Inventory comes from the pre-existing build tree.
- Citation and link correctness are not machine-verified by this gate.
- passed=true is judged on the docs gate commands (redact, check, symlink, diffstat) and the inventory listing.
