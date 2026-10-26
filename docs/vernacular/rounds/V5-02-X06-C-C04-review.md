# V5-02-X06-C-C04 independent review (corrective for the C03 request-changes)

Disposition: **request-changes, records only**. This is the fifth review in the X06-C lineage.

The product side of C04 is done and holds up under independent checks:

- C03-R1 is closed by operator decision A. The seam entities `candidate_values`, `native_candidate` and
  `complete_native_candidate` now sit in the nested namespace `planar::engine::ledger::seam_detail`. That namespace is not
  exported and holds no exported or reachable entity. `acceptance_result` stays outside it, so argument-dependent lookup (ADL)
  from `acceptance_result` cannot find the seam.
- I compiled the C03 forge r7 again with the exact flags and module map of `planar_command_invocation`. It now fails with
  "use of undeclared identifier 'complete_native_candidate'".
- Four new trusted-code ADL and using-directive probes of my own also fail. Controls c1 and c2 compile.
- The direct-seam tests are split into their own implementation unit, so C03-R6 is closed by the split rather than by a
  justification.
- All 1038 C03 names are kept, plus three new names. Diagnostics are unchanged. #531 is not slower. Every identity hash in the
  C04 record recomputes with its stated pathspec, including the two rows that cover untracked bytes.

What blocks approval is a pair of record statements that are wrong about the final candidate. Criterion 9 requires "no section
describes an earlier candidate":

- C04-R1: the C04 record says the post-red repair of `acceptance.t.cpp` is unknown. The gate's own raw log shows what it was,
  and the record's amendment still describes the A2 pin as it was before the repair. The repair also narrowed the call-count
  scan, and the record does not say so.
- C04-R2: a C03 record line says its consumer list was "completed by the C04 sync", but the list still leaves out the targets
  C03-R4 named.

Both are fixed by editing records alone. The other findings (C04-R3 to C04-R5) do not block.

I applied decision B as recorded verbatim in the C04 dispatch. One route compiles: a translation unit (TU) that declares itself
`module planar.engine.ledger.acceptance;` can name the seam, even without `import :seam;` (prober l1b, my v5). That needs a
deliberate module-unit declaration, which is a recorded limit and not a finding. The in-tree scan catches it under `src/`.

