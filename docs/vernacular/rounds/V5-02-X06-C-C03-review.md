# V5-02-X06-C-C03 independent review (corrective for the C02 request-changes)

Disposition: **request-changes** (not records-only). This is the fourth review in the X06-C lineage. The operator approved a
fourth review on 2026-10-26; whether a fifth round runs or the residual below is accepted is the operator's call.

C03 does what the operator decision asked for, and does it cleanly:

- The seam is the non-exported interface partition `planar.engine.ledger.acceptance:seam`.
- The `prepared_profile` adapter is the partition `:profile`, which the primary interface re-exports with `export import`.
- Consumers keep `import planar.engine.ledger.acceptance;`.
- `seam_key`, `seam_grant`, `make_key`, `test_probe` and every seam friend are gone from `src/`.
- All three C02 forgeries now fail to compile with the CLI target's flags.
- Diagnostics are byte-identical. #531 is not slower. The inventory is exact. Every identity hash I recomputed matches the
  record.

One thing stops approval:

- C03-R1: the language rule does not make the seam unnameable from outside the module. A plain importer of
  `planar.engine.ledger.acceptance` can call `complete_native_candidate` by argument-dependent lookup. It can also pass a
  forged `native_candidate` subclass that it never names.
- I compiled this with the exact flags and module map of `planar_command_invocation` (exit 0). The object references the
  same seam symbol the library defines.
- The authority analysis, the seam's own `@file` comment and X06-C row 9 claim the opposite.
- The in-tree source scans would still catch this text under `src/`. So, for this one route, the protection is again
  language plus scan, which is the shape C02-R1 rejected.
- The fix is small and I tested its shape on a minimal module: move the seam entities into a namespace that holds no
  reachable entity of the module, or move `acceptance_result` out of the module. The alternative is the operator accepting the
  residual.
- Either way the seam's `@file` comment (a source file) has to change, so this is not records-only.

The other findings are records and evidence items that would not block on their own (C03-R2 to C03-R6).

