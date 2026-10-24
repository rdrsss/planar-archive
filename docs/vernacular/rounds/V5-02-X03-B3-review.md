# V5-02-X03-B3 independent review

Decision: **approve**. No blocking finding or requested change. This verification
closes the integrated native X03-B obligations and permits the coordinator to
advance to X04-A after checking that the reviewed candidate remains current.
It does not complete the mandatory external-profile runtime gate.

Reviewed on shared `<repo>`, branch
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Reviewer ownership is limited to this
report and `tests/native/v5-02-x03-b3-review-evidence/`. Product, existing tests,
author artifacts, contracts and status documents were read-only. No subagents,
Planar tools/skills/queue, installed executable, operator data, commit,
installation, publication or merge were used. No full test suite was duplicated.

## Reviewed scope and exact identity

Read the active guide, implementation plan, roadmap, v5 direction, B3 ownership
and search correction, exact X-STORE contract and bounded implementation path,
the X03-A/B1/B2-A/B2-B/C01 reviews, author README, coverage report, exact diff,
four new support scripts and raw gate/audit evidence. C01 independently closes
B2B-R1; its controlled metadata fixture limitation remains explicit.

Author paths below resolve under `tests/native/v5-02-x03-b3-evidence/`.

| Frozen item | SHA256 |
| --- | --- |
| 13-member `candidate-manifest.json` | `81f93dfb3a0179092df56e9ded03edf781d2f26ea467881e566c6103e9421ade` |
| `owned.diff` | `de9f90f8b3aebbd9d6a7e093bba4513ecf585ebb19130c8887ffe9387181500d` |
| 156-member `artifact-manifest.json` | `71f070efba709dab129f68832587eae8cea73adbc9b2fa13dca070cd3423d407` |
| `final-complete-filesystem.json` | `3ee8b2ce4e7869ab72cc1c727b0a87fd60f1007e05a732b4e66fb88977eec654` |

Independent audits verify every candidate member against both the working file
and exact candidate snapshot, every author artifact, complete artifact membership,
and a byte-identical independent reconstruction of the owned diff. The only
changes are eight assigned status documents, the new coverage report and four
new verification scripts. The status prose accurately records pending independent
B3 review rather than claiming self-approval. The roadmap correction that native
request-v2 is supported, while interpretation-v2 remains future work, is accurate.

The complete baseline contains 15,424 regular files; the final tree contains
15,425. No baseline file is missing. Exactly the eight allowed existing files
changed and the new coverage report was added outside B3 support/evidence.
Every archived original status file matches its baseline hash. The independent
current filesystem walk reproduces the final inventory, excluding only root
`.git/`, root `build/`, B3 support/author evidence and reviewer-owned additions.
Historical nested-build evidence remains included. Both separately recorded
symlinks retain their targets; `AGENTS.md` still points to `CLAUDE.md`.

The entire executable source/test/configuration candidate remained unchanged
through both gates. Per-configuration stability reports bind the full baseline;
the post-verification status prose is correctly distinguished from executable
verification. Both final build-identity manifests still match the tested binaries,
cache, compilation database and build graph. Current source members also match
the independently approved combined B2-B+C01 manifests, applying the C01 override
to `format.cpp` and its narrow CMake wiring.

## Full gate evidence

The author used the prescribed helper-equivalent sequence exactly once per
configuration: configure; build `all planar_tests`; exact inventory; full CTest
with retained absolute-path JUnit; immediate LastTest/CTestCostData copy; actual
case validation; pinned active lint/Doxygen; final no-work build. Command records
include exact argv, cwd, UTC times, elapsed time and zero exit status. Their times
are sequential, with debug preceding release. The commands agree with the current
gate helper, while preserving JUnit that the helper normally stores temporarily.

| Configuration, macOS ARM64 | Actual full run | Other gate evidence |
| --- | --- | --- |
| Debug, LLVM 23.1.4 | 430/430; 223.05 s | Zero failures/errors/skips; lint/Doxygen 98 active sources; 15 compiled native support files formatted; final all+planar_tests no work |
| RelWithDebInfo, same toolchain | 430/430; 119.37 s | Zero failures/errors/skips; same lint/support checks; final all+planar_tests no work |

Independent parsing verifies 430 unique actual JUnit cases in each run, every
status `run`, no failure/error/skipped element, and zero corresponding suite
counters. Names equal the respective inventories and the exact independently
approved C01 inventory. All 405 X01 names remain. Each copied LastTest contains
430 passing results; the raw full output confirms the totals and elapsed times.
The cache confirms Debug versus RelWithDebInfo, rather than inferring release
mode from its directory name. No focused selection is presented as a full run.

Raw LastTest tuples independently establish the matrix below for both full runs.
Each faulted ordinal has a positive injected hit and a failed native result;
the terminal ordinal has zero hits and succeeds. Source assertions require typed
`storage_full`, original table/history state, same-key retry and exact replay.

