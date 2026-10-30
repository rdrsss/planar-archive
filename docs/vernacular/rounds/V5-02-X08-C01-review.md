# V5-02-X08-C01 independent review (whole X08 candidate with C01, at the R6 gate run 6 repair)

Disposition: **request-changes** (not records-only).

The code side of the candidate is sound. The R6 repair is test-only. Every test name is preserved and all four configurations
pass 1260 of 1260 with zero skips. The new tests kill their mutants, and the platform-audit failure from the previous gate is
closed. Three acceptance items are still not met, though, and two of them need changes to evidence data or test-support code:

- The closeout validator still checks the stale 1258-name tree.
- The G06 platform rows quote log lines that appear in no final log, and they leave out the newly audited branch.
- The I16b "equivalent survivor" claim is contradicted by an existing native test that the prober never ran.

The round record already marks the validator item "not met". The fixes are small, but they touch `validate.py`, the evidence
tree and `docs/testing.md`, so the records-only path does not apply.

## Candidate identity

I ran `identity.py src tests/native` from the repository root (`tests/native/v5-02-x08-c01-evidence/review-r6/identity.txt`).
Its output, verbatim:

| identity | value |
|---|---|
| HEAD | `48b8e526764a76de1392d397bb21659bf63e8b8d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `4eca3b80e9b318a64b32064f2758a240c603f8416fe955b15be59aad83467724` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `8177d388939ecd09ee6498002906184dd22f5c5519d13d525ec3c5d0702dedc8` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/bootstrap.t.cpp` | modified | `558c811d5fe670e874a5b446f1a10625f2f7ec997f95d75ed72f6b5720291b6d` |
| `src/engine/capture/host_path.cpp` | modified | `9207c2b79ecc9fce3064c865bb8e253931795a503244304efcc172a5eb8ca7d3` |
| `src/engine/capture/host_path.t.cpp` | modified | `ddafdd85acc47e0b0c400e9f19158709029fc4a988e06edf235c89c0137308e8` |
| `src/engine/runtime/commands.t.cpp` | modified | `f36e6a3cf2f52ab54ac24a0a7284109a832668d063516e959bc3592159ffa072` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `22afc6e0c8f15a9dd705538c898b8bd263604f3c5da6fed673d04ba3f556b605` |
| `tests/native/v5-02-x08-support/generate.py` | untracked | `0211ae98cd8064385b628d8f89e744bbfb314de95f8489032efbc8bc364b7420` |
| `tests/native/v5-02-x08-support/validate.py` | untracked | `02f55552d639da2f2ed08dc0ecb27296ae0d59eb9461d53f6066e7ee3e427b94` |
| `tests/native/v5-02-x08-support/x08.cmake` | untracked | `5c6d671fac9f016be46ccd136c529e0d3be55b8c866aa234247deb868948c7ea` |

This output is byte-identical to the round record's "Final candidate" table, to the R6 gate identity (`raw/run2/identity.txt`, and
`raw/identity.txt` of run 1) and to the prober's identity (`probes/raw/00-identity-full.txt` and `probes/superseded-1/raw/00-identity-full.txt`).

The candidate also has files outside that pathspec: the five `tests/acceptance/x08_*.py` and the three `scripts/linux-gate.*`. Their
sha256 values match the record. The whole-tree `git diff HEAD` hash is also unchanged (`0b0047f5...7ad33`). Every candidate file
predates the R6 gate run 2 (21:01). The Linux context manifest (`raw/run2/linux/container/context-manifest.txt`) contains the current
hashes of `external_registration.t.cpp`, `bootstrap.t.cpp`, `commands.t.cpp`, `host_path.cpp`, `x08_cli.py`, `x08_adversarial.py` and
`validate.py`, so the container tested these exact bytes.

## What I checked and found sound

- **No product change in R6.** `git diff HEAD --stat -- src ':!*.t.cpp'` lists only `src/engine/capture/host_path.cpp`, which is the
  C01 pre-check (19 added lines, last modified at 05:14, before the R6 dispatch). The pre-check throws only the existing
  "Cannot resolve explicit root directory", which HEAD's `host_path.cpp` already contains. The R6 diff removes no line from any
  product file, so every diagnostic is byte-identical.
- **Platform audit (decision (a)).** `audited_platform_counts()` now includes `engine/runtime/external_registration.t.cpp` with
  count 1 and the one-line reason the operator asked for. A grep of `src/` (vendor and `bootstrap.t.cpp` excluded) for the seven
  tokens finds exactly the audited set: native.cpp 1, admission.cpp 2, runtime.t.cpp 1, source_capture.t.cpp 1, source_export.cpp 4
  and external_registration.t.cpp 1 (`:4557`). Four prober mutants kill `X08 platform branches are exactly the audited set`,
  including the missing-row mutant (the previous gate failure).