I reviewed the shared checkout on `dev/planar-v2-vernacular` at HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52`, with the
uncommitted X06-C plus C01 plus C02 plus C03 candidate on top. I wrote only this file and new files under
`tests/native/v5-02-x06-c-c03-evidence/review/`. I applied no mutants and edited no source, test, record, build file or existing
evidence. My compile-only probes compiled new files from my own scratch directory against the existing BMIs, with objects
written to scratch. With no other ninja or ctest process running, and one command at a time, I ran:

- three no-work builds;
- one full Debug ctest;
- `ctest -N`;
- the lint;
- a final no-work build and lint after the probes.

I did not commit, push, merge, install or publish. I used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

I recomputed every row of the C03 final record's identity table with its stated pathspec (`review/raw/identity.txt`):

| Pathspec | Record | Recomputed |
| --- | --- | --- |
| `git diff HEAD -- src` (tracked) | `daced2fd…1ee8` | equal |
| `git diff HEAD -- src`, then the five untracked sources in the stated order | `5a9214f1…a1fb` | equal |
| `git diff HEAD -- src` plus both `generate.py` (tracked) | `866b2997…5194` | equal |
| `git diff HEAD` (all tracked) | `099386a6…acf2` | equal |

- All eleven per-file hashes in the final table equal the working tree, including the five untracked sources and both
  `generate.py` files. They also equal `probes/PROBES.md` (run 2) and the gate's `raw/07-identity.txt`.
- `runtime.cpp` is still `a82d1abf…7c3c`, and `git status` shows nothing under `src/engine/runtime` or `src/cmd`.
- The last source or test edit was at 11:45:48 (`acceptance.t.cpp`). The gate (11:46 to 11:53) and probe run 2 (`PROBES.md`
  12:58) postdate it.

## Independent verification

| Check | Result (raw under `tests/native/v5-02-x06-c-c03-evidence/`) |
| --- | --- |
| Gate raw exits | `raw/01.rc` to `raw/06.rc` all read 0. `raw/02-ctest.log`: "100% tests passed out of 1038", 1038 `Passed` lines; #531 took 15.51 s; the run took 371.56 s. `raw/04-lint.log`: 141 files. `raw/05` and `raw/06` report no work. These agree with `REPORT.md` and the record. |
| My full Debug ctest | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` exits 0: "100% tests passed out of 1038", with 1038 `Passed` lines and labels unit 1031 and acceptance 7. The only lines matching "skipped" are the test name "command load refuses forged family order skipped …". There are no `***`, Not Run or Disabled lines. The run took 371.26 s. #531 took 15.41 s, against 15.51 s in the gate and 16.52 s in my C02 review run. #531 lives in the runtime test binary, and runtime does not import the acceptance module, so it is not slowed. (`review/raw/ctest-full.*`) |
| No-work builds | `cmake --build build/rewrite-debug --target planar_tests`, then the default target twice, then once more after my probes: each exits 0 with `ninja: no work to do.` (`review/raw/build-*.log`, `.exit`). The gate's build also did no work, so neither run compiled the tree itself; the prober's scratch builds did. |
| Lint | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` exits 0 before and after my probes: "Formatting and Doxygen checked 141 active source files" (`review/raw/lint*.log`). |
| Inventory | `ctest -N` exits 0, "Total Tests: 1038", equal as a set to the gate's `inventory.txt`. 0 committed names missing out of 1003. 0 of the 28 X06-C and C01 names missing. Against C02 (1036), the five "X06 C C02 …" passkey names are removed and seven "X06 C C03 …" names are new, exactly as the record lists (`review/raw/inventory-now.txt`, `inventory-compare.txt`). Criterion 5 met. |
| Diagnostics | Compared with HEAD's `acceptance.{cpp,cppm}`, the string literals of the six acceptance and profile units lose nothing. They add only "Prepared profile differs from request intent", the X06-C message already analysed (`review/raw/diagnostic-literals.txt`). The new pin tests also kill D2 to D9 and E1. Criterion 8 met. |
| Partition structure (R1) | `acceptance_seam.cppm:12` `export module planar.engine.ledger.acceptance:seam;`. `acceptance.cppm:14-15` `import :seam;` and `export import :profile;`. `profile_acceptance.cppm:8` `export module planar.engine.ledger.acceptance:profile;`, which imports runtime and `:seam` without re-exporting either. The implementation units are `module planar.engine.ledger.acceptance;`. A grep of `src/` finds no `seam_key`, `seam_grant`, `make_key` or `test_probe`. Every `friend` under `src/` is outside the acceptance units and predates this lineage (capture, sqlite, format, storage, runtime). The adapters are anonymous-namespace classes. |
| Dependency audit | Re-run read-only (`review/raw/dependency-audit.txt`). Among the ledger modules, `src/engine/runtime` imports only `planar.engine.ledger.completion` (`runtime.cppm:14`). `:seam` is imported only by the six module units and `profile_acceptance.t.cpp`; `:profile` only by `acceptance.cppm`. No old module or target name remains outside the negative scan assertions in `acceptance.t.cpp`. A closure over the CMake DEPENDS and `target_link_libraries` edges finds no cycle, and `planar_runtime`'s closure does not contain `planar_ledger_acceptance`. Both generated variants (`build/…/r14c3b2-generated`, `v5-02-x03-b2-b-c01-generated`) contain `acceptance_seam.cppm` as `planar.test.<variant>_acceptance:seam`. Their `acceptance.cppm` has `// profile partition omitted from the generated variant` in place of the re-export, and no generated acceptance file mentions runtime. The variants are built by `planar_tests` and their tests pass. Criterion 7 met, but see C03-R4 for the consumer list. |
| Negative-compile probes | I wrote my own probes and compiled them with the exact command line of `planar_command_invocation` from `compile_commands.json` (`review/raw/forge/`, `run.sh`, `forge.log` with paths redacted). Importing `planar.engine.ledger.acceptance:seam` from a plain TU fails at parse level. The old module name fails with "module not found". Defining a grant class fails ("unknown type name 'seam_key'"). `std::bit_cast<seam_key>` fails ("no member named 'seam_key'"). `seam_grant::test_probe` fails ("no member named 'seam_grant'"). A qualified `&…::complete_native_candidate` fails ("must be imported … before it is required"). Both controls compile: embedded overloads (c1) and the `prepared_profile` overload (c2). An implementation unit naming `candidate_values` without `import :seam;` compiles (r9, the known limit). The ADL probe r7 compiles: see C03-R1. The prober's n1 to n12, c1, c2 and l1 logs agree with what I saw. |
| Red-first | The `red/raw/t1.log` to `t4.log` failures are Catch2 assertion failures at the lines `RED.md` names, and those lines hold the same assertions in today's file: t1 at 827 to 835, t2 at 776/779/809, t3 at 841 to 846, t4 at 883. They are behavioural. For the identity limits, see C03-R2. |

## Closure of the C02 findings