| Configuration / format | Journal SQLITE_FULL | Database SQLITE_FULL | Unique process boundaries |
| --- | --- | --- | --- |
| Debug /1 | 38 faults, 0–37; success38 | 15 faults, 0–14; success15 | 0,1,2,3,4 |
| Debug /2 | 38 faults, 0–37; success38 | 15 faults, 0–14; success15 | 0,1,2,3,4 |
| RelWithDebInfo /1 | 38 faults, 0–37; success38 | 15 faults, 0–14; success15 | 0,1,2,3,4 |
| RelWithDebInfo /2 | 38 faults, 0–37; success38 | 15 faults, 0–14; success15 | 0,1,2,3,4 |

Repeated buffered fork output is deduplicated by boundary. Boundaries0/1 preserve
the main file and non-hot sidecar bytes and deliberately refuse ordinary open,
explicit recovery and retry. Boundaries2/3 use explicit recognized-hot-journal
recovery before retry; boundary4 replays committed completion. These are injected
SQLite write/capacity and process-death checks under the retained cooperative
filesystem assumptions, not physical disk exhaustion, universal crash recovery,
hardware power-loss durability or malicious-journal safety.

## Coverage and independent sanity checks

The coverage report maps native X-STORE obligations to actual executed names and
source assertions. Independent membership checks confirm its exact test references
and local links. Source inspection confirms all 14 inherited native acceptance
cases select formats1/2, including concurrency, receipt ownership, busy COMMIT,
rollback, replay-before-CAS and unusable-candidate ordering. The actual format2
workflow uses native acceptance, removes original inputs, reads all four history
selectors, exports exact binary source and exercises new-target backup/restore
and replay. SQL reader fixtures are accurately limited to structural reader proof.

Typed v1/v2 coexistence, immutable request-v2 intent, complete codec-aware request
equality, forced collision rollback/redaction and fresh format/profile refusal
ordering remain the approved B2-B+C01 implementation. All three metadata boundary
tests run both formats and unchanged controls. Their fixture injects alternate
supported metadata after real verification; it establishes comparison/rollback
plumbing and does not reproduce external format changes or conversion.

Independent reviewer commands retained under
`tests/native/v5-02-x03-b3-review-evidence/` verify:

- Record-v1 and request-v2 byte/hash oracles: **10 + 2 pass**, with check-only
  execution and no vector regeneration.
- Five current storage/export variants in each configuration: exact source and
  generated hashes, substitution counts, forward generation and reverse recovery;
  archived generated files also equal current build-tree files.
- Eight fresh inherited graph audits, four per configuration: seven native/CLI
  families and fixed-export harness; six D6 variants; D6-C01; D7-B CLI graph and
  exact embedded original/assembly correspondence. Every independently regenerated
  report equals the author's report as parsed JSON.
- All **62** protected record-v1 codec, shipped stdlib/W and old fixture files:
  current hashes equal the pre-X03-A manifest.

The retained pre-B1 executable still hashes to
`be1c4cee1b358208831faa919707ce5fd01de3ea31bc63db333a027f8844464a`.
The original B1 proof has eight real history/load invocations against native-created
format2 clean/journal/WAL/SHM fixtures: exit3 `unsupported_ledger_version`, empty
stdout and equal complete recursive before/after directory snapshots. The B3 reuse
record matches that proof, binary and exact provenance/review file hashes. The B1
independent reviewer also reran the frozen executable. This is sufficient retained
old-binary refusal proof; B3 accurately claims reuse, with no fresh invocation or
pristine historical full-source rebuild.

The author discloses the initial raw-git-diff display UnicodeDecodeError and the
delayed baseline binary/cache capture. The saved binary diff and original complete
filesystem manifest remain available. The disclosed note is a transcript summary,
not contemporaneously captured stderr. The actual tested final binary/cache
identities remain verified. No success is inferred from the failed display step.

One initial reviewer audit stopped because it expected an older CTest summary
phrase. CMake 4.4.4 prints `100% tests passed out of 430`; JUnit validation had
already passed. The reviewer expectation was corrected and its limitation recorded
in `attempt-1-note.md`. No candidate or author artifact changed.

## Disposition and limits

Native X03-B has the required integrated debug/release evidence and no remaining
blocking finding. Approval permits X04-A; the coordinator owns status advancement.
Public init remains format1. Fresh format2 native acceptance requires embedded
request-v2; fresh request-v1/profile acceptance refuses, and profile accepted
history remains unsupported pending X06. X04/X06, public cutover, Linux, the X08
external runtime gate, RA01 and execution remain open. No conversion tooling,
migration path or compatibility CLI is introduced.

Reviewer artifacts and this report are finalized before ownership release. Final
audit and freeze manifests record their exact identities; no edits follow handoff.
