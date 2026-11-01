# V5-02-RA01-C-C01 independent review (orchestrator projection wrapper corrective)

Disposition: **request-changes, not records only**. The product fix and the new tests are sound, and finding
RA01-C-F1 is closed on the evidence. `project()` now returns `{version=1,sections}`. All five sections carry
`mandatory=true`, in the contract order. `orchestrator_command_impl` iterates `result.sections`, and the command output
bytes are unchanged. The new test T1 failed by assertion on the pre-fix product, and every dispatch mutant is killed by a
named test. No public field, method or hook was added. All 1437 names are kept, two names are added, and the Debug and
RelWithDebInfo inventories are identical. The boundary files match the c02 pin. The Linux gate ran on the frozen
candidate and includes `x07.two_process_cas_one_winner`.

One gate claim is false. The recorded redaction check ("exit 0 on 23 explicit files") cannot have checked those files.
Two committable evidence files still contain the absolute repository path (R1). The fix changes no source and no test,
so no rebuild is needed. It does change evidence bytes (`red-first.log` and `plan/DECISIONS.md`) and
`tests/native/path-redaction.json`, which is why this is not a records-only request.

I reviewed the uncommitted candidate on HEAD `93053c42` against the [round record](V5-02-RA01-C-C01.md),
`plan/INVARIANTS.md` and `plan/DECISIONS.md` (revision 4), `REPORT.md` (gate run 2), `red/RED.md`, `red-first.log`,
`red/snapshot/`, `probes/PROBES.md`, `probes/results.jsonl`, `probes/superseded-1/` and the raw logs under `raw/` and
`raw/linux2/`. I also checked authoring-contract.md:422-426 (Sections paragraph), the A07 row in
acceptance-matrix.md, rows C-15r and RA01-C-F1 in `tests/native/v5-02-ra01-c-evidence/acceptance-rows.md`, and
`tests/native/v5-02-ra01-b-w2-evidence/c02-pin.md:34-37`. I ran no script found under `tests/native/`.

## Exact candidate

identity.py (the workflow script), run from the repository root with pathspecs `src tests/native ':!tests/native/*-evidence'`
and pasted verbatim (also saved as `tests/native/v5-02-ra01-c-c01-evidence/review/identity.txt`):

