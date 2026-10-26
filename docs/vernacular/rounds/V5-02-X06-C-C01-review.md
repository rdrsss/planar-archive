# V5-02-X06-C-C01 independent review (corrective for the X06-C request-changes)

Disposition: **request-changes** (not records-only).

The corrective closes most of the X06-C review. The seam now lives in its own module,
`planar.engine.ledger.acceptance_seam`. `acceptance.cppm` imports it privately and exports only the two embedded overloads
and `acceptance_result`. Both adapters still complete through the one body, `complete_native_candidate`. Embedded
diagnostics are byte-identical. All 1003 committed names and all 25 X06-C names are kept. The built-in plus custom job mix
is covered by a test that its mutants kill (X06C-R2 closed). Records R3 and R4 are corrected, and the R3 analysis checks out
against `response.cpp`. Every gate figure I re-checked matches the raw logs, and my own full Debug run agrees.

Three things still stop approval:

- C01-R1: a test added in this round cannot fail for the mutant it was written for. The record calls the gap
  "unobservable", but the SQLite trace probe already in the same test file can observe it.
- C01-R2: the criterion 9 authority analysis says the seam is restricted by the link graph. That is false. Every CLI
  target has the seam module available, and the prober's own C22 build proves it.
- C01-R3: the no-Lua test comment from X06C-R5 is still uncorrected, and the record wrongly says a coder rewrote it.

C01-R1 and C01-R3 need test-file edits, so the fix is not records-only.