| Finding | Status | Basis |
| --- | --- | --- |
| C02-R1 (passkey not exclusive) | Closed as decided; residual in C03-R1 | The operator withdrew the passkey and chose the partition. The passkey, every grant and friend, and `test_probe` are gone. The three C02 forgeries fail with the CLI target's flags (my r3, r4 and r5; the prober's n5 to n7). Invariant rows I1, I1b, I3, I3b and I5 are killed by their named tests in `probes/raw/I1.log`, `I1b.log`, `I3.log`, `I3b.log` and `I5.log`, each ending `restored, hashes verified: True`. "No code outside the module can name the seam" still does not hold: see C03-R1. |
| C02-R2 (allowed-set count) | Closed | `acceptance.t.cpp:787-791` allows eight units and asserts `allowed.size() == 8` and `mentioning == allowed`. The C03 record says eight and lists them. The record states that the equality's lower bound is unprobed and why. |
| C02-R3 (amendments line, blind spots) | Mostly closed (see C03-R3) | The C03 "Amendments" line names the `generate.py` ownership extension and its use at line 65. The blind spots are restated against the partition: sources outside `src/`, generated sources, and a unit declaring itself part of the module. But the C02 record's own line 149 still reads "Amendments to earlier constraints: none" with no note, and the restated blind spots miss the ADL route (C03-R1). |
| C02-R4 (red snapshot hash) | Not closed (see C03-R2) | The C14 `expectRed=false` departure is now stated explicitly. The red snapshot is still identified only by a tracked-only hash (`df3828ed…`). The record's "untracked-inclusive" hash is the coder's reported hash of a file list. |

## Findings

### C03-R1 (required, product or operator, and records): a plain importer can call the seam by argument-dependent lookup and forge a candidate

`acceptance_result` is defined in the `:seam` partition (`acceptance_seam.cppm:19`), so it is attached to
`planar.engine.ledger.acceptance`. The primary interface re-exports it with `using` (`acceptance.cppm:18`), and every
`complete_acceptance` overload returns it. `complete_native_candidate` lives in the same namespace, `planar::engine::ledger`,
and is declared in a module interface unit of the same module.

Argument-dependent lookup ([basic.lookup.argdep]/4) therefore treats the seam entry point as visible to any TU whose call has
an associated entity attached to the module in that namespace, even when the TU only imports the primary interface. The
probe `review/raw/forge/r7_adl_forge.cpp`, abridged, does exactly that:

```cpp
import planar.engine.ledger.acceptance;
template <class Tag> struct any_candidate {
  template <class T> operator const T&() const {          // T deduced as native_candidate
    struct fake final : T { /* selects_profile, values, verify overridden */ };
    static const fake instance{};
    return instance;
  }
};
... return complete_native_candidate(p, i, any_candidate<planar::engine::ledger::acceptance_result>{}, std::nullopt, r);
```

- Compiled with the exact flags and module map of `planar_command_invocation` (`forge.log`), it exits 0. The forged
  `values()` names its return type through `decltype` on `T`, so the TU never writes `native_candidate` or `candidate_values`.
- `nm` on the object shows an undefined reference to `planar::engine::ledger::complete_native_candidate@planar.engine.ledger.acceptance(…)`.
  `libplanar_ledger_acceptance.a` defines that symbol (`T`). The forgery would therefore link: a CLI-side TU could supply its own
  trees, workspace id, interpretation bytes and an always-true `verify()`.
- The control r8 makes the same call without the `acceptance_result` template argument. It fails with "use of undeclared
  identifier 'complete_native_candidate'". So the route is exactly the association with an entity attached to the module.
- A minimal module with no Planar code (`review/raw/adl-mini/`, `run.txt`) reproduces this. With the entry point beside an
  exported type of the module, `user.cpp` compiles. With the entry point moved to a nested namespace (`n::seam`), it fails
  with "use of undeclared identifier".

In-tree, the second check would still catch this text: a `.cpp` under `src/` containing `complete_native_candidate(` fails
both "X06 C C01 seam is unreachable from a plain acceptance importer" and "X06 C C03 only the adapters call
complete_native_candidate". I argue this from the scan bodies; I did not apply it as a mutant. Sources outside `src/`, such as
`tests/native/*-support/*.cpp` compiled into test targets that import the module, are outside the scan.

These statements are therefore wrong:

- the C03 authority analysis layer 1: "no code outside `planar.engine.ledger.acceptance` can name the seam" and "a plain
  importer of `planar.engine.ledger.acceptance` also cannot name it";
- the seam's `@file` comment (`acceptance_seam.cppm:4-6`): "importers of the acceptance module cannot name native_candidate,
  candidate_values or complete_native_candidate";
- X06-C acceptance row 9: "structurally enforced in the final candidate by the non-exported partition";
- C03 criteria 1 and 2 as "met" without this limit, and the C02-R3 restatement of blind spots, which omits it.

