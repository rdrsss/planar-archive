# R09 coder report

Implementation complete; independent review is pending. No self-approval.
Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.

## Cleanup and preserved boundaries

Removed 7,775 exact tracked/untracked inventoried legacy files and 25 separately
inventoried ignored legacy vendor source files. Their original path/type/SHA256
and confirmed absence are in `deleted-inventory.json`,
`supplemental-ignored-deletion-inventory.json` and `deletion-check.json`.
Legacy engine/library/command fleet, migrations, agents/skill/workflows, templates,
evals/examples, unused vendors, installers/releases, old gate scripts/CMake
helpers, obsolete public docs and CI/Docker entrypoints are removed. Generic
GitHub templates remain; stale security/bug-template claims are refreshed.

The active product source/policy/tests/fixtures were not modified. All 1,117
initial protected product/test/fixture/dependency files match after the gates.
R08-A-C01 20/20, unchanged R08-B 23/23, and C01 3/3 candidate hashes match.
Ignored local settings, caches, root bin/build outputs, installed files and
operator data were not deleted. AGENTS.md remains a symlink to CLAUDE.md.
No Planar tools, queue, delegation, commits, install, publication or merge.

Public docs now describe actual render-only behavior, explicit roots/registry,
captured versus context-only packets, unverified assertions, no execution/state,
resource/host limitations and unavailable distribution. Active instructions and
notices match the graph. Vernacular changes are confined to the authorized
status-only opening paragraphs of declaration/render contracts. Other planning
and historical evidence are preserved; historical links to removed source
remain citations of the original tree, not active instructions. Coordinator owns
R09 record and roadmap.

## Verification

Direct serialized commands via `run-gates.py`, raw output in `debug-gate.log`
and `release-gate.log`, complete CTest output in matching `*-ctest-last.log`.

| Command | Result | Test time | Full elapsed |
| --- | --- | --- | --- |
| `make test-all PRESET=debug` | 113/113, zero failures/skips; format/Doxygen25 files; no-change build no work | 61.90s | 63.44s |
| `make test-all PRESET=release` | 113/113, zero failures/skips; format/Doxygen25 files; no-change build no work | 44.15s | 56.57s |

The registry is 112 native Catch2 cases plus one Python acceptance case. Release
rebuilt 243 steps before its tests. Both configurations use committed offline
dependency sources; no configure-time fetching exists. `*-registry.json`
preserves exact registered names. `gate-results.json` records actual commands,
UTC starts, elapsed time and host. `host-tool-versions.json` records CMake4.4.4,
LLVM/formatter23.1.3, Ninja1.13.2, Python3.15.0 and Doxygen1.18.0.

`git diff --check` passes. 47 active relative document links resolve. Active
reference scan finds no stale source/CPM/legacy-command/scaffold claims (rg exit1
means no matches). The exact README captured-render example succeeds and its
complete JSON/stdout is retained in `scope-check-2.stdout`; opposite stderr is
empty. These commands/results are in `scope-check-results.json`.

## Candidate identity and limits

`candidate.sha256` snapshots 1,296 retained files, excluding this evidence
directory and coordinator-owned R09/roadmap files. `candidate-scope.json` records
its digest and symlink contract. `retained-delta.json` identifies 20 modified/new
retained files relative to the initial inventory; deletion manifests establish
exact removed bytes. Initial HEAD/status/diff preserve the already approved
rewrite edits. `evidence.sha256` covers this evidence set excluding itself.

Native debug/release macOS ARM64 only. No replacement Linux result or Linux CI
is claimed. Exact 16 MiB serializer limits retain native unit evidence; CLI
large-source probes reach the trusted instruction limit before that envelope
boundary. No atomic/hostile filesystem snapshot guarantee is added. Operator
render evaluation and future distribution remain separate gates.
