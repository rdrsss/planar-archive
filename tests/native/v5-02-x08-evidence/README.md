# V5-02-X08 evidence (with C01)

This directory is the closeout evidence for the mandatory full external-profile acceptance gate X08 and its corrective round
C01. It is gitignored raw output plus tracked-by-hand records; the validator reads only what is listed here.

## Layout

- `requirements.json`: E01 to E14, the twelve adversarial matrix rows with sub-clauses, the platform rows G06a to G06i and the
  `images` object. Test names are `ctest -N` names from the macOS Debug inventory; `logs` name paths of the final assembled tree.
- `raw/`: the final macOS gate output (ctest logs, inventories, `prior-names.txt`, lint, redaction, no-op builds, build logs and
  their `.exit` files). `raw/superseded-1/` holds the first failed gate; `raw/superseded-2/` holds the second failed gate (its
  whole former `raw/` tree including its Linux run, its run scripts and `REPORT-second-gate.md`, a byte-identical copy of the
  second gate's report). Superseded trees are retained, never rewritten.
- `linux/`: the final Linux gate output (`linux/container/{debug,release}-{ctest,inventory-sorted}.*`, `*-nowork.log`,
  `toolchain.txt`, `context-manifest.txt`, `host-manifest*.txt`, `docker-build.exit`, `identity-{before,after}.txt`).
- `plan/`: the X08 plan tables. C01 raw red-first records live in `tests/native/v5-02-x08-c01-evidence/{red,raw,records}/`.
- `REPORT.md`: the round report.

State of this tree (refreshed in C01-R7, 2026-10-29): `raw/` and `linux/` are the final gate logs of C01-R6 run 2 (1260 names in Debug and RelWithDebInfo, macOS and
Linux), copied from `tests/native/v5-02-x08-c01-r6-evidence/raw/run2/` (macOS `ctest-*.log` become `raw/macos-*.ctest.log`, `inventory-*-N.txt` become the `ctest -N`
inventories, the container tree is `linux/container/`; `linux/identity-before.txt` and `identity-after.txt` are both `run2/identity.txt`). The run-4 tree (1258 names) is
retained under `raw/superseded-3/{raw,linux}/`. The validator (1260 names, G06a to G06i with pass_line occurrence check) exits 0 over this tree; its logs are
`tests/native/v5-02-x08-c01-r7-evidence/raw/`. `REPORT.md` here is the interim PK-REC text; the final reports are `tests/native/v5-02-x08-c01-r6-evidence/REPORT.md` and
the R7 section of `docs/vernacular/rounds/V5-02-X08.md`.

## How to run the validator

From the repository root, unpiped:

    python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .
    python3 tests/native/v5-02-x08-support/validate.py --self-test

It prints `Gxx PASS` or `Gxx FAIL <reason>` per gate and exits 1 when any gate fails. It never runs a build, ctest or a product
binary. The self-test is registered as ctest `x08.validator_selftest`.

## E01 to E14 mapping

The authoritative per-requirement name lists are in `requirements.json`; the plan table is `plan/INVARIANTS.md` (Requirement
mapping). Public-CLI tests (they spawn the built `planar` binary; E14 is the gate itself):

| Requirement | Public-CLI test names (X08 or X07) |
|---|---|
| E01 | `x08.e01_no_discovery`, `x08.e01_registration_policy` |
| E02 | `x08.e02_envelope_rows_binary`, `x08.e08_projection_vector_binary`, `x08.e09_history_id_binary`, `x08.adv_opaque_ordering_binary` |
| E03 | `x08.e03_closure_separation_binary`, `x08.adv_hardlink_module_refuses_before_chunk`, `x08.adv_missing_declaration_initial_admission`, `x08.adv_opaque_ordering_binary`, `x08.adv_registry_colocation_binary` |
| E04 | `x08.adv_limits_cli` |
| E05 | `x07.substitute_history`, `x07.substitute_export`, `x07.substitute_init` |
| E06 | `x08.adv_job_mix_and_phase_failures`, `x07.workflow_shape_load`, `x07.workflow_render_external` |
| E07 | `x08.e07_old_goldens_unselected_profile`, `x08.e07_old_w_vectors_binary` |
| E08 | `x08.e08_projection_vector_binary` |
| E09 | `x08.e09_history_id_binary` |
| E10 | `x08.e10_removed_profile_replay_binary`, `x07.replay_after_inputs_deleted`, `x07.replay_conflict_matrix`, `x07.relookup_equal_replay_before_stale_cas`, `x07.relookup_unequal_conflicts`, `x07.two_process_cas_one_winner` |
| E11 | native only; the E12 cases read the format2 ledger through the binary |
| E12 | `x08.e12_variant_reads_identically`, `x08.e12_variant_exports_identically`, `x08.e12_variant_poison_is_live`, `x08.e12_variant_poison_is_live_load`, `x08.e12_variant_poison_is_live_profile_render`, `x08.e12_variant_audit`, `x08.e12_sources_and_profile_removed`, `x07.substitute_history`, `x07.substitute_export` |
| E13 | `x07.late_receipt_survives_to_seal`, `x07.receipt_races_refuse_before_authorization`, `x07.final_race_rolls_back_before_commit`, `x07.post_commit_allocation_failure_keeps_receipt`, `x07.post_commit_encoding_failure_keeps_receipt`, `x07.post_commit_stdout_failure_keeps_receipt`, `x07.two_process_cas_one_winner`, `x08.e10_removed_profile_replay_binary` |
| E14 | validator gates G01 to G09 (`x08.validator_selftest` guards the validator itself) |

The matrix rows 1 to 12 and their sub-clauses, with the native and public-binary names for each, are in `requirements.json`.

## How C01 differs from the earlier X08 gates

C01 follows the second failed gate (macOS 1255 of 1255, Linux four failures). It changes one product file and one test file:

- `src/engine/capture/host_path.cpp`: a platform-neutral pre-check in `host_directory` that maps a regular file (or a link to one)
  followed by `..` to io "Cannot resolve explicit root directory" before `canonical` runs. No diagnostic text changed.
  Two macOS mappings change (see the round record C01 result): `alias/file/..` and `physical/file/../file` as a directory argument.
- `src/engine/runtime/external_registration.t.cpp`: the C02 budget-gap fixture is pinned to a fixed canonical root length.
- Three test names are added at the C01 stage (1227 prior + 28 X08 + 3 C01 = 1258); the final R6 candidate adds two more (1260); no existing test name is removed or renamed.
- The validator derives its expected total from the name lists (1260 with the two R6 names, `R6_ADDED_NAMES`).
- The raw tree is re-assembled from fresh C01 gate runs; the second gate's tree moved to `raw/superseded-2/`.

## Limits recorded, not repaired

- CL-1 to CL-6, as classified in `docs/vernacular/x07-addendum.md` (CL-1 and CL-2 reachable, CL-4 probably reachable and not
  exercised, CL-3, CL-5 and CL-6 not reachable by untrusted callers); X07 and C01 repair none.
- X07-C01 limit: a writer killed during a busy wait is rolled back silently (non-POSIX-lock filesystems, RESERVED probe side
  effects and Debug-only probe limits are recorded alongside it).
- There is no RSS guarantee; resource bounds are the whole-invocation native allocation and instruction budgets.
- There is no hostile-writer guarantee for concurrently modified inputs (ledger-contract.md writer limits); the X-CAP final
  paragraph applies.
- Red snapshots and evidence are tracked-only: untracked bytes are covered by `identity.py`, but raw logs are gitignored.
- x86_64 Linux is not covered; the Linux gate runs arm64 natively under Docker on the operator's Mac.
- Public-clone limitation of `expected-schema.sql`: a fresh public clone cannot reproduce that fixture without the retained
  copy (recorded as a limit; no change made here).
- Concern for the operator: load of an oversize obligation set reports `capture_changed` while render reports
  `resource_limit` / `native_bytes` for the same input. The contract message table is closed, so no change was made.

## Results

Final candidate (V5-02-X08-C01-R7; product bytes identical to R6): macOS Debug and RelWithDebInfo and Linux Debug and RelWithDebInfo each 1260 of 1260, zero skips; see
`tests/native/v5-02-x08-c01-r6-evidence/REPORT.md` (product gate), `tests/native/v5-02-x08-c01-r7-evidence/{REPORT.md,probes/PROBES.md}` (light gate and validator probes), and the
"Final candidate: V5-02-X08-C01-R7" section of `docs/vernacular/rounds/V5-02-X08.md`. This tree holds the final 1260-name logs (run-4 logs in `raw/superseded-3/`).