Required, one of the following:

- Close the route. For example, declare `candidate_values`, `native_candidate` and `complete_native_candidate` in a namespace
  that contains no reachable entity attached to the module. A nested `planar::engine::ledger::seam` works in the minimal
  repro. Alternatively, define `acceptance_result` outside the acceptance module, for example beside `completion_receipt`.
  - Mind the generators: both rewrite `namespace planar::engine::ledger {` by exact count.
  - Add the ADL probe (r7 or equivalent) to the negative-compile set, so that it must fail.
  - Add an invariant row whose mutant puts the seam back beside `acceptance_result` and whose probe then compiles.
- Or take the residual to the operator as an amendment to the 2026-10-26 decision: language rule plus the two name scans, with
  sources outside `src/` uncovered. Record it verbatim.

In both cases, correct the seam's `@file` comment and the record statements listed above.

### C03-R2 (minor, evidence and records): the red-first snapshot is still identified by a tracked-only hash

- `red/RED.md` identifies the red tree as `sha256(git diff HEAD -- src)` = `df3828ed…`. At red time the converted layout
  lived largely in untracked files (`acceptance_seam.*`, `profile_acceptance.*`), so this hash does not identify the snapshot.
  This is the fourth review to raise it (X06-C, C01, C02, now C03).
- The record presents `9376aab7…` as "the red coder's own pre-edit snapshot hash, which includes untracked files". By its own
  description it is a hash of the output of `git ls-files -m -o --exclude-standard -- src tests/native docs`, which is a list
  of file names, not their contents. It exists only in the coder's report and was not saved.
- `red/raw/build.log` shows only `ninja: no work to do.`, so the red run does not show its own build.
- The four expectRed logs are behavioural, and the failing lines hold the same assertions in today's file. I accept them as red
  evidence for the test bodies, as I did in C02, with this identity limit recorded.

Required (records): describe `9376aab7…` accurately, as a hash of file names that does not identify contents, and keep
criterion 10 at "partly met". The orchestration should make the red coder save `git diff HEAD` plus the bytes of every
untracked candidate file (or their per-file hashes) into `red/` before the run; otherwise this recurs.

### C03-R3 (minor, records): passkey statements left uncorrected in the C02 record

The C03 objective lists "the C01 and C02 authority analyses" for correction. The C01 authority analysis carries an in-place
"(Superseded …)" note. The C02 record does not:

- Its authority analysis, the dispatch section at lines 25 to 44, still says "The passkey closes this at compile time … fails to
  compile. This is the structural enforcement …", with no note.
- The C02 "Repair: C14 registry-tree limit fixture" paragraph (line 153) still says "backed by the passkey".
- Line 149 still reads "Amendments to earlier constraints: none", although C02-R3 asked that it name the `generate.py`
  ownership extension. The C03 line now names it.

The superseded banner sits at the head of the C02 final record and covers only "every statement below". Add one-line
superseded notes at these three places.

### C03-R4 (minor, records): the link-graph and importer statements are incomplete

- The record lists the consumers that now also link `planar_runtime` through `planar_ledger_acceptance`. It omits
  `planar_cli_invocation_0` to `_5` and `planar_load_cli_probe_invocation` (`src/cmd/planar/CMakeLists.txt:85`, `:141`), and
  therefore the `planar_cli_variant_*` and `planar_load_cli_probe` executables.
  - Those targets carry their own renamed runtime (`planar.engine.cli_bundle_N`, `planar.engine.load_cli_probe`).
  - I checked that the built `planar_cli_variant_0` and `planar_load_cli_probe` contain no `@planar.engine.runtime` symbols
    (the only `planar.engine.runtime.*` symbols are from the pre-existing `closure_guard` module). So no real runtime code is
    pulled in today, but the edge exists and should be stated.
- The importer row for `src/engine/CMakeLists.txt` says "the generated variant targets keep their names". The dispatch's
  `planar_atomic_acceptance_seam_test` and `planar_metadata_acceptance_seam_test` targets were removed and folded into
  `planar_atomic_acceptance_test` and `planar_metadata_acceptance_test`. `REPORT.md` criterion 7 still names the removed targets
  as building, and the evidence README does not correct it.

### C03-R5 (minor, evidence): incidental probe failures are stale binaries, not flakiness

`probes/PROBES.md` and the record count I5b-r as "4 tests failed" and P7b as "2 tests failed". They describe the extra
failures as incidental ("a metadata-variant failure that run 1 also recorded as incidental"), and `REPORT.md` calls them
"earlier incidental flakiness".