- **BUD-CONV.** In the new test `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`, deleting
  `charge(s, 6 * 320 + sizeof(value))` moves the boundary from 228586 to 228621. The window is 8, so the test fails on an assertion
  (`probes/superseded-1/raw/R6-BUD-CONV.log`, `CHECK( off <= c02_off_boundary + c02_off_window )`, 228621 <= 228594). The test also
  fails under the -half and -up mutants and on Linux (`linux/linux-R6-BUD-CONV.log`).
- **I08 and I08b.** `X08 C01 native decimal id facts are bounded at INT64_MAX exactly` (`commands.t.cpp:309`) is killed by name
  under the max+1, UINT64_MAX, max-1 and 20-digit mutants. The test drives the retained history and export handlers through fake
  services, adds no public field, and asserts both sides of the boundary.
- **I02.** I read `src/cmd/planar/envelope.cpp:49-50`. `classify_prefix` refuses `selector_option->count() != 1` independently of the
  multi-option policy, so Throw versus TakeLast gives the same usage error. Recording it as an equivalent survivor is honest. The new
  block in `x08_cli.py` still pins the observable policy (three pairs times three tails, exit 2 with the usage bytes).
- **HP-I3 and HP-I24 on Linux.** Both mutants were applied in the container and killed by
  `host path maps a regular file followed by parent traversal to the root directory error`. The logs show the `host_path.t.cpp:30`
  assertion "Cannot resolve explicit input path" == "Cannot resolve explicit root directory", followed by a byte-copy restore checked
  against `9207c2b7...`. The container was arm64 with clang 23.1.5 (`linux-env.txt`).
- **The remaining survivors HP-I2a, X-I21, BUD-I15b and X-I04** have recorded reasons that I find honest. For HP-I2a, `status` with
  `ec` set yields `file_type::none`. X-I21 is redundant with the per-test parse. For BUD-I15b, a looser pin cannot be detected while
  the measured value is 4742 (4743 is killed). For X-I04, the mutated literal is not exercised by the goldens; this is a coverage
  limit, and the record states it as such.
- **Gates (R6 run 2).** I read the raw logs rather than the summaries:
  - `ctest-debug`, `ctest-release`, `linux/container/debug-ctest` and `linux/container/release-ctest` each print
    `100% tests passed out of 1260` with exit 0.
  - Each shows 1260 Passed lines and no `***Failed`, Not Run or Disabled. The only "skip" hits are inside test names.
  - `x07.two_process_cas_one_winner` passed in all four configurations (1.21 s, 0.51 s, 0.97 s, 0.43 s).
  - Lint exit 0 ("Formatting and Doxygen checked 147 active source files").
  - Four macOS no-op builds and both container no-work builds print `ninja: no work to do.` with exit 0.
  - Host manifest equal before and after the build and equal to the container context. `builder prune` exit 0. 107 GiB free before
    and after.
- **Names.** All four sorted inventories are byte-equal: 1260 names. Compared with the 1258-name C01 inventory, nothing is removed
  and exactly the two new R6 names are added. All 1227 prior names and all 28 X08 and 3 C01 names in `validate.py` are present, so
  every one of the 1255 earlier names is preserved.
- **Earlier red-first records** under `c01-evidence/red/` are behavioural (exit 42 on assertions), with the pre-fix snapshot hashes
  in `scratch-snapshots.txt`. R6 is test-only, so its "red" is the prober's mutants, and the record says so.

## Findings

### V5-02-X08-C01-R1 (blocking): the validator does not cover the final candidate

The acceptance criteria in the brief and in the gate run 6 dispatch (step 4) are:

- refresh `tests/native/v5-02-x08-evidence/raw/` from the final gate logs;
- run `validate.py --evidence tests/native/v5-02-x08-evidence --repo .`, exit 0, with the log under the C01 evidence `raw/`.

I re-ran the validator (`review-r6/validate-rerun.{log,exit}`). It exits 0, but G01, G02, G04 and G05 report 1258 tests. Several
things are wrong:

- `raw/macos-debug.ctest.log` is the run-4 log (`100% tests passed out of 1258`).
- `validate.py` derives `EXPECTED_TOTAL = 1227 + 28 + 3 = 1258` and does not know the two R6 names.
- `linux/` is the run-4 container output.
- The only R6 validator log is in `c01-r6-evidence/raw/run2/validate.log`, not in the C01 evidence `raw/`.

