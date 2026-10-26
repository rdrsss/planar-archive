# V5-02-X06-C-C02 independent review (corrective for the C01 request-changes)

Disposition: **request-changes** (not records-only). This is the third request-changes in the X06-C lineage, so by the
dispatch rule the chain stops here and goes to the operator.

C02 closes C01-R1 and C01-R3 in full and most of C01-R2 and C01-R4:

- The C14 test now installs the trace probe and asserts that no staging statement ran. After a fixture repair, the prober's
  C14 kill names that test alone.
- The no-Lua comment has been rewritten and agrees with X27.
- The C01 link-graph claim is withdrawn.
- Every identity hash I recomputed matches the record.
- The gate figures match their raw logs, and my own full Debug run agrees with them.

One thing stops approval. The passkey does not deliver what the operator decided:

- C02-R1: the operator asked for a key that only the two in-tree adapters can construct, so that any other importer "fails to
  compile even if it imports the seam by name". The candidate befriends a third class, `seam_grant::test_probe`.
- All three grant classes are declared `extern "C++"` inside the exported seam namespace, so any importer can define one of them
  and mint a key.
- The key is also an empty, trivially copyable class, so `std::bit_cast` mints one with no constructor call at all.
- I compiled all three forgeries with the exact flags and module map of the CLI target `planar_command_invocation`. Each one
  compiled with exit 0.
- The round's own test file mints keys exactly this way, from a TU that is not an adapter.
- The records, and the seam's own `@file` comment, say the opposite.

The other findings are records and evidence items that would not block on their own (C02-R2 to C02-R4). C02-R1 needs a
source change, or at least a corrected seam doc comment, and an operator decision, so the fix is not records-only.