- Both rows rebuilt only `planar_ledger_acceptance_tests` and `planar_ledger_format_tests`. The atomic and metadata variant
  test binaries were therefore left over from the previous row's build.
- The extra failures equal those of the previous row: S3, before I5b-r, fails tests 160 and 168, and E4b, before P7b, fails
  test 168 (`results.jsonl` order; `S3.log`, `E4b.log`).
- They are stale mutated binaries. The named kills of both rows come from rebuilt binaries and stand.

Correct the description in the C03 record and the evidence README. Future harnesses should rebuild the variant test targets on
every row, or rebuild them after each restore.

### C03-R6 (minor, records): the direct seam test unit is wider than its justification

Criterion 3 allows a direct seam test only as a module implementation unit, "justified in the record".

- Only two tests in `profile_acceptance.t.cpp` drive the seam directly: "X06 C seam interns an effective stdlib closure …" and
  "X06 C C01 seam interns the registry tree …". They need fake candidates because a real `prepared_profile` cannot exceed those
  limits, which the test comments (lines 774 to 776, 870 to 872) explain and the record does not.
- The whole file, with about 30 adapter-path tests, became a unit of the module. Those tests now see module-internal
  declarations. For example, `STATIC_REQUIRE(completes<const prepared_profile&>)` in "X06 C profile completion surface accepts
  only the opaque prepared profile" would pass even if the `:profile` overload were not exported. That export is shown instead
  by I2-g and control c2.
- Either justify the whole-file choice in the record and state this consequence, or move the two direct tests into a small
  separate implementation unit.

## What is sound

- The partition conversion matches the operator decision. The generators handle it without a runtime dependency, with an
  exact-count check on the replaced re-export. The variants build and pass.
- Probe run 2 is on the final candidate (identity equal to the gate's). It has 91 rows: 76 killed, 7 survived, 7 build
  failures (none counted as kills) and 1 SegFault kill (C16, named in its log).
- I read the logs of I1, I1b, I3, I3b, I4, I5, I8, I2-g, CY2, C16, I5b-r and P7b. Each kill names the table's test, and each
  log ends `restored, hashes verified: True`.
- The three repair tests (message pins and the runtime layering scan) close probe run 1's F1 and F2. As guards, their evidence
  is the probe kills of D2 to D9, E1 and CY2.
- The standing carry-over rows hold: C14 (I8), X27 (I10), R1 (I11, I12), I7, I9 and I13 are each killed by the named test.
- Accepted survivors C05, C19, X12, X18 and X20 survive again. I5b survives by design (comment-only text), and its real-code
  twin is killed.
- E6, the embedded workspace id hard-coded to `quota`, is a new survivor. The record accepts it on X18's argument: every
  fixture names its workspace `quota`. I accept it for consistency with X18, but it is a fixture gap rather than an equivalence.
  A fixture with another workspace name would kill both.

## Limits

- I applied no mutants. The scan-catches-r7 statement in C03-R1 is argued from the test bodies.
- My forgeries were compiled, not linked or run. The symbol match shows they would link.
- #531 is one full-suite measurement per run.
- macOS Debug only; Linux was not run. Carried unchanged: P01 masked, N1 phase coverage unmeasured, N2 message open, and fresh
  profile acceptance refused at the public surface.
- The roadmap opening and `docs/architecture.md` are commit-step items and are not updated, as the record says.
- The redact check must be re-run at commit over the final file list, including the evidence folders. A check of the C03
  evidence folder today reports local paths in 20 gate and probe logs. My own ctest and lint logs also contain local paths; my
  `forge.log` is redacted to `<repo>` and `<scratch>`.

## Review evidence

Under `tests/native/v5-02-x06-c-c03-evidence/review/raw/`:

- `identity.txt`: HEAD, every pathspec hash, per-file hashes, status and mtimes.
- `build-planar_tests.*`, `build-all.*`, `build-all-2.*`, `build-final.*`: the no-work builds.
- `ctest-full.*`: my full Debug run. `ctest-N.*`, `inventory-now.txt` and `inventory-compare.txt`: the inventory check.
- `lint.*` and `lint-final.*`: my lint runs.
- `diagnostic-literals.txt`: the literal comparison with HEAD.
- `dependency-audit.txt`: module imports, importer lists, generated variant imports, CMake DEPENDS closure and cycle check.
- `forge/`: probe sources r1 to r9, c1 and c2, `run.sh` (exact command) and `forge.log` (paths redacted).
- `adl-mini/`: the minimal module repro of the ADL route and of the nested-namespace fix, with `run.txt`.