The round record's own acceptance table says "not met". The README of the validator tree and the R6 README say the same.

Fix:

1. Assemble `raw/` and `linux/` of the validator tree from `c01-r6-evidence/raw/run2/` in the layout `validate.py` expects. Move the
   run-4 tree to `raw/superseded-3/`; do not delete it.
2. Add the two R6 names to `validate.py` (for example an `R6_ADDED_NAMES` tuple included in `EXPECTED_TOTAL` and in the G01
   presence check), and update the self-test's literal to 1260.
3. Re-run `x08.validator_selftest` and the validator, and save both under `c01-evidence/raw/`.

`validate.py` is inside the identity pathspec, so the coordinator must also refresh the identity and decide how much of the gate to
re-run.

### V5-02-X08-C01-R2 (blocking): the G06 records are stale and do not record the audited branch

There are two problems.

(a) The eight `pass_line` values in `requirements.json` are lines from the second, failed X08 gate (`149/1255 Test #149: ...` and so
on). None of them occurs verbatim in the validator tree's Linux logs or in the final R6 Linux logs
(`review-r6/g06-passline-check.txt`: 8 of 8 absent in all four logs). C01 invariant I29 requires every G06 `pass_line` to quote a line
present in the final Linux ctest log, and says the validator's G06 check enforces this. It does not: `gate_g06` only checks that the
field is non-empty, which is why the stale lines pass. That makes the I29 row hollow.

(b) Operator decision (a) says to record the `external_registration.t.cpp` branch "in the X08 platform-branch audit records (G06 rows
and this record) so validate.py still passes". `requirements.json` mentions `external_registration` zero times. The platform table
in `docs/testing.md` (around line 1782) still says "the seven source branches below" and does not list it. The record explains that
the validator forbids ids beyond G06h. But `validate.py` is an owned file in this round and has to change anyway for R1, so that is
not a reason to depart from an operator decision without asking.

Fix:

1. Add a G06i row: the branch at `external_registration.t.cpp:4557`, the Linux test `X06 D C02 ...` / `X07 N30 ...` / `X08 C01 role
   preparation outputs-off ...`, and the operator reason.
2. Extend `PLATFORM_IDS` and the self-test, or ask the operator for an alternative.
3. Refresh all G06 `pass_line` values from the final Linux logs.
4. Make `gate_g06` check that each `pass_line` occurs in `linux/container/debug-ctest.log`, so I29 has a real check. A
   self-test mutant for a stale line would make that check able to fail.

### V5-02-X08-C01-R3 (blocking): the I16b survivor is probably not equivalent; the probe filter was too narrow

The record and PROBES.md call `R6-I16b` (`profile.cpp:91` per-module bound raised from 1048576 to 16777216) an equivalent survivor.
The stated reason is that a second bound in `runtime.cpp` refuses the same module, and the record adds that the native test
`profile capture module ceiling checked before observed module read` "also covers the capture bound". These two claims contradict
each other.

That native test (`src/engine/capture/profile.t.cpp:368-377`) calls `profile_capture::open` directly. It requires a 1048576-byte
module to be accepted and a 1048577-byte module to be refused with `resource_limit` / `profile.modules[1]` and zero read attempts.
`runtime.cpp` is not on that path. In `native.cpp`, `read_bounded` with `text=false` applies `maximum = 16777216` (`:312`). So once
the explicit `byte_ceiling` from `profile.cpp:91` is raised, nothing on that path refuses 1048577 bytes, and the test should fail.

The prober ran the mutant only against `ctest -R ^x08\.adv_limits_cli$` (`probes/raw/R6-I16b.log`). This is the filter-miss pattern:
a survivor that has not been run against the test most likely to kill it. I did not apply the mutant myself (only the prober
mutates).

Fix: the prober re-runs `R6-I16b` with the profile capture tests included, at minimum
`-R 'profile capture module ceiling|x08\.adv_limits_cli'` (or the whole `planar_capture` label). Then update PROBES.md, the record's
survivor and acceptance tables, and the R6 REPORT. If the mutant is killed, the operator's I16b item is met by an existing native
test plus `x08.adv_limits_cli`.

### V5-02-X08-C01-R4 (required, reference doc): `docs/testing.md` describes the pre-R6 candidate