| identity | value |
|---|---|
| HEAD | `93053c42a4c68fd3ecc8911edab6464e3d1307f3` |
| sha256(git diff --binary HEAD -- src tests/native :!tests/native/*-evidence), tracked only | `b17fda64e25a9683559256259c5c88b19563798acdfcdcbb22da1b82ec97b843` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native :!tests/native/*-evidence, evidence excluded | `b17fda64e25a9683559256259c5c88b19563798acdfcdcbb22da1b82ec97b843` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/authoring_orchestrator.t.cpp` | modified | `38d79424ab104ac3be416588bcc58d33715976fd781a6128e173b51c6a06a46f` |
| `src/stdlib/planar/orchestrator.lua` | modified | `e7d9f570ef70b014e6b3facac55709c59ea8736958bb5ef7d5147eb227bc87a1` |

This matches the round record, the gate report and both prober identity captures. `git diff HEAD -- src | shasum -a 256`
is `6b23117b...4aeda`, as the record states. The four record-file hashes in the record's last paragraph also match the
files on disk.

## Product change

`git diff HEAD -- src/stdlib/planar/orchestrator.lua` is 4 insertions and 3 deletions:

- The `section` helper adds `mandatory=true` to every entry, so each entry has exactly `id`, `text` and `mandatory`.
- `project()` ends with `return {version=1,sections=sections}`. The five `section(...)` calls are unchanged and keep the
  contract order: authority, graph, roles, prescriptions, actions. This matches authoring-contract.md:422-423.
- `orchestrator_command_impl` binds `local result=project(c)` and iterates `ipairs(result.sections)`. The joined text is
  the same, so the markdown and digest are unchanged.

No diagnostic string is touched, and no other caller of `project()` exists (`grep -n "project(" orchestrator.lua` shows
only the definition and the one call). `runtime.cpp`, `commands.lua`, `schema.lua` and `semantic.lua` each pass
`git diff --quiet HEAD`, and their sha256 values equal `c02-pin.md:34-37`. `c02_off_boundary` needs no re-pin, and the X08
boundary test passes.

## The new tests: what they observe and what they cannot see

`authoring_orchestrator.t.cpp` has 273 inserted lines and 0 deleted lines. No existing test body or name changed.

**T1, `RA01-C-C01 orchestrator projection wrapper is versioned and mandatory`, is behavioural with a source-pin
supplement.** Three of its four SECTIONs run real code. They read the real `src/stdlib/planar/orchestrator.lua` bytes
from the source tree and append `return orchestrator_command_impl`. They load the chunk in a test-built Lua state (base,
table and string libraries) and find the file-local `project` as an upvalue of `orchestrator_command_impl`. Then they
call `project()` with a minimal context and assert on the runtime value:

- `wrapper shape`: integer `version == 1`, exactly the keys `version` and `sections`, and no array part.
- `sections order and count`: five entries, with ids in contract order.
- `every section mandatory`: on each entry, a boolean `true`, exactly three keys, and text starting with `## `.

The fourth SECTION, `source pins`, is a squashed-source text pin in the style of the existing pin at :614. It pins the
wrapper literal, the helper literal, the `mandatory` counts, the call order and the consumer wiring. `lua_getupvalue` is
reflective access to a file-local function through the public Lua C API. It is not a product seam: no product type,
field, method or symbol was added, so lesson 7 holds. The Lua headers come in through `extern "C"` includes in a
`src/cmd/` test harness. `closure_guard.t.cpp` is an in-repo precedent for that, and the include and link work on macOS
and in the Linux container.

What T1 cannot see, stated plainly:

- It does not run through the embedded authoring stack, the production engine state or its sandboxed library set. It
  runs the same source bytes in a separate state.
- It does not exercise `orchestrator_command_impl` itself. The consumer is covered only by the source pin and by the
  existing behavioural test `RA01-B2 orchestrator prompt carries five sections in order`. That test kills the bypass
  mutant because iterating the wrapper with `ipairs` yields nothing.
- The minimal context has empty roles, graph, prescriptions and actions, so section bodies for non-empty inputs come from
  the existing goldens.
- The `mandatory` flag still has no behavioural consumer in the orchestrator command. The command never calls the runtime
  projection reader (runtime.cpp:366, :420), so the flag has no effect on output. That matches the contract: it requires
  the shape, and the prompt is the join of all sections, none of which is ever dropped. F1 is closed as shape
  conformance, and nothing more is claimed.

The record, REPORT.md, the test comment and plan D22 all state this limit honestly.

**T2, `RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins`, is a behavioural guard.** It
runs through the real CLI path (`orchestrate`). It pins the title, the authority sentences, the headings, the
single-blank-line joins and the trailing LF, using prefix and contains checks. It was added because the first probe run
showed that I6join, I6title and I6text survived the existing tests. T2 has no red-first run by design, since it pins
bytes the fix leaves alone. The nine I6 mutants show that it can fail.

## Red-first

The evidence is behavioural, and I verified it independently of RED.md's prose:

- `shasum -a 256 red/snapshot/candidate.diff` is `c2a1fda1...f09f74` and `SHA256SUMS` is `f9147844...a88b36`, as stated.
  The snapshot diff touches only `authoring_orchestrator.t.cpp`, so `orchestrator.lua` was at HEAD.
- I applied the snapshot diff to `git show HEAD:src/cmd/planar/authoring_orchestrator.t.cpp` in my scratch directory.
  The result (sha256 `379d82d7...2a3d`) differs from the final test file only by the appended T2 block. T1's bytes and
  line numbers at red time are therefore the final ones.
- `red-first.log` shows `ctest ... -R '<exact name>' --no-tests=error --output-on-failure` failing T1 by assertion. It
  shows `lua_type == 3` as `0 == 3` (version absent), `lua_rawlen == 0` as `5 == 0` (bare array), `keys == 2` as
  `5 == 2`, `REQUIRE(lua_istable)` failing on the nil `sections` in both per-entry SECTIONs, and the source pins failing
  (`returnsectionsend` count 1, `ipairs(project(c))` count 1). The load, upvalue lookup and `project()` call all
  succeeded, and there is no Lua error text. It is not a compile failure.
- The prober's PF row adds independent confirmation. It restored the whole of `orchestrator.lua` to the HEAD bytes (the
  mutated sha `a7f5ebb4...` equals `git show HEAD:src/stdlib/planar/orchestrator.lua | shasum -a 256`), and all four T1
  SECTIONs failed.

Gap (advisory A2): RED.md gives the red-time identity as prose hashes, not as verbatim identity.py output. The facts above
reconstruct it, so I do not treat the gap as blocking.

## Probes

`probes/results.jsonl` (9 re-run rows) and `probes/superseded-1/results.jsonl` (29 rows) give for each mutant the build
exit 0, a test exit of 8, the name of the failing test, the failing lines and `restored_verified: true`. A mutant that was
not applied, or a runner that was missing, would show neither of those. I read the mutant definitions in
`superseded-1/raw/driver/drv.py`. The dispatch mutants map as follows:

| dispatch mutant | probe id (exact edit) | killed by | failing lines |
|---|---|---|---|
| drop `mandatory` | I3a (helper without `,mandatory=true`) | T1 `every section mandatory`, `source pins` | 821, 822, 838, 868-873 |
| `mandatory=false` on one section | I3c1 `.mandatory=false`, I3c2 `["mandatory"]=false`, I3c3 string-built key, each after the roles call | T1 `every section mandatory` (I3c3 only behaviourally) | 822 (plus pins for c1, c2) |
| drop `version` | I1a `return {sections=sections}` | T1 `wrapper shape`, `source pins` | 750-752, 765, 861-862 |
| change `version` | I2a `version=2`, I2b `"1"`, I2c `1.0` | T1 `wrapper shape`, `source pins` | 752 / 750-751 / 751 |
| reorder sections | I4swap (runtime swap of entries 2 and 3), I3e (`table.insert(sections,1,...)`) | T1 `sections order and count` | 798 |
| bypass the wrapper in `orchestrator_command_impl` | I5 / I5b `ipairs(result.sections)` to `ipairs(project(c))` | T1 `source pins` (890-891) and `RA01-B2 orchestrator prompt carries five sections in order` (426) | |

The extra rows (I1b-d, I3b, I3d, I3f, I4del, I4add, X1-X7, I8, PF) are also all killed. The 9 I6 byte mutants are each
killed by T2 alone. The carried rows are still valid because the only change to the test file after the first probe run is
the appended T2. I confirmed that with the red snapshot comparison above, and `orchestrator.lua` has not changed since.
There are no survivors.

## Gate evidence, verified from raw logs

| check | what I read or ran | result |
|---|---|---|
| lint | `raw/g2-lint.log`; my own `clang-format --dry-run --Werror` on the test file | "Formatting and Doxygen checked 171 active source files", and the list includes `authoring_orchestrator.t.cpp`; my format run exits 0 |
| macOS Debug / RelWithDebInfo | `raw/g2-ctest-debug.log`, `g2-ctest-release.log` | "100% tests passed out of 1439" each, with no Failed, Not Run or Skipped lines |
| my focused run | `ctest --no-tests=error -R "RA01-C-C01\|RA01-B2 orchestrator\|X08 C01 role preparation outputs-off"` in both builds | 14 of 14 passed in each, both new names included |
| no-work builds | `raw/g2-nowork{1,2}-{debug,release}.log` | all four `ninja: no work to do.` |
| inventories | my `ctest -N` in both build dirs against the committed 93053c42 inventory (`review/inventory-check.txt`) | 1439 in each, Debug list equals RelWithDebInfo list; 0 removed, 2 added (T1, T2); `inventory.txt` equals ctest -N |
| Linux | `raw/linux2/container/*.exit`, `*-ctest.log`, `manifest-compare.txt`, `context-manifest.txt` | ten exits 0; 1439 of 1439 each preset; `x07.two_process_cas_one_winner` passed (#1430, 1.00 s and 0.44 s); the container context carries the two candidate hashes; host and container manifests equal |
| Docker hygiene | REPORT.md Linux section, `raw/linux2/builder-prune.exit` | about 64 GiB free before (floor 15), prune exit 0 |
| redaction | my own non-vacuous check (`review/redaction-check.txt`) | **fails** on 2 of 26 committable files; see R1 |

## Findings

### RA01-C-C01-R1 (blocking): the recorded redaction check was vacuous and two committable files leak the local path

REPORT.md (Redaction) and the round record (acceptance table, G6) say that `redact-local-paths.py check` exited 0 with no
output on 23 explicit files, including `red-first.log` and `plan/DECISIONS.md`. Those two files contain the absolute
repository path. `red-first.log` has 33 lines that contain it, for example "Test project <repo>/build/rewrite-debug"
and every Catch2 file:line. `plan/DECISIONS.md:27` (row D21) cites an `-isystem <repo>/vendor/lua/...` path. Both
files were last modified before the gate's check ran (02:49 and 02:30, against 04:32 for `raw/g2-redact.log`), so a real
check over them would have reported both. I ran the check with one argument per file over every committable file: the
seven edited repository files plus every file under `tests/native/v5-02-ra01-c-c01-evidence/` outside `raw/` and
`probes/raw/`, 26 files in all. It reports `local path in tests/native/v5-02-ra01-c-c01-evidence/plan/DECISIONS.md` and
`local path in tests/native/v5-02-ra01-c-c01-evidence/red-first.log` and exits 1. The positive control on `red-first.log`
alone also exits 1. `candidate_files()` silently drops any argument that is not an existing file. Passing the whole list
as one unsplit word (zsh does not split an unquoted variable) therefore gives exit 0 with no output, which is one way to
get the gate's empty `raw/g2-redact.log`. The gate did not save its exact command line, so the cause is not proven, but
the recorded result is wrong.

The round planned a `redact` step at commit time, but the commit procedure relies on this check to confirm the result.
Repeated the same way, it would pass again whatever the files contain.

Fix (no source, test or build change; no gate re-run beyond redaction):

1. Run `python3 scripts/redact-local-paths.py redact --repo-path <repo> --home <home>
   tests/native/v5-02-ra01-c-c01-evidence/red-first.log tests/native/v5-02-ra01-c-c01-evidence/plan/DECISIONS.md`. This
   records both files in `tests/native/path-redaction.json`, which is already owned by the commit package (D18) and must
   be staged.
2. Re-run `check` with one argument per file (for example through `xargs` or a bash array) over the full committable
   set. That set includes the `review/` files, this review file, the roadmap once edited, and `path-redaction.json`.
   Show non-vacuity: either print the file count, or keep a pre-redaction positive control.
3. Correct REPORT.md (Redaction) and the round record (acceptance row "Redaction check", G6, Limits) to say that the
   gate-run check was vacuous, and name the redact step and the new check output. Do not delete `raw/g2-redact.log`.

### Advisories (non-blocking; fold in with R1 where cheap)

- **RA01-C-C01-A1: `inventory-debug.txt` is stale.** It has 1438 names, lacks T2, and is byte-equal to the gate run 1
  list `raw/gate-names-debug.txt`. The round record's Evidence paragraph lists it among the evidence files next to
  `inventory.txt` with no label. Either label it in REPORT.md and the record as the gate run 1 list (superseded), or
  replace it with the gate run 2 Debug list (`raw/g2-names-debug.txt` sorted equals `inventory.txt`).
- **RA01-C-C01-A2: the red-time identity was not pasted as identity.py output.** RED.md states the hashes in prose. The
  reconstruction in the Red-first section above is enough to accept the red evidence. The record could cite this review
  for it instead of leaving the gap open.
- **RA01-C-C01-A3: the A07 text and rows C-15r and RA01-C-F1 still say "gate pending until round close".** At close,
  replace this with the gate run 2 result and this review. They name T1 and the D22 limit, which is the right test for
  F1. T2 guards the unchanged bytes (I6), so naming it there is optional.
- **RA01-C-C01-A4: REPORT.md (a) and (b) predate the focused re-probe.** They still say T2 is "NOT mutation-probed" and
  recommend a re-run. The round record already says the re-probe ran (9 of 9 killed). A one-line pointer in REPORT.md to
  `probes/PROBES.md` would stop the two documents from contradicting each other.

## Invariant table

The record's table copies all 22 rows of `plan/INVARIANTS.md` (revision 4). I compared the invariant, mutant, test,
testFile and expectRed cells programmatically, and they are identical. The record adds T2 and F1, which cover the test and
the finding this round closed. Every dispatch acceptance criterion has a row: I1-I5 and I5b for the mutants, I6 and T2 for
unchanged bytes, I7 and G4 for names and inventories, I8 for the boundary, G1-G3 and G5 for lint, ctest and no-work, G6 for
redaction, G7 for Linux, G8 for review and commit, G9 for the roadmap, and I9-I11 for records, carried findings and file
ownership.

Each mutation row's mutant breaks its invariant, and the probe logs show the listed test killing it, with the details in
Probes above. I6 lists the B2 test as its test, but the B2 test let three I6 mutants survive. The outcome column correctly
says T2 is the killer, and the T2 row records that. Rows with `expectRed=true` (I1-I5, R1, F1) all failed on an assertion
before the fix, as red-first.log and the PF row show. G6's outcome "partly met: check exit 0 on 23 files" is wrong (R1).
G7, G8 and G9 are correctly marked as owed at close.

## Closure check

- **RA01-C-F1 (row C-15r): closed.** The product meets authoring-contract.md:422. T1 observes version 1, the five ids in
  order and `mandatory=true` on each, behaviourally, within the D22 limit. The A07 cell, row C-15r and the F1 row all say
  so, with the limit.
- **RR-A1 to RR-A3 (RA01-C records advisories):** already applied at the RA01-C close (V5-02-RA01-C.md:268). This round
  adds only superseded-by-C-C01 labels to the RA01-C record and its REPORT.md. I checked the line references in the new
  REPORT.md note (lines 84 and 104) and they are correct.
- **F2 to F8:** the diff to `acceptance-rows.md` touches only the C-15r and RA01-C-F1 lines, so the rest stay carried to
  R15 or later.
- **File list:** `git status` shows exactly the files in the record's Round file list. The owed files are the roadmap,
  this review file, `path-redaction.json` (after R1) and the new `review/` evidence files written by this review.

## Limits

- I did not rebuild. I ran `ctest` on the existing build directories and relied on the gate's no-work logs. A
  `ninja -n` dry run prints a CMake re-run step because it does not execute the glob check, so it cannot prove no-work.
- I ran no mutants. The probe conclusions rest on the prober's logs and results files, which I read.
- Linux coverage is linux/arm64 only.
- Reviewer evidence files written by this review: `tests/native/v5-02-ra01-c-c01-evidence/review/identity.txt`,
  `review/inventory-check.txt` and `review/redaction-check.txt`. They contain no absolute paths, and they must be in the
  committable set and the redaction check.