I reviewed the shared checkout on `dev/planar-v2-vernacular` at HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52`, with the
uncommitted X06-C plus C01 plus C02 candidate on top. I wrote only this file and new files under
`tests/native/v5-02-x06-c-c02-evidence/review/`. I applied no mutants and edited no source, test, record, build file or existing
evidence. My compile-only spot checks (C02-R1) compiled new probe files from the reviewer scratch directory against the existing
BMIs, and the object files went to scratch. Serialized, with no other ninja or ctest process running, I ran:

- three no-change builds;
- one full Debug ctest;
- `ctest -N`;
- the lint.

I did not commit, push, merge, install or publish. I used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

I recomputed every row of the C02 final record's identity table with its stated pathspec (`review/raw/identity.txt`):

| Pathspec | Record | Recomputed |
| --- | --- | --- |
| `git diff HEAD -- src` | `e9f10163…cb582` | equal |
| `git diff HEAD --` both `generate.py` | `fcf5903e…15fa` | equal |
| `git diff HEAD -- src` plus both `generate.py` | `f8226750…2032` | equal |
| `git diff HEAD -- docs/vernacular/roadmap.md` | `af333153…a4b4` | equal |
| `git diff HEAD` (whole tree, tracked) | `5bcd9cc3…4b4e` | equal |
| `src` diff, then the five untracked sources in the stated order | `fff9ee64…6de4` | equal |
| `src` plus `generate.py` diff, then the five untracked sources | `0c02c326…f3e9` | equal |

- All eleven per-file hashes in the final table equal the working tree, including the five untracked sources and both
  `generate.py` files. The six files the record marks "Changed by C02: no" keep their C01 hashes.
- `runtime.cpp` is still `a82d1abf…7c3c`. `git status` shows nothing under `src/engine/runtime` or `src/cmd`.
- The last source or test edit was at 08:48:42 (`profile_acceptance.t.cpp`). Gate run 3 (08:56) and probe run 2 (09:35)
  postdate it. The prober's `identity.txt` reports a byte-identical scratch copy.

## Independent verification

| Check | Result (raw under `tests/native/v5-02-x06-c-c02-evidence/`) |
| --- | --- |
| Gate raw exits | All six `raw/g3-*.exit` files read 0. `raw/g3-02-ctest.log`: "100% tests passed out of 1036", 1036 `Passed` lines; #531 took 15.67 s; the run took 378.31 s. `raw/g3-03-lint.log`: "Formatting and Doxygen checked 141 active source files". |
| My full Debug ctest | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` exits 0: "100% tests passed out of 1036", 1036 `Passed` lines, labels unit 1029 and acceptance 7, and no `***`, Not Run or Disabled lines. The run took 391.36 s, against 378.31 s in the gate, so the machine ran about 3 percent slower. #531 took 16.52 s, against 15.67 s in the gate, 15.70 s in my C01 review run and 15.27 s in the C01 gate. That is noise on one shared machine, and well inside the 30 s limit. #531 lives in `runtime.t.cpp`, whose binary links no ledger acceptance module (runtime imports only `ledger.completion`), so C02 cannot reach it. Not slowed. (`review/raw/ctest-full.*`) |
| My lint | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` exits 0: "Formatting and Doxygen checked 141 active source files" (`review/raw/lint.*`). |
| No-work builds | `cmake --build build/rewrite-debug --target planar_tests`, then the default target twice: each exits 0 with `ninja: no work to do.` (`review/raw/build-*.log`, `.exit`). My later compile-only probes wrote nothing into the build tree; a final no-work build after them is in `review/raw/build-final.*`. |
| Inventory | `ctest -N` exits 0, "Total Tests: 1036". Compared as sets: 0 removed against the 1003 committed names (`v5-02-x06-b3-c-c02-evidence/inventory.txt`) and 0 removed against the 1031 C01 names. There are 5 new names, all "X06 C C02 …" in `profile_acceptance.t.cpp`, and 33 new names in total over the committed list (28 + 5) (`review/raw/inventory-now.txt`, `inventory-compare.txt`). Criterion met. |
| Diagnostics | Compared with HEAD's `acceptance.cpp`, the string literals of `acceptance.cpp`, `acceptance_seam.cpp` and `profile_acceptance.cpp` lose nothing. They add only "Prepared profile differs from request intent", the X06-C message already analysed under X06C-R3, plus the linkage string `"C++"` of `extern "C++"`, which is not a diagnostic (`review/raw/diagnostic-literals.txt`). The adapters' inline refusals keep their codes and texts, so diagnostics are byte-identical to C01. |
| Dependency audit (criterion 7) | Re-run on the final tree (`review/raw/dependency-audit.txt`). `src/engine/runtime` imports only `planar.engine.ledger.completion` from the ledger. The seam interface imports `storage_types`, `completion` and `acceptance.prepared`; its implementation adds `storage` and `tree_core`. `acceptance` imports the seam privately, and nothing leads back, so there is no cycle. `src/engine/CMakeLists.txt` is byte-identical to C01 (`94846903…`), so the link graph is unchanged, with DEPENDS still PUBLIC (`cmake/rewrite.cmake:23`). No `src/cmd` file names the seam. Met. |
| Shared body | `acceptance_seam.cpp` keeps the X06-C order. The only body change is the key parameter and `candidate.verify(key)` / `selects_profile(key)` / `values(data, key)`. The generator's `--mutate-final-receipt` substitution follows that line, and the variants build and pass. |

## Closure of the C01 findings

| Finding | Status | Basis |
| --- | --- | --- |
| C01-R1 (C14 test cannot fail) | Closed | `profile_acceptance.t.cpp:878-902` installs `installed_probe` before the seam call and ends with `CHECK_FALSE(installed.probe.pending)` (line 901). The fixture now uses two 9 MiB registry files. They exceed the 16 MiB provenance budget but fit both the 64 MiB source budget and the 16 MiB per-file cap (`tree_core.cppm:9-11, 23`). Under the mutant, the registry is therefore staged and refused only at pre-commit verification. `probes/raw/I-C14.log` fails exactly this test ("1 tests failed out of 57", restored with hashes verified). `c14-diagnostic/I-C14-output-on-failure.log` shows `CHECK_FALSE( installed.probe.pending )` failing with `!true` at :901, and the message check failing at :894. The superseded run 1 recorded the survivor and its specific observable (the 17 MiB file hit the per-file cap under every profile), as C01-R1 asked. That also corrects my C01 review's reading that the original mutant had been refused late. |
| C01-R2 (link-graph claim) | Closed for the link graph; replaced by C02-R1 | The C01 text is withdrawn in place (C01 record, authority analysis and "What the candidate is"). The C02 analysis says "The link graph is unchanged … Nothing in this round claims the link graph restricts reachability". But the restated analysis rests on a compile-time passkey claim that does not hold; see C02-R1. |
| C01-R3 (no-Lua comment) | Closed | `profile_acceptance.t.cpp:575-578` now says the test detects Lua-side activity under the write lock, that no ledger-side mutant reaches the VM, and that it is killed by X27, a scratch edit of `verify_native_receipts`. `probes/raw/I-R3-X27.log` fails only this test, and the restore is verified. The C01 record's R5 bullet now says the comment was not rewritten in C01. |
| C01-R4 (amendments) | Mostly closed (see C02-R3) | Criterion 4 is restated as met by probe substitution. The `5b5d9d20` combined hash is explained as not recomputable, and new combined hashes that include untracked files are given. The C02 owned-file list owns both `generate.py` files. The criterion 1 amendment and its blind spots are now written, but in terms of the passkey's supposed backing. |

## Findings

### C02-R1 (required, product and records): the passkey does not make a non-adapter importer fail to compile, and the round added a third friend against the operator decision

The operator decision (C02 record, verbatim) requires a key "whose constructor is private and that only the two in-tree
adapters … can construct via friend declarations, so any other importer fails to compile even if it imports the seam by name".
Acceptance criterion 1 repeats "only the two adapters are friends". The candidate does not meet this.

1. **Third friend.** `acceptance_seam.cppm:24-37` declares and befriends `seam_grant::test_probe` alongside the two adapters.
   - The record calls this "a decision of this round's coders" (C02 record, "What the candidate adds"). It was not escalated,
     although it changes an explicit operator decision.
   - Standing lesson 7 also covers it: a product type gains a privilege only so a test can drive it.
   - The record twice describes it as test-only: "in the test file only" (criterion 1 row) and "The `test_probe` friend exists
     only in the test file" (Remaining limits). Both are wrong. The forward declaration and the friend declaration are in the
     product interface, and they ship in every BMI of the seam.
2. **Any importer can define a grant class.** The three grant classes are forward-declared inside `extern "C++"` within the
   exported `planar::engine::ledger` namespace. They are therefore attached to the global module and visible to every importer
   of the seam, and any translation unit may supply their definition. `profile_acceptance.t.cpp:17-27` does exactly that: a
   non-module, non-adapter TU defines `seam_grant::test_probe` and mints keys.
   - I compiled three probes with the exact flags and module map of `src/cmd/planar/invocation.cpp`, a CLI target that is
     neither adapter (`review/raw/key-forge/`, `cmd.txt`, `forge.log`).
   - Control `f0_control.cpp` fails as the prober's N1 did: `calling a private constructor of class
     'planar::engine::ledger::seam_key'`, exit 1.
   - `f1_test_probe.cpp` defines `seam_grant::test_probe` and calls `complete_native_candidate`: exit 0.
   - `f2_embedded_adapter.cpp` defines its own `seam_grant::embedded_adapter` (no test grant involved), derives a forged
     `native_candidate` and calls the seam: exit 0. Linking it beside `acceptance.cpp` would be an ODR violation, but no
     diagnostic is required and the compile succeeds.
3. **`std::bit_cast` mints a key.** `seam_key` is empty and trivially copyable, so `std::bit_cast<seam_key>(unsigned char{0})`
   creates one with no constructor call and no access check. `f3_bit_cast.cpp` compiles with exit 0 and defines no grant class.
4. **The negative-compile probe missed these routes.** N1 to N4 only try direct construction or a missing key, and the prober
   ran them with the test target's flags. They are correct for what they try, and I re-checked their logs: N1, N3 and N4 fail on
   the private constructor, N2 on the missing key, and the control compiles. But they do not show that "any other importer fails
   to compile".
5. **No invariant row covers "only the two adapters are friends".**
   - I-PK5 scans only for which files contain the token `seam_grant`. The seam interface is an allowed file, so an added
     `friend` of any other class there, or a fourth grant name, would pass every listed test.
   - I-PK7-friend and I-PK7-friend2 only remove friends, and they were build failures.
   - I did not apply this mutant (reviewer remit). It is a reading of the I-PK5 test body (`profile_acceptance.t.cpp:1013-1029`)
     and of the friend list.

In-tree, the source scans still catch all three forgeries: f1 and f2 name `seam_grant`, and all three import the seam. So the
real protection is still convention plus a source scan of `src/**/*.cpp(m)`. That is the form C01-R2 described and the operator
chose to go beyond. These statements are therefore wrong:

- `acceptance_seam.cppm:8-11`: "any other code that imports the seam cannot construct a key";
- C02 record, authority analysis layer 2: "a named importer that is neither adapter cannot construct the key and fails to
  compile";
- C02 record: "the structural protection is the private `seam_key` constructor";
- C02 criterion 1 "met" and criterion 5: "blind spots … backed by the passkey, which does not rely on the scan";
- C01 record: "the structural enforcement is the private `seam_key` constructor"; "C02 added the passkey, which is [a
  compile-time proof]";
- X06-C acceptance row 9: "structurally enforced by the C02 passkey".

Required. These points need an operator decision, because they touch the operator's own wording:

- Either make the key exclusive in fact, or take the residual to the operator as an amendment.
- For exclusivity:
  - Remove the test grant, or obtain the operator's explicit acceptance of it.
  - Make the grant classes impossible to define outside their owners. For example, attach them to a module and define the
    adapters in that module's units. Mind the runtime-layering constraint that C01 recorded for the profile adapter.
  - Give `seam_key` a user-provided copy constructor, so that it is no longer trivially copyable and `bit_cast` no longer
    compiles.
- Add negative-compile probes for each route: defining a grant class, and `bit_cast`, from a CLI target. Each must fail on key
  construction.
- Add an invariant row and mutant for "only the two adapters are friends", for example a fourth friend added in
  `acceptance_seam.cppm`.
- If the operator instead accepts friend-by-name plus the scan, record that amendment verbatim with these blind spots:
  - grant-class definitions in any TU;
  - `bit_cast` or other object-representation copies;
  - sources outside `src/`.
- In both cases, correct the seam's `@file` comment and the record statements listed above.

### C02-R2 (minor, records): the I-PK5 allowed set is described as five files; the test allows four

The criterion 1 row says the scan "confines that name to the five allowed files". The test's `allowed` set
(`profile_acceptance.t.cpp:1015-1016`) has four entries: `acceptance_seam.cppm`, `acceptance.cpp`, `profile_acceptance.cpp` and
`profile_acceptance.t.cpp`. "What the candidate adds" lists the same four. Correct the count.

### C02-R3 (minor, records): leftovers of C01-R4

- The C01 record still says "Amendment to earlier constraints: none" (repair 4) and keeps criterion 1 and criterion 4 as
  "partly met". The C02 record carries the amendment text instead, which is acceptable because the C01 section is labelled
  superseded. The C02 "Amendments to earlier constraints: none" line should still name the `generate.py` ownership extension.
  The C02 coder edited `r14c3b2-support/generate.py` line 65 under it.
- The criterion 1 amendment's blind spots must be restated without "backed by the passkey" once C02-R1 is resolved.

### C02-R4 (minor, evidence): red-first snapshot hash again covers tracked files only

- `red/RED.md` identifies the stub snapshot as `sha256(git diff HEAD -- src)` = `769c5a2c…`. The passkey stubs lived in the
  untracked `acceptance_seam.cppm`, so this hash does not identify the snapshot the red run used. The X06-C and C01 reviews both
  asked that red-first snapshots hash untracked files (standing lesson 5).
- `red/raw/build.log` shows only `ninja: no work to do.`, so the red run did not show its own build.
- The five expectRed logs are behavioural: Catch2 assertion failures at the lines RED.md names, for example `t2.log` at
  977-979 and `t5.log` at 1023. I accept them as red evidence for the test bodies, with this identity limit recorded.
- The dispatch marked the C14 row test-first, but the invariant table gives it `expectRed=false`. The record explains this: the
  assertion checks correct existing behaviour, so the probe kill is the evidence. Say explicitly that this departs from the
  dispatch.

## What is sound

- C14 (C01-R1) and the no-Lua comment (C01-R3) are fully closed by test kills that I read in the raw logs.
- Probe run 2 has 61 rows: 49 killed, 5 survivors and 7 build failures, none counted as a kill. Each log I read ends
  `restored, hashes verified: True` and names only the listed test: I-C14, I-R3-X27, I-PK1, I-PK2 (two C02 tests), I-PK4d, I-PK5,
  I-R1a-r and C22. The literal rows that could not build (I-PK3, I-R1a, C02-r2, C02-r2-r and the three friend or copy rows) are
  recorded as BUILD-FAILED with corrected rows where possible.
- The five C02 runtime tests are not constant. Each failed in red and is killed by its own mutants: I-PK1 to I-PK4d are
  permissive forms (a keyless overload re-added, a public constructor, a protected default constructor), which is the stronger
  probe shape.
- Survivors C05, C19, X12, X18 and X20 remain the accepted equivalents. Nothing new survives.
- The R1 source-scan test is unchanged (`acceptance.t.cpp` hash equals C01) and is still killed by I-R1a-r and C22.

## Limits

- I did not apply mutants. The missing friend-set row (C02-R1 point 5) is an argument from reading the test, not a probe result.
- The forgeries were compiled, not linked or run. The claim is about compile-time exclusivity, which is what the operator
  decision states.
- #531 timing is a single full-suite measurement per run (see the table).
- macOS Debug only; Linux was not run. Carried unchanged: P01 masked, N1 phase coverage unmeasured, N2 message open, and fresh
  profile acceptance refused at the public surface.
- The roadmap opening and `docs/architecture.md` are commit-step items and were not updated, as the record says. The redact
  check must be re-run on the final file list at commit.

## Review evidence

Under `tests/native/v5-02-x06-c-c02-evidence/review/raw/`:

- `identity.txt`: HEAD, every pathspec hash, per-file hashes, status, mtimes.
- `build-planar_tests.*`, `build-all.*`, `build-all-2.*`, `build-final.*`: the no-work builds.
- `ctest-full.*`: my full Debug run. `ctest-N.*`, `inventory-now.txt` and `inventory-compare.txt`: the inventory check.
- `lint.*`: my lint run.
- `diagnostic-literals.txt`: the literal comparison with HEAD.
- `dependency-audit.txt`: imports, CMake DEPENDS, link-graph linkage.
- `key-forge/`: `f0` to `f3` probe sources, `cmd.txt` (exact command) and `forge.log` (paths redacted to `<repo>` and
  `<scratch>`).