I reviewed the shared checkout on `dev/planar-v2-vernacular` at HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52`, with the
uncommitted X06-C plus C01 to C04 candidate on top. I wrote only this file and new files under
`tests/native/v5-02-x06-c-c04-evidence/review/`. I applied no mutants and edited no source, test, record, build file or existing
evidence. My compile-only probes compiled from my own scratch directory against the existing module files (BMIs), with
`-o /dev/null`. No other ninja or ctest process was running. I ran one command at a time:

- a no-work `planar_tests` build;
- the probes;
- one full Debug ctest;
- `ctest -N`;
- the lint;
- two no-work default builds.

I did not commit, push, merge, install or publish. I used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

I recomputed every identity row of the C04 final record with its stated pathspec (`review/raw/identity.txt`):

| Row (pathspec) | Record | Recomputed |
| --- | --- | --- |
| `git diff HEAD -- src` (tracked) | `179c4a8a…f8ba` | equal |
| `git diff HEAD -- src`, then the bytes of the six untracked `src` files in the listed order | `62432745…93fb` | equal |
| `git diff HEAD -- src` plus both `generate.py` (tracked) | `baa876b8…4939` | equal |
| the previous row, then the same six untracked files | `cc4eb86e…c1fb9` | equal |
| `git diff HEAD` (all tracked) | `618c6ceb…e494` | equal |

- All twelve per-file hashes in the record equal the working tree. That includes the new
  `profile_acceptance_seam.t.cpp` (`a24ba72d…`) and `acceptance.t.cpp` (`0188afa2…`). They also equal `probes/PROBES.md`
  and the gate `REPORT.md`.
- `runtime.cpp` is unchanged at `a82d1abf…7c3c`. `git status` shows nothing under `src/engine/runtime` or `src/cmd`.
- The record lists the six untracked files explicitly. That list is needed: plain `git ls-files -o` order gives a different hash
  (`40851e18…`, recorded in `identity.txt`).
- The last candidate edit was `acceptance.t.cpp` at 15:39:10. The gate (15:46) and the final prober run (`PROBES.md`, 16:55)
  both postdate it.

## Independent verification

| Check | Result (raw under `tests/native/v5-02-x06-c-c04-evidence/review/raw/`) |
| --- | --- |
| Builds | `cmake --build build/rewrite-debug --target planar_tests` exits 0 with `ninja: no work to do.`. After my probes and ctest, the default target twice also exits 0 with no work (`build-*.log`, `.exit`). Neither my run nor the gate compiled the tree itself; the prober's scratch builds did. |
| Full Debug ctest | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` exits 0: "100% tests passed out of 1041", with 1041 `Passed` lines and labels unit 1034 and acceptance 7. There are no `***`, Not Run or Disabled lines. The only "skipped" match is a test name. The run took 374.43 s. #531 took **15.45 s**, against 16.11 s in the gate and 15.41 s in my C03 run, so it is not slower (`ctest-full.*`). |
| Inventory | `ctest -N` exits 0 with "Total Tests: 1041". On exact full names, the set equals the gate's `inventory.txt`. Against the C03 `inventory.txt` (1038), nothing is missing. The three new names are exactly the record's three C04 names. The two relocated direct-seam tests keep their names, at `profile_acceptance_seam.t.cpp:203` and `:265` (`inventory-now.txt`, `inventory-compare.txt`). Criterion 5 is met. |
| Lint | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` exits 0: "Formatting and Doxygen checked 142 active source files" (`lint.*`). |
| Diagnostics | The string literals of HEAD's `acceptance.{cpp,cppm}` are all present in the six current product units. The only addition is "Prepared profile differs from request intent", the X06-C message already analysed. This is the same result as in C03, so C04 changed no message (`diagnostic-literals.txt`). The pin tests pass in my run. Criterion 8 is met. |
| Seam shape (criterion 1) | `acceptance_seam.cppm:18-62`: `namespace planar::engine::ledger {` holds `export struct acceptance_result`, then the non-exported `namespace seam_detail {` holds the three entities, with no `export` anywhere inside. The implementation (`acceptance_seam.cpp:7`) and the three adapter calls (`acceptance.cpp:45`, `:52`; `profile_acceptance.cpp:49`) use `seam_detail::`. The rest of the partition structure is unchanged from C03: `import :seam;` and `export import :profile;` in `acceptance.cppm:14-15`. A grep of `src/` finds no `seam_key`, `seam_grant`, `make_key` or `test_probe`, and no `friend` or `PLANAR_TEST` in any acceptance unit. Criterion 1 is met. |
| Scan (criterion 4) | Grepping `src/` for the scan's six tokens finds exactly the eight units in the allowed set of `acceptance.t.cpp:786-791`, and the test asserts `allowed.size() == 8` and equality. The new seam unit is in the set and `profile_acceptance.t.cpp` is out. Criterion 4 is met, but see C04-R1 for the call-count scan. |
| Split (criterion 3) | `profile_acceptance_seam.t.cpp` is `module planar.engine.ledger.acceptance;` with `import :seam;` and holds exactly the two direct-seam tests. `profile_acceptance.t.cpp` is a plain importer (`import planar.engine.ledger.acceptance;`) with 25 test cases and no seam name. I2-g shows the consequence the record states: dropping the re-export becomes a compile error in the plain importer. The seam unit duplicates about 150 lines of fixture helpers. That is acceptable, because first-party headers are not allowed outside `src/cmd` test harnesses. Criterion 3 is met. |
| Dependency audit (criterion 7) | Re-run read-only (`dependency-audit.txt`). Among ledger modules, `src/engine/runtime` imports only `planar.engine.ledger.completion` (`runtime.cppm:14`). The units of the acceptance module are the seven product and test units listed in the audit. `:seam` is imported by six of them. `:profile` is imported only by `acceptance.cppm`. Both generated variants contain the nested `seam_detail` and the omitted-profile comment, and do not mention runtime. The CMake consumers of `planar_ledger_acceptance` are `src/cmd/planar/CMakeLists.txt:7`, `:85`, `:141` and `src/engine/CMakeLists.txt:187`, `:325`, `:333`, `:355`, plus the acceptance tests. That matches the C04 record list exactly. The full suite passes, so every variant built. Criterion 7 is met, but see C04-R2 for the C03 copy of the list. |
| Generators | Both `generate.py` diffs differ only in variant names and in the tuple format each file already used. Each check raises unless there is exactly one `namespace seam_detail {` and no `planar::engine::ledger::seam_detail` spelling, and the seam interface must contain no `export namespace`. The single `namespace planar::engine::ledger {` rename still matches. The generated `acceptance_seam.cppm` under `build/rewrite-debug/src/engine/r14c3b2-generated/` has the nested namespace inside `atomic_test`. |

## Negative-compile probes (criterion 2, decision B)

I copied the prober's 29 probe sources unchanged. r7, r8, c1 and c2 are byte-identical to my C03 review sources. I added five
probes of my own and compiled all 34 with the exact command line of `planar_command_invocation` from `compile_commands.json`
(`-std=c++26 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror @…/invocation.cpp.o.modmap`, cwd `build/rewrite-debug`,
`-o /dev/null`). The files are `review/raw/forge/run.sh`, `forge.log` (paths redacted) and `v1` to `v5`.

| Probe | Expected | Result |
| --- | --- | --- |
| r7 ADL forge with an `acceptance_result` tag (the C03-R1 route) | fails | exit 1: "use of undeclared identifier 'complete_native_candidate'" (it compiled on C03) |
| r8 control without the tag | fails, undeclared identifier | exit 1, same message |
| r12 ADL with an `acceptance_result` argument | fails | exit 1, undeclared identifier |
| v1 the r7 forge with a `format_error` tag | fails | exit 1, undeclared identifier |
| v2 the r7 forge with an `std::expected<acceptance_result, format_error>` tag | fails | exit 1, undeclared identifier |
| v3 reopen `seam_detail` in a plain TU and use a local tag type there (deliberate) | fails | exit 1, undeclared identifier |
| v4 `using namespace …::seam_detail;` then `&complete_native_candidate` | fails | exit 1: "declaration of 'seam_detail' must be imported from module 'planar.engine.ledger.acceptance:seam'" |
| r1 to r6, r10, r11, r13, n1 to n12 (C03 probes and the nested-name forms) | fail | all exit 1, with the diagnostics `PROBES.md` records |
| c1 (embedded overloads), c2 (`prepared_profile` overload) | compile | both exit 0 |
| l1b, and my v5 (a TU declaring `module planar.engine.ledger.acceptance;` names or takes the address of `seam_detail::complete_native_candidate`, v5 without `import :seam;`) | compiles (decision-B limit) | both exit 0. A new unit declaring itself part of the module is a recorded limit, and the scan would flag either under `src/` because of the `seam_detail` token. |

The prober's raw logs agree with these results:

- `probes/raw/negative-compile/final-negative.log` gives r7 and r8 exit 1, c1 and c2 exit 0, and l1b exit 0.
- Under mutants, r7 compiles (exit 0) for A1x, A2d and A1i (`A1x.probe-r7_adl_forge.log` and the others).
- r7 still fails for A1, A1p and A1y.

Criterion 2 is met. No trusted-code route I tried reaches the seam. The remaining routes are the decision-B class, and the C04
record, the seam's `@file` comment and X06-C row 9 now state them.

## Closure of the C03 findings

| Finding | Status | Basis |
| --- | --- | --- |
| C03-R1 (ADL route) | Closed | The decision-A shape is described above. r7 fails with the CLI target's flags. The ADL row A1p is killed in the way the record now states it: A1x, which returns to the C03 shape (entities beside `acceptance_result` and exported from the partition), makes r7 compile, and its log ends `restored, hashes verified: True`. The Catch2 pins kill A1, A1x, A1y and A2-g by name (`A1.log`, `A1x.log`). The statements C03-R1 listed are corrected: the seam `@file` comment (`acceptance_seam.cppm:4-12`) names the threat model and its limits; X06-C row 9 is restated to decision B; the C03 final record's superseded banner (lines 171 to 176) and its review-outcome section say criteria 1 and 2 were not met as recorded. The C03 dispatch's authority layer 1 (lines 30 to 35) keeps its original wording without an inline note. It is a dispatch-time statement, and the C03 "superseded" banner and review-outcome section cover it, which I accept. |
| C03-R2 (red snapshot) | Described accurately; not closed as evidence | The C03 record (line 339) and README now describe `9376aab7…` as a hash of a file-name list. C04's own red check again saved no diff and no untracked bytes into `red/`, and the record says so. See C04-R3. |
| C03-R3 (C02 record) | Closed | Superseded notes are at C02 lines 27 and 62 and on the C14 repair paragraph (line 155). The C02 "Amendments" line (151) now names the `generate.py` ownership extension. |
| C03-R4 (consumers, folded targets) | Closed in the C04 record; one C03 statement still wrong | The C04 record (line 277) lists every consumer, and I verified the list. The C03 importer row (line 249) and the C03 evidence README (lines 33 to 35) record the folded variant targets and correct `REPORT.md` criterion 7. The C03 record's link-graph amendment is not corrected: see C04-R2. |
| C03-R5 (stale binaries) | Closed for C03; see C04-R4 for C04 | The C03 README (line 37) and the C03 findings table now call them stale mutated binaries, and the word "incidental" is gone from the C03 record. The C03 `REPORT.md` and `PROBES.md` keep the old wording, as unedited outputs of other agents, and the README corrects it. |
| C03-R6 (whole-file unit) | Closed by the split | See criterion 3 above. |

## Invariant table

- Every acceptance criterion and every closed finding has a row or a criterion entry: A1, A2, A1p (ADL), S1 to S4, I1, I2,
  I7 to I14, and CTRL.
- C03's I3, I3b, I4 and I5 are folded into S1 to S3, and I6 into CTRL. I1b (`export using native_candidate`) does not appear in
  the table, although `PROBES.md` has it as BUILD-FAILED: under C04 the export does not compile, which is stronger.
- I read the logs of A1, A1x, A2-g, S1, S2, S4, I1, I2-w, I8, I9, I10, I11, I12b, I13 and CY2. Each kill names the table's
  test, and each log ends `restored, hashes verified: True`.
- `results.jsonl` holds 131 rows: 93 KILLED, 27 BUILD-FAILED and 11 SURVIVED, as recorded.
- The accepted survivors C05, C19, X12, X18, X20, E6 and I5b survive again. D11, I14b, I14c and L3 are reported and are not
  table rows. I accept I14b and I14c as equivalent: the shape they unguard is already pinned by the A1 and A2 tests. I accept L3
  as a transitive dependency.
- Red-first: `red/raw/t1.log`, `t2`, `t3`, `t4` and `t6` are Catch2 assertion failures, not compile errors:
  - t1 (`namespaceseam_detail{` absent, line 852);
  - t2 (line 875);
  - t3 (`allowed.contains(file)`, 808);
  - t4 (seam-unit count `0 >= 1`, 968);
  - t6 (`REQUIRE(stream.good())` at line 58 because `profile_acceptance_seam.t.cpp` did not exist yet).
- t6 fails inside the read helper rather than on the test's own logic. It is still behavioural, because the missing file is
  exactly the pre-split state.

## Findings

### C04-R1 (required, records): the post-red repair of `acceptance.t.cpp` is undocumented, and the record still describes the earlier A2 pin

The record's "Repairs" point 1 says the specific defects the implementation coder reported "are not in the report text available
to this sync, so none is described here". Point 2 says the change "is known only from the hash chain". The gate's own raw log
from before the repair shows both defects. `tests/native/v5-02-x06-c-c04-evidence/raw/2-ctest.log` (13:57, exit 8 in `2.exit`,
"2 tests failed out of 1041") contains:

- `acceptance.t.cpp:978: FAILED: CHECK( counts.empty() )` with the message `engine/ledger/acceptance.t.cpp;`. The call-count
  scan counted the test file itself, because the new C04 pins contain the literal `"complete_native_candidate("`.
- `acceptance.t.cpp:874: FAILED: CHECK( contains(seam, "exportnamespaceplanar::engine::ledger{structacceptance_result{") )`.
  The implementation chose `namespace planar::engine::ledger { export struct acceptance_result {`.

The final file resolves these in two ways:

- It drops `acceptance.t.cpp` from the call count entirely, with `counts.erase("engine/ledger/acceptance.t.cpp"); // this file
  names the call in its own pins` (line 977). This is a narrowing of the second check that the record does not state. Its
  effect is small: `acceptance.t.cpp` is a plain importer, and my r10 and v4 show that a plain importer cannot name the seam
  even qualified. It is still a scan blind spot the record should name.
- It pins `namespaceplanar::engine::ledger{exportstructacceptance_result{` (line 877). The record's amendment bullet ("Wave 1
  narrowed … the A2 positive check `export namespace planar::engine::ledger { struct acceptance_result {`") therefore
  describes a pin that no longer exists. That breaks criterion 9 ("no section describes an earlier candidate").

The gate `g2-ctest.log` (14:17) already passed on an intermediate file. The final file (15:39) changed again; per the record's
probe comparison that later edit is what kills A1y, A1i and A2d with two tests and I4es. So two edits happened after the red
check, not one.

Required (records): describe both repairs from `raw/2-ctest.log` and the hash chain (red-time file, the intermediate
`abf22e8d…`, final `0188afa2…`). State the `acceptance.t.cpp` exemption from the call count as a scan limit, mitigated by the
language. Correct the A2 amendment bullet to the final pin.

### C04-R2 (required, records): the C03 record says its consumer list was completed, but it was not

C03 record line 297 now reads "Link graph (list completed by the C04 sync, finding C03-R4): every consumer … (`planar_command_invocation`,
`planar_ledger_format_tests`, `planar_source_export_tests`, `planar_export_fault_test_tests`, `planar_history_fixture`, the
acceptance tests)". That list still leaves out `planar_cli_invocation_0` to `_5` and `planar_load_cli_probe_invocation`, which
are exactly the targets C03-R4 named. The complete list exists in the C04 record (line 277) and the C03 evidence README
(line 35). The C03 line claims a completion it does not contain.

Required (records): complete the list on that line, or replace it with a pointer to the C04 list.

### C04-R3 (minor, evidence; recorded limit, not blocking): the red snapshot again omits untracked bytes

The C04 objective (C03-R2) and criterion 10 required the red check to save `git diff HEAD` plus the bytes or per-file hashes of
every untracked candidate file into `red/` before running. `red/` holds only `RED.md` and `raw/`. The snapshot hash
`fe6980a9…` is `git diff HEAD -- src`, tracked files only. The record and evidence README say this plainly, and criterion 10
stays "partly met".

I do not block on it:

- The five expectRed logs are behavioural.
- Their failure content identifies the pre-implementation state by itself: no `seam_detail` opener, no
  `profile_acceptance_seam.t.cpp`, and zero seam-unit calls.
- The missing evidence cannot be made now without rebuilding a past tree.

This is the fifth review in a row to see the gap (X06-C, C01, C02, C03, C04). The orchestration's red step should create the
`red/snapshot/` files mechanically (diff plus untracked bytes) before handing control to the red coder, instead of relying on
the brief.

### C04-R4 (minor, evidence and records): "rebuilds every variant test target on every row" is not what the harness did

The objective asked that "the prober rebuilds all variant test targets on every row". The record says "on every row that
builds them", which is true but circular.

- `harness4.py` builds only `LTARGETS` (no `planar_atomic_format_test_tests` or `planar_metadata_format_test_tests`) for 49 of
  131 rows. Five of those rows (I10, CY3, L1, L2, L3) build a single target.
- Most of those rows still run the default test regex, which includes the 8 variant tests.
- I checked every such row in `results.jsonl`: none has a failing atomic or metadata variant test. So no row result was
  affected by a stale binary the way C03's I5b-r and P7b were.
- Several of those rows (B2, B2b, B2c, B2d) mutate `acceptance.cppm`, which the generators read, so their variant binaries were
  not rebuilt with the mutant. The rows' kills come from the rebuilt main targets.

State this accurately in the record. Future harnesses should rebuild the five default targets on every row whose regex includes
variant tests.

### C04-R5 (minor, records): the explanation for why A1 leaves r7 failing is a clang observation

The record's amendment and `PROBES.md` point 2 say that with the entities beside `acceptance_result` but not exported, "the
entities have module linkage and are not visible to a plain importer even through ADL".

- That is what the pinned clang does.
- [basic.lookup.argdep]/4, however, speaks of declarations "in the interface of a named module", not of exported declarations.
  Whether a non-exported declaration in an interface partition qualifies is at least arguable.
- The decision-A shape does not depend on this, because `seam_detail` has no associated entity of the module at all. That is
  the stronger argument, and it is the one the seam's `@file` comment makes.

Phrase the A1 note as observed behaviour of the pinned compiler, so that a later reader does not treat "un-nested but not
exported" as a safe fallback.

## What is sound

- The decision-A implementation is minimal. Only the namespace placement and the three adapter qualifiers changed. The
  generators guard the shape loudly, and both variants build and pass.
- The threat model of decision B is recorded verbatim in the C04 dispatch and restated in the final record, the seam's `@file`
  comment and X06-C row 9. The recorded limits match what I could and could not compile.
- The direct-seam split is clean. The plain importer now really checks the `:profile` re-export from outside the module (I2-g).
- C03-R3 and C03-R6 are fully closed. C03-R4 and C03-R5 are closed where it matters, in the C04 record and the C03 evidence
  README, apart from C04-R2.
- The prober reran every row after the final `acceptance.t.cpp` edit. Its before-and-after identity files are equal to each
  other, and also equal to my recomputation.

## Limits

- I applied no mutants. My statements about scans catching v5 and l1b under `src/` are argued from the scan bodies and the
  `seam_detail` token, not executed.
- My probes were compiled, not linked or run.
- #531 is one full-suite measurement per run.
- macOS Debug only; Linux was not run. `make test-all` was not run (not an agreed command). Carried unchanged: P01 masked, N1
  phase coverage unmeasured, N2 message open, and fresh profile acceptance refused at the public surface.
- The roadmap opening and `docs/architecture.md` (which has no text about the partition layout yet) are commit-step items and
  are not updated, as the record says.
- The redact check must be re-run at commit over the final file list, including the evidence folders. Many C04 logs contain
  local paths: `probes/raw/negative-compile/run.sh`, `final-negative.log`, the probe row logs and the `red/raw` logs. My own
  ctest, lint and build logs contain the repository path. My `forge.log` and `run.sh` are redacted to `<repo>` and `<scratch>`.

## Review evidence

Under `tests/native/v5-02-x06-c-c04-evidence/review/raw/`:

- `identity.txt`: HEAD, every pathspec hash (rows A to E), the `git ls-files`-order control, per-file hashes, status and
  mtimes. The first block's D and E lines are a zsh word-splitting mistake of mine and are superseded by the bash rerun below
  them.
- `build-planar_tests.*`, `build-all.*`, `build-all-2.*`: the no-work builds.
- `ctest-full.*`: the full Debug run.
- `ctest-N.*`, `inventory-now.txt`, `inventory-compare.txt`: the inventory check on exact full names.
- `lint.*`: the lint run.
- `diagnostic-literals.txt`: the literal comparison with HEAD.
- `dependency-audit.txt`: runtime imports, importers, module units, partition importers and the generated variant check.
- `forge/`: my probes `v1` to `v5`, `run.sh` (exact command, scratch path redacted), `forge.log` (all 34 probes, paths redacted)
  and `README.txt`. The other 29 sources are byte-identical to `probes/raw/negative-compile/`.

## Records re-check (2026-10-26, same reviewer)

Disposition: **request-changes, records only**. C04-R2, C04-R4 and C04-R5 are closed. C04-R3 stays a recorded limit. C04-R1 is
only partly closed. The new sentences are correct, but the same Repairs point keeps the sentence the finding was about, and that
sentence now contradicts the new text and gives the wrong edit as the repair. The amendment bullet also keeps a reason that is false
for the final interface. Both are fixed by editing the C04 record alone.

### Candidate unchanged

I recomputed every identity row and per-file hash (`tests/native/v5-02-x06-c-c04-evidence/review/raw/recheck.txt`).

- Rows A to E equal the record and my earlier `identity.txt`: `179c4a8a…`, `baa876b8…`, `618c6ceb…`, `62432745…`, `cc4eb86e…`.
- All twelve candidate file hashes and `runtime.cpp` (`a82d1abf…`) are unchanged.
- `git status` is the same list as at my review, plus this review file, which was already untracked when the record was synced.
- Since my last review log (17:12), the only files changed under `src`, `tests/native`, `docs`, `cmake` and `scripts` are:
  - `V5-02-X06-C-C03.md`;
  - `V5-02-X06-C-C04.md`;
  - `probes/PROBES.md`;
  - my own review files.

  `PROBES.md` is the prober's summary. Findings C04-R4 and C04-R5 named its header and point 2, so editing it was in scope.
- No raw log, `results.jsonl`, `REPORT.md`, `RED.md` or inventory changed.
- No source, test, build or evidence-data file changed.

### Finding by finding

| Finding | Status | Bytes checked |
| --- | --- | --- |
| C04-R1 | Partly closed, see C04-RC1 and C04-RC2 | **Correct:** Repairs point 2 (C04 record line 262) now names both defects. Its log lines are right: `raw/2-ctest.log:1479` is the `counts.empty()` failure with the message `engine/ledger/acceptance.t.cpp;`, and `:1516` is the `exportnamespaceplanar::engine::ledger{structacceptance_result{` failure. Point 2 also states the `counts.erase("engine/ledger/acceptance.t.cpp")` narrowing and why the language limits it, and it gives the final pin as it appears at `acceptance.t.cpp:877`. **Wrong:** the second sentence of the same point (line 263) and the amendment bullet (line 269). |
| C04-R2 | Closed | C03 record line 297 now lists `planar_cli_invocation_0` to `_5` and `planar_load_cli_probe_invocation` and points to the C04 list. |
| C04-R3 | Recorded limit, unchanged | No edit, as the fix section says. The follow-up (save the red snapshot before the red coder runs) is correctly left to the orchestration. |
| C04-R4 | Closed, with a minor leftover (C04-RC3) | The C03-R5 correction (line 279) and the `PROBES.md` header say plainly that the variant test targets were not rebuilt on 49 of the 131 rows. They name the B2 rows and say that no result was affected. |
| C04-R5 | Closed | The A1p amendment (line 270) and `PROBES.md` point 2 call ADL invisibility an observation of the pinned clang, not a guarantee of [basic.lookup.argdep]/4. They say "beside but not exported" is not a safe fallback. |

### C04-RC1 (required, records): Repairs point 2 still calls the change unknown and names the wrong edit as the repair

Line 263 is the old point 2, left in place after the new text:

> `acceptance.t.cpp` was then repaired (a tests-only edit; the repair report was not available to this sync, so the change is known
> only from the hash chain and file times): its hash changed from the first prober run's `abf22e8d...` to `0188afa2...`

This sentence has two problems.

1. **It contradicts the sentences before it.** Those sentences describe the change from `raw/2-ctest.log`, so the change is no
   longer "known only from the hash chain". Point 1 (line 261) has the same problem: it still ends "so none is described here".
2. **"then repaired" names the wrong edit.** It reads as if the two defects were fixed by the `abf22e8d…` → `0188afa2…` edit. They
   were fixed before that:
   - The first prober run was on `abf22e8d…` (`probes/superseded-1/raw/identity.txt`).
   - Its baseline already passes both failing tests: #56 `X06 C C03 only the adapters call complete_native_candidate` and #62
     `X06 C C04 acceptance_result stays outside the internal seam namespace`, with "100% tests passed out of 62"
     (`probes/superseded-1/raw/baseline-ctest.log`, extract in `review/raw/recheck.txt`).
   - The gate's `g2-ctest.log` (14:17) also passes.

The actual sequence has two steps:

1. **Red-time file to `abf22e8d…`.** The red-time file's hash is not recorded. Between 13:57 and 14:17, this edit fixed defects (a)
   and (b).
2. **`abf22e8d…` to `0188afa2…`.** This edit at 15:39 came after the first prober run. It strengthened the pins: A1y and I4es are
   now killed, and A1i and A2d are killed by two tests.

The phrase "Two edits therefore happened" also gives the wrong reason. Two defects do not prove two edits. The hash chain and the
`abf22e8d…` baseline are the evidence.

Required: rewrite points 1 and 2 as that two-step sequence, citing the superseded-1 identity and baseline. Delete the "known only from
the hash chain" sentence, and make point 1 refer to point 2.

### C04-RC2 (required, records): the amendment bullet keeps a reason that is false for the final interface

Line 269 still ends:

> because the interface legitimately opens with an exported enclosing namespace. Reason: the bare pins would fail on a correct tree.

The final interface does not open with an exported enclosing namespace:

- `acceptance_seam.cppm:18` is `namespace planar::engine::ledger {` and `:20` is `export struct acceptance_result {`.
- Both generators refuse any `export namespace` in the seam interface (C04 record line 234).
- Repairs point 2 itself says the interface does not contain it.

So the first narrowing's reason is false on the final tree. The brief's bare "no `export namespace`" pin would pass on the final tree.
The narrower `exportnamespaceseam_detail` pin loses nothing in practice only because the generators already refuse the wider shape at
build time; the literal A2 mutant shows this, since it is BUILD-FAILED.

Required: say that this was wave 1's assumption about the interface shape, that the implementation did not take that shape, and that
the generator check covers the wider pin. Alternatively, drop the reason clause.

### C04-RC3 (minor, records; not blocking)

- The CTRL row of the invariant table (C04 record line 306) still says "every variant test target rebuilt per row". This is the
  claim C04-R4 corrected elsewhere. It should say what line 279 says.
- The edited `PROBES.md` header (lines 7 to 9) is left with an unbalanced parenthesis ("own build directory (the configure exited 0;
  its output was not saved),", then "(Every row restores …"). This is cosmetic, but the sentence no longer parses.

### No other new false claims

I checked the rest of the edited text and found no other new false claim:

- The "Records fix after review" section's file list matches the files that changed.
- Its "no source, test, evidence data or review file touched" is true.
- The C03 line 297 targets are the ones named in `src/cmd/planar/CMakeLists.txt` and listed in the C04 correction.

### Re-check evidence

`tests/native/v5-02-x06-c-c04-evidence/review/raw/recheck.txt` holds:

- the recomputed rows A to E;
- the per-file hashes and status;
- the list of files changed after 17:12;
- the superseded-1 identity and baseline extracts;
- the `raw/2-ctest.log` failure lines;
- the namespace lines of the seam interface.

Local paths in it are redacted to `<repo>`. I ran no build, test or probe for this re-check, and edited nothing except this section and
that file.