I reviewed the shared checkout on `dev/planar-v2-vernacular` at HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52`, with the
uncommitted candidate on top. I wrote only this file and new files under
`tests/native/v5-02-x06-c-c01-evidence/review/`. I applied no mutants and edited no source, test, record or existing
evidence. Serialized, with no other ninja or ctest process running, I ran:

- three `cmake --build build/rewrite-debug` calls (`planar_tests`, then the default target, then `planar_tests` again);
- `ctest -N`;
- one full Debug `ctest --no-tests=error --output-on-failure`;
- the lint.

I did not commit, push, merge, install or publish. I used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

- `git diff HEAD` sha256 is `3ad35762c7dfab9f09c4fa005a0f68b8c8ce8c9ee8a8a4e37ef5a8eba14ab1e6`, and `git diff HEAD -- src`
  sha256 is `d8cec5718ee4a058f9aeb873e8a431246d3182184b0b8c4a4276ea051c5ba102`. Both equal the Final record and the evidence
  README. The tracked hash also equals gate run 3 and `probes/PROBES.md` run 2.
- All eleven per-file hashes in the Final record's identity table equal the working tree. This covers the five untracked
  source files (`acceptance_seam.{cppm,cpp}` and `profile_acceptance.{cppm,cpp,t.cpp}`).
- `runtime.cpp` still hashes to `a82d1abf…7c3c`, and `git status` shows nothing under `src/engine/runtime` or `src/cmd`.
- The last source or test edit was at 02:03:28 (`acceptance.t.cpp`). Gate run 3 (02:04 to 02:10) and probe run 2 (03:10)
  both postdate it.

Raw: `review/raw/identity.txt`.

## Independent verification

| Check | Result (raw under `tests/native/v5-02-x06-c-c01-evidence/`) |
| --- | --- |
| Gate raw exits | All seven `raw/g3-*.exit` files read 0. `raw/g3-ctest.log`: "100% tests passed out of 1031", 1031 `Passed` lines, labels unit 1024 and acceptance 7. Its only "skip" text is in the names of tests #560 and #1003. Test #531 took 15.27 s, against 15.28 s in the B3-C-C02 baseline log, so it is not slowed. `raw/g3-lint.log`: "Formatting and Doxygen checked 141 active source files". |
| My full Debug ctest | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` exits 0: "100% tests passed out of 1031", 1031 `Passed` lines, unit 1024 and acceptance 7, none failed, skipped or not run. Test #531 took 15.70 s, against 15.27 s in the gate and 15.28 s in the baseline. The whole run took 379.57 s, against 366.34 s in the gate, so the machine was slightly slower overall. A 0.4 s spread on one run is within noise, and well inside the 30 s limit. The candidate does not touch the code this test exercises. (`review/raw/ctest-full.*`) |
| My lint | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` exits 0: "Formatting and Doxygen checked 141 active source files" (`review/raw/lint.*`). |
| No-work builds | `cmake --build build/rewrite-debug --target planar_tests`, then the default target, then `planar_tests` again: each exits 0 with `ninja: no work to do.` (`review/raw/build-*.log`, `.exit`). This also covers `planar_tests`, which gate run 3 did not check separately. |
| Inventory | `ctest -N` exits 0, "Total Tests: 1031". Compared as sets with `tests/native/v5-02-x06-b3-c-c02-evidence/inventory.txt` (1003 names): 0 removed and 28 new. All 25 names in `v5-02-x06-c-evidence/raw/new-names.txt` are present. The 3 remaining new names are the C01 tests (`review/raw/removed-vs-baseline.txt`, `new-vs-baseline.txt`, `x06c-missing.txt` (empty)). The moved test "X06 C seam interns an effective stdlib closure beyond the original tree limits" keeps its name. |
| Diagnostics | I compared string literals between HEAD's `acceptance.cpp` and the union of the three implementation files now. None is removed. The only added literal is "Prepared profile differs from request intent" (`profile_acceptance.cpp:24`, `invalid_argument`) (`review/raw/diagnostic-literals.txt`). `response.cpp:86-88` maps every `invalid_argument` to `schema` / "Invalid ledger arguments", so the R3 analysis holds. |
| Dependency audit (criterion 7) | My re-run is in `review/raw/dependency-audit.txt`. `src/engine/runtime` imports only `ledger.completion`, and `planar_runtime`'s DEPENDS lists no ledger acceptance, storage or seam target. The seam interface imports `storage_types`, `completion` and `acceptance.prepared`; its implementation adds `storage` and `tree_core`. Nothing leads back to `acceptance`, so there is no cycle. No `src/cmd` file names the seam or the profile module, and `invocation.cpp` is the only `src/cmd` importer of `ledger.acceptance`. Criterion 7 is met as worded. C01-R2 covers what the audit also shows about module availability. |
| Shared body and order | `acceptance_seam.cpp:16-130` is the X06-C order unchanged: in-use receipt, `begin_write`, identity, locked request lookup with replay or conflict, format, embedded profile refusal (`unsupported_state`, line 47), head CAS, `values()`, interning, `append_acceptance_rows`, `verify_before_commit`, `candidate.verify()`, phases and COMMIT. `acceptance.cpp` holds only the `prepared_acceptance_candidate` adapter and the two overloads that call the seam. Its `values()` and `verify()` read the same fields and keep the "Unavailable prepared acceptance" text. |
| Generated variants | Both `generate.py` edits clone `acceptance_seam.{cppm,cpp}` into the atomic and metadata variants with reversible substitutions. They also move the `--mutate-final-receipt` substitution to the seam, where `candidate.verify()` now lives. The variants build, and their tests pass in both full runs. |

## Closure of the X06-C findings

| Finding | Status | Basis |
| --- | --- | --- |
| X06C-R1 seam exported to every importer | Closed for plain importers, with a records error (C01-R2) | A plain `import planar.engine.ledger.acceptance` cannot name the seam: the seam is a non-exported import, and clang enforces module visibility. The probes confirm the guard reacts. Killed by the R1 test alone: I-R1a (the `export import` re-export), I-R1b-r and I-R1b2-r (using-declarations), C03-r (exported alias), C01 (re-export from the profile module), C02-r2 and C02-r3 (spaced re-export), and C22 (CLI imports the seam). I read the raw logs for I-R1a and C22; each ends `restored by hash: True` and names only the R1 test as failing. The I1i test moved to `profile_acceptance.t.cpp`, which imports the seam explicitly, and I1i still kills it. |
| X06C-R2 built-in and custom job mix | Closed | `profile_fixture(2, "quota", true)` keeps `split.lua` on `fanout_review`. The test requires the interpretation to contain both `fanout_review` and `custom` and to differ from the all-custom fixture. It checks the stored interpretation bytes and the stdlib and registry trees, then a byte-identical replay. Killed by I-R2a, I-R2a2 and C18 (among 19 failing tests), and by I-R2a-rt1, I-R2a-rt2 and I-R2b-r (this test alone; raw logs read). |
| X06C-R3 new message, "Decision B" | Closed (records) | The X06-C record now cites `profile-preparation-architecture.md` lines 353 to 355, which I checked. It withdraws "Decision B", names the new message and its `response.cpp` mapping, and corrects "no new diagnostic message". Both R3 tests now assert the text (C17 is killed). |
| X06C-R4 moved-from order | Closed (records) | Carried item 1 point 2 now says the refusal is in-transaction, after replay, format and CAS and before any row, and separates it from the receipt case. This matches `profile_acceptance.cpp:19-21` and seam lines 54-56. I-R4, X11 and C16 are killed. |
| X06C-R5 minor items | Partly closed | Row I3b now says `unsupported_state`, and I-R5 is killed by the embedded test. The placement justification and audit are recorded. The no-Lua comment is not fixed: see C01-R3. |

## Findings

### C01-R1 (required, test): the new registry-limit test cannot fail for its mutant, and the record's reason is wrong

"X06 C C01 seam interns the registry tree with the provenance limits" (`profile_acceptance.t.cpp:859`) was added in this
round to kill C14 (the registry tree interned with the source-profile limits). Probe run 2 shows C14 surviving, and the
record keeps the test with this justification: "the refusal site is not observable through this API". That justification
does not hold.

- In the unmutated seam, the 17 MiB registry file is refused by `intern_tree(..., provenance)` at
  `acceptance_seam.cpp:63-65`. That is before `append_acceptance_rows` (line 97) and before any staging statement.
- The prober's own diagnostic (`probes/raw/c14-message-probe.txt`) says the C14 mutant is refused later, by pre-commit
  verification of the stored provenance tree. So under the mutant, staging ran.
- The same test file already has the instrument that tells the two apart. `trace_probe` sets `pending` when the writer runs
  `UPDATE current_state SET` (`profile_acceptance.t.cpp:187-189`), and the race tests already rely on it.

The C14 test installs no probe (`review/raw/c14-observability.txt`). As written it passes on both trees, so it breaks
standing lesson 1: a new test that cannot fail for its own invariant is hollow.

Required:

- Make the test observe the refusal site without adding product observability. For example, install `installed_probe` and
  require that no staging statement ran (`CHECK_FALSE(installed.probe.pending)`), or record the SQL seen through `each`.
- Re-run C14 in a scratch copy and record the kill, naming this test.
- If the prober still cannot kill C14, record why that specific observable fails, rather than calling the site
  unobservable.

### C01-R2 (required, records): the authority analysis claims a link-graph restriction that does not exist

The criterion 9 analysis in the Final record says: "Reachability is therefore restricted by the link graph (only
`planar_ledger_acceptance`, `planar_ledger_profile_acceptance`, its tests and the generated variants link it)". That is wrong.

- `planar_rewrite_module` links DEPENDS as PUBLIC (`cmake/rewrite.cmake:23`), so `planar_ledger_acceptance_seam` is in the
  link closure of every consumer of `planar_ledger_acceptance`. In the build tree, the seam target directory appears in
  `linked-target-dirs` of 67 `src/cmd/planar` targets, including `planar`, `planar_command_invocation`, every CLI variant and
  the load CLI probes.
- The modmap for `invocation.cpp` already passes
  `-fmodule-file=planar.engine.ledger.acceptance_seam=…pcm` (line 26), because clang needs the transitive BMI.
- Probe C22 built `planar_command_invocation` with `import planar.engine.ledger.acceptance_seam;` and
  `using seam_probe = ::planar::engine::ledger::native_candidate;`. The build succeeded; only the R1 test's source scan
  failed (`probes/raw/C22.log`).

Raw: `review/raw/dependency-audit.txt`.

The correct statement:

- A plain importer of `ledger.acceptance` cannot name the seam. That holds at language level, and the R1 test checks the
  private import textually.
- Any target that links `ledger.acceptance` (the CLI included) can still import the seam by naming it, and so can supply
  trees, values, a `verify()` callback and `selects_profile()`.
- The only guard against that is the R1 test's scan of `src/**/*.cpp(m)`, which C22 proves works. The scan does not cover
  sources outside `src/` or generated sources.
- The architecture's "callers cannot supply its tag, fields or a callback" is therefore met by convention plus a
  source-scan test, not by construction.

The record must also state plainly that the passkey option was not taken. It should say whether structural enforcement is
infeasible under the runtime-layering rule; I think it is hard, because a partition of `ledger.acceptance` would pull
`runtime` into the ledger variants. It should also say whether the operator accepts the convention-plus-scan form.

This is a records-only fix, but it is part of criterion 5 and the core of X06C-R1, so it is blocking.

### C01-R3 (required, test comment and records): the X06C-R5 comment is not corrected, and the record misstates it

The comment at the top of "X06 C no Lua runs while the profile write transaction is open"
(`profile_acceptance.t.cpp:563-564`) still says "no mutant is constructible without a new public runtime entry point. This
is an observational guard, not a mutant killer." That is the wording the X06-C review quoted. But X27 (a scratch edit to the
runtime's `verify_native_receipts`) is killed by this test alone in probe run 2.

The X06-C review asked for the correction "when the source is touched for R1", and `profile_acceptance.t.cpp` was edited
twice in this round. Section "Records R3 to R5" also says the comment "was rewritten by a coder to call the test an
observational guard". I found no sign of a rewrite: the text matches what the X06-C review quoted.

Required:

- Correct the comment so it agrees with X27. For example: it detects Lua-side activity under the lock; it is killed by
  X27, a scratch edit to `verify_native_receipts`; no product mutant within the ledger can reach the VM.
- Fix the record's description.

This is a comment-only edit with no behaviour change. The candidate hashes change, so the identity table must be refreshed,
and the commit-step gate covers the rebuild.

### C01-R4 (minor, records): criterion 1's compile-time form and criterion 4's red-first are met only in substitute form; say so as an accepted amendment

- Criterion 1 asks for "a compile-time surface test (a negative compile or a concept/requires check)". The final R1 test
  instead checks the text of `acceptance.cppm` and scans the source tree. The record says "partly met". I accept the
  substitute on its merits:
  - Visibility for a plain importer is a language guarantee once the import is private. The test checks that the import is
    private, and the probes show every re-export or naming form is caught.
  - The scan catches C22, which a compile-time check of a plain importer could never catch.
  - A negative compile is possible but would add a build-inside-ctest harness for no added detection.
  The record should present this as an amendment to criterion 1 that the coordinator accepted, not leave it as "partly met"
  next to a "met" overall reading. It should also list the known blind spots: a re-export through a module that avoids all
  four names, and sources outside `src/`.
- Criterion 4 asks for red-first "with exact command and a hash that includes untracked files" for the R1 and R2 tests.
  - `red/RED.md` again hashes tracked files only (`9681ec4d…`). The X06-C review had flagged exactly this for future
    rounds.
  - The only behavioural red is for the stub-era body of the R1 test (`red/raw/I-R1abc.log`, assertion failures at lines
    754 to 756). That body was replaced twice.
  - R2 could not be red, because the adapter already stored the classification.
  I accept the probe kills as the behavioural evidence: I-R1a puts the test against a seam re-export and gets an assertion
  failure, and the probe run carries a combined tracked plus untracked hash, `5b5d9d20…`. The record should say criterion 4
  is met by probe substitution. It should also say the untracked-hash requirement is unmet for red-first, and why.
- The two `tests/native/*-support/generate.py` edits fall outside the dispatch's owned-file list. They were necessary, and
  they are listed under "Files edited". But "Amendment to earlier constraints: none" should record them as an ownership
  extension.

## What is sound

- Single body (criterion 2): `complete_native_candidate` is the only transaction body. Shared-body mutants fail both
  families, for example I1c (26 of 52), X13 (4) and X31 (13). The embedded diagnostics and order are unchanged.
- Opaque profile authority: `profile_acceptance.cppm` still exports only the `prepared_profile` overload and does not
  re-export the seam (C01 is killed). The I9 negatives are now runtime `CHECK_FALSE`s, so X28 is a test kill and no longer
  only a build failure.
- Survivors: X12, X18, X20 and C09 are equivalent, as accepted in X06-C. For C05 and C19 I accept the argument that a real
  closure never exceeds the provenance limits, so the adapters' `stdlib_effective` flag cannot be observed through real
  inputs; I-I1i kills the seam's handling of the flag. That leaves C14 (C01-R1).
- BUILD-FAILED rows are not counted as kills, and each has a corrected `-r` row or an explanation. C02, C04, C02-r and
  C04-r fail at configure time because of the generator in `src/engine/CMakeLists.txt`, so they are a build-graph guard
  with no killing test. That is recorded honestly.

## Limits carried and observed

- Mutants ran against 52 ledger tests (C22 against 2); only the unmutated full suite ran in the gate and in my run.
- I confirmed C14's observability by reading the code and the prober's diagnostic, not by applying a mutant (reviewer
  remit). C01-R1 asks the prober to show the kill.
- The redact check (`raw/g3-redact.*`, 15 files, 02:11) predates the record edits and this review file. It must be re-run at
  commit on the final file list, as V7 says.
- The roadmap opening still describes the candidate as unchanged since review with C01 next. `docs/architecture.md` does
  not mention `ledger.acceptance_seam` or `ledger.profile_acceptance`. Both are for the commit step.
- Carried and unchanged: P01 masked, N1 phase coverage unmeasured, N2 message open, macOS Debug only, Linux not run, and
  fresh profile acceptance still refused at the public surface.

## Review evidence

Under `tests/native/v5-02-x06-c-c01-evidence/review/raw/`:

- `identity.txt`: HEAD, diff hashes, per-file hashes, status.
- `build-planar_tests.*`, `build-all.*`, `build-planar_tests-2.*`: the no-work builds.
- `ctest-N.*`, `inventory-now.txt`, `baseline-sorted.txt`, `removed-vs-baseline.txt`, `new-vs-baseline.txt`,
  `x06c-names.txt`, `x06c-missing.txt`: the inventory check.
- `ctest-full.*`: my full Debug run.
- `lint.*`: my lint run.
- `dependency-audit.txt`: imports, DEPENDS, the CLI targets with the seam in their link closure, and the `invocation.cpp`
  modmap entry.
- `diagnostic-literals.txt`: the literal comparison against HEAD.
- `c14-observability.txt`: seam order, the prober's C14 diagnostic, and the existing trace probe.