`docs/testing.md:1826-1832` still says `c02_outputs_gap` = 4742 "measured ... on macOS" as the single pin. It gives the rule "keep
the smaller value", which the per-platform pins (4742 / 4737, boundary 228586 / 228612) now contradict, and it says "The expected
inventory is 1258 names". The platform-branch section (R2 (b)) also omits the audited test branch.

Update the C01 paragraph with the per-platform pins and their reason, the outputs-off boundary pin with its re-measure rule, the
audited test-only branch, and the 1260-name inventory (1227 + 28 + 3 + 2).

### V5-02-X08-C01-R5 (required, records): the roadmap opening shows gate run 4 figures

The X08 candidate bullet in `docs/vernacular/roadmap.md` still reads "Gate run 4: macOS ... 1258/1258 ... 31 new; the Linux container
gate ... 1258/1258". The final candidate is 1260 in all four configurations, with 33 new names. The record's limits section admits
this but calls the roadmap outside the coordinator's owned records. The gate run 6 dispatch lists "the roadmap opening" among its
owned files, so the coordinator should relabel it. Keep the next-step text (X07-C02, then RA01).

### V5-02-X08-C01-R6 (minor, records): local home path in committed evidence reports and a wrong file count

`redact-local-paths.py check` reports a local path in `tests/native/v5-02-x08-c01-r6-evidence/REPORT.md:27` and in
`tests/native/v5-02-x08-c01-evidence/REPORT.md:32`. Both quote `--home` followed by the local home path. Exit 1 is recorded in
`review-r6/redact-reports.{log,exit}`. Evidence `REPORT.md` files are the parts that get committed with `git add -f`, so replace the
path with `<home>`.

The R6 REPORT also says "26 files" for the redaction run, but `raw/run2/redact-files.txt` lists 23.

### V5-02-X08-C01-R7 (minor, records): gate run numbering and the CAS history are misstated

The gate run 6 dispatch says "Run 5 failed because the survivor repair added `#if defined(__APPLE__)` pins". In the evidence,
`c01-evidence/REPORT-run5.md` is "GATE PASSED", and the audit failure is in `c01-evidence/REPORT.md`, titled "gate run 6 report ...
GATE FAILED". Align the record's labels with the evidence, or add a mapping note.

Separately, the brief's statement that the "x07 CAS test did not fail in any gate run" holds for every run since the 2026-10-29
no-exemption rule, including both R6 runs. It did fail once, in C01 gate run 1 (`c01-evidence/raw/05-ctest-release.log`). That run
failed for other reasons anyway, and the failure is recorded as the open X07 product finding. The record should state this
explicitly rather than leave it implicit.

### V5-02-X08-C01-R8 (advisory): no R6 invariant file

`tests/native/v5-02-x08-c01-r6-evidence/plan/INVARIANTS.md` does not exist. The R6 dispatch said "no re-map and no re-plan", and the
R6 rows are carried by the record's survivor table and PROBES.md Tables R and A. I checked each operator item against them:

| item | carried by | result |
|---|---|---|
| audit row | AUD ×4 | killed |
| BUD-CONV | ×3 plus Linux | killed |
| I08 / I08b | DEC ×4 | killed |
| I16b | see R3 | see R3 |
| I02 | equivalence | equivalent, checked |
| HP-I3 / HP-I24 | Linux | killed |

C01 invariant rows I17 and I26 (1258) are superseded by the 1260 inventory, and I29 is hollow (R2). A short R6 addendum to the C01
INVARIANTS table, moved to superseded per the protocol, would keep the table authoritative.

Advisory on code, not blocking: the outputs-off boundary pin (228586 / 228612, window 8) is an absolute measurement. Any future change
to native accounting on the role path will move it. The comment explains the slope; it should also state the re-measure rule (both
configurations, per platform), as the gap constant does.

## Limits of this review

- I did not apply any mutant; mutation results are the prober's logs, which I spot-checked: BUD-CONV, the four DEC mutants,
  AUD-row-missing, I16b-lower, I16b-both-runtime, I02, and Linux HP-I3, HP-I24 and BUD-CONV.
- I did not re-run ctest or the Linux container. The gate conclusions rest on the raw logs above, whose identity matches the
  candidate.
- RelWithDebInfo mutants were not run, and x86_64 Linux is not covered. Both are carried limits.

## Evidence written by this review

`tests/native/v5-02-x08-c01-evidence/review-r6/`:

- `identity.txt`
- `validate-rerun.log` and `.exit` (exit 0, 1258 counts)
- `g06-passline-check.txt`
- `redact-reports.log` and `.exit` (exit 1)
