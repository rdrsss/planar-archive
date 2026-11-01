# V5-02-RA01-B-W2-C02 independent review (second corrective for RA01-B wave 2)

Disposition: **approve**. The round closes all six findings of the [C01 review](V5-02-RA01-B-W2-C01-review.md).
R1 is fixed where it had to be fixed. Both digest helpers now map `Canonical allocation failed` as well as
`Hash allocation failed` to the Q-C01-2 bound. No message literal was added or changed. The two source guards pin the
exact spelling, and they fail on the pre-fix product and under every R1 mutant the prober ran. R2's comment and row now
describe E02c, the mutant the test actually kills. R3's three live answers match the operator's answers in the session
transcript word for word. The delegated residual is labelled delegated everywhere. R4 to R6 are applied. The inventory
(1437 names) and the four production-embedded files are unchanged. I re-ran the full Debug suite myself (1437 of 1437).
I raise four minor findings (R1 to R4 below). None of them blocks the combined commit, and all can be fixed in records
after the commit or at the next sync.

I reviewed the frozen candidate (wave 2 plus C01 plus C02, uncommitted on HEAD `3343ad38`) against the
[round record](V5-02-RA01-B-W2-C02.md), the C02 `plan/INVARIANTS.md` and `plan/DECISIONS.md`, `red/RED.md` with its raw
logs, `probes/PROBES.md` with `probes/superseded-1/` and `probes/raw/`, the gate `REPORT.md` with `raw/gate2/` (including
`raw/gate2/linux/`), the C01 round record and its `plan/DECISIONS.md`, `red/RED.md` and README, `c02-pin.md`, and the
authoring contract (Messages and Pointer amendments). I read the C02 product delta in full, all five changed test files
at their changed hunks, and the code behind the R1 trace (`canonical.cpp:206-228`, `hash.cpp:12-30`,
`runtime.cpp:684-690`).

## Exact candidate

Output of identity.py (the workflow script), run from the repository root and pasted verbatim. Pathspec `src`
(`tests/native/v5-02-ra01-b-w2-c02-evidence/review/identity-src.txt`, which also holds the per-file table):

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src), tracked only | `7a206bc2cc84ef0bb69602bc83f11761a49932e553e90f2d67153208cee291aa` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `68097c45b99b1ea681a856099c0d9f9315134e37bceeb47c8b3946cc7d338151` |

Pathspec `tests/native` (`review/identity-tests.txt`):

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- tests/native), tracked only | `15899df20287472033b359e593096e723403b3c93716162626c6a5eb07684c70` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec tests/native, evidence excluded | `15899df20287472033b359e593096e723403b3c93716162626c6a5eb07684c70` |

| file | state | sha256 |
|---|---|---|
| `tests/native/path-redaction.json` | modified | `330e8586ff8aa7bf2ec7dcaa4fdfba8a6797dc33ccdc374d839cec5103849c98` |

Pathspec `docs`, taken before this review file existed (`review/identity-docs.txt`; header, plus the one doc row C02 changes):

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- docs), tracked only | `c2eb51383fbb0f3af4d8a304d37ce36c7f0ec45b43173675f04f66de1af90dd7` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs, evidence excluded | `e0738ee960894b185e4cc1ac140bd9138a83e1a808708b774682555de4b10f3a` |

| file | state | sha256 |
|---|---|---|
| `docs/vernacular/authoring-contract.md` | modified | `a9996f774eb68b84665dd38c8ce6a91c32bd7452f009d9b3cf0e5368521ba236` |

Boundary pin files (`review/identity-pin.txt`):

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src/engine/runtime/runtime.cpp src/stdlib/planar/commands.lua src/stdlib/planar/schema.lua src/stdlib/planar/semantic.lua), tracked only | `70222fe5328f101932b089438b8a394c885bd46f2741a8f63943c64f0a004804` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src/engine/runtime/runtime.cpp src/stdlib/planar/commands.lua src/stdlib/planar/schema.lua src/stdlib/planar/semantic.lua, evidence excluded | `70222fe5328f101932b089438b8a394c885bd46f2741a8f63943c64f0a004804` |

| file | state | sha256 |
|---|---|---|
| `src/engine/runtime/runtime.cpp` | modified | `9c8c15cff3bc42eaddb4066f26f32e8b9464be2426fef3b57ebee5f94448a71d` |
| `src/stdlib/planar/commands.lua` | modified | `f32c5be4b8ae3de2a36ded7b51081813bfe3c63ca3d3cfa0e58e702274cf10b7` |
| `src/stdlib/planar/schema.lua` | modified | `e71fa6d969602e23ab9ee61c8838ccf3d7f8a0118ad27b35e1d0504927d86fe9` |
| `src/stdlib/planar/semantic.lua` | modified | `82bfe3dcad643d9eff7eb7d4c75108b40d2dcfd93dee69be711ba5e5f65f89aa` |

The `src` header and per-file table, and the `tests/native` and pin blocks, equal the round record's "Candidate identity
(final)". `review/identity-src.txt` is byte-equal to the gate's `raw/gate2/identity-src.txt`. The `src tests/native`
combined value of the probe run (`d56c7fbb...`, `probes/raw/identity-before.txt` and `identity-after.txt`, which are
identical) carries the same per-file rows. So the gate, the probes and my own run all used these bytes. The Linux
container's `host-manifest.txt` lists the C02 hashes for `capture.cpp`, `capture.cppm`, `invocation.cpp`,
`capture.t.cpp`, `authoring_observe.t.cpp`, `directory.t.cpp` and `runtime.cpp`, and its context manifest equals the host
manifest.

Against the C01 review's `identity-src.txt`, exactly eight `src` files changed: `capture.cpp`, `capture.cppm`,
`invocation.cpp`, `capture.t.cpp`, `authoring_observe.t.cpp`, `authoring_orchestrator.t.cpp`, `authoring_prompts.t.cpp`
and `directory.t.cpp`. Among `docs/architecture.md`, `authoring-contract.md` and `command-contract.md`, only
`authoring-contract.md` changed. This is the round's owned set. No other source file moved.

## What I checked and found sound

- **R1 product, literal diff.** In `base/`, `capture.cpp` (`56c61dd3...`), `capture.cppm` (`fcfb2812...`),
  `invocation.cpp` (`4d2bcf73...`) and `authoring-contract.md` (`6fef9d65...`) equal the C01 review's per-file hashes. So
  `diff -U0` against `base/` is a true C01-to-C02 diff for these four files
  (`review/t1-and-product-diff-vs-c01.txt`). In each helper the only change is the doc comment and the condition, which
  becomes `error.code == "resource_limit" && (error.message == "Hash allocation failed" || error.message == "Canonical
  allocation failed")`. The return statements (`capture_limit("diagnostic_bytes", maximum)` and
  `capture_limit("observation_total_bytes", 4194304)`) are unchanged. `capture.cppm` changes only in its `@note`. No
  message literal was added. The only literal the diff adds is `Canonical allocation failed`, used as a comparison
  operand, and it already exists in `canonical.cpp:206,208,226,228`. No existing literal was edited. The match is
  correct: `canonical_encode` returns exactly `resource_limit` / `Canonical allocation failed` {`canonical_bytes`}, and
  `hash_value` passes it through unchanged (`hash.cpp:26-28`). The only other `canonical_bytes` result of the encoder is
  the size exhaustion at 256 MiB, which inputs bounded at 65536 bytes or 4 MiB cannot reach.
- **R1 guards.** `T37` (`capture.t.cpp:1064`, SECTION `file_digest allocation site`) and `T44`
  (`authoring_observe.t.cpp:770`) keep every C01 assertion. Each adds a check for the Canonical literal, an
  exact-spelling count of 1 for the whole condition plus return, and a count of exactly one `capture_limit(`. T44 now
  bounds the helper body by its signature and the first `#endif` instead of a 600-character window, and the source shows
  the helper's `#endif` follows directly (`invocation.cpp:119-127`). I compared both tests with the C01 bytes (see "T1
  diff base" below). Only assertions and the leading comment were added.
- **R1 red-first.** `red/identity.txt` is identity.py output at red time. It shows the three C01 product hashes with
  the final T1 test hashes, so the red build really was "C01 product plus new guards". `red/raw/test1.log` fails at
  `capture.t.cpp:1097` and `:1100`. `test2.log` fails at `authoring_observe.t.cpp:785` and `:789`. Both are assertion
  failures (`1 tests failed out of 1`), not compile errors. Tests 3 to 5 pass, as guards.
- **R1 probes.** I read the raw logs of C02-01 to C02-04, C02-35, C02-36, C02-05a to d, X-C02-a to e and C02-06
  (`probes/superseded-1/raw/`). Each shows the exact diff, a clean build, a named-test ctest with `out of 1`, the failing
  assertion lines, and a byte restore to the C02 hash with `verified=True`. Dropping the canonical branch is killed in
  both helpers (C02-01 at `capture.t.cpp:1097`/`:1100`, C02-02 at `authoring_observe.t.cpp:785`/`:789`). A separate
  `canonical_bytes` branch is killed only by the count check (C02-35 `:1101`, C02-36 `:790`), which is the reason that
  check exists. The wrong limit names and maxima are killed (C02-05a to d). The baseline was 228/228 before and after the
  run. The focused re-probe (`probes/raw/`) confirms E02, E02b and C01-37r survive both the named test and the broad
  228-test run, and that E02c is still killed. The 193-row ledger (`superseded-1/results.jsonl`: 153 killed, 17 killed by
  another test, 23 survived) matches PROBES.md, and the 23 survivors are the documented ones.
- **R1 records.** The C01 round record site table has the two remapped canonical rows and the Lua bridge pass-through
  row (lines 295-297 and 319-321). The bridge row's basis, D-C01-17 route (1), is right: `runtime.cpp:687-690` throws the
  native result unchanged. `plan/DECISIONS.md` rows 25 (D-C01-17), 35 (D-C01-27), 40 (D-C01-32), 83, 87 and 88 name both
  messages, each with an "Amended by C02" note.
- **R2.** The comment at `directory.t.cpp:342-344` now says the test guards `list()`'s `openat(dir, ".")` on a readable
  but unsearchable directory (E02c) and does not reach `fstatat` (E02, E02b). That is what the code does:
  `directory.cpp:112-114` fails before the `fstatat` at `:151` and `:187` is reached. The test name is unchanged. The C01
  record's C01-46 row and its E02/E02b row now say the same, and they record the missing red-first as a limit.
- **R3 against the operator's own answers.** I read the AskUserQuestion calls and their results in this workflow's
  session transcript. "Root unread" was asked at 19:25 UTC and answered `A: widen the new row (Recommended)`. "Swap race"
  and "Read fail" were asked at 20:39 UTC and answered `B: capture_changed (Recommended)` and
  `A: widen to 'read' (Recommended)`. The question texts, option labels and descriptions in the C01 record's three LIVE
  entries match those calls word for word. The residual entry is marked DELEGATED, and it reuses the "Delegated
  decisions" row verbatim. The C01 `plan/DECISIONS.md` rows for D-C01-04, 05, 08, 22 and 25 carry the same live or
  delegated status. Item 7 of the authoring contract credits Q-C01-1, D-C01-05 A and Q2 A to the operator, attributes the
  post-gate kind, identity and vanish remap to Q1 B (live), and calls the residual delegated. The `base/` diff shows only
  item 7 changed. No Messages row changed.
- **R4.** The two C01 Linux rows of `c02-pin.md` (lines 19 and 122) and the new section "C01 Linux gate rows" use the
  window wording and say that no production-embedded byte changed. They also say that Linux RelWithDebInfo was never
  freshly measured. No "pending" text is left. The production-embedded set really is unchanged. The `planar_embed_lua`
  list (`src/engine/CMakeLists.txt:82-92`) contains `commands.lua`, `schema.lua` and `semantic.lua`, which are byte-equal
  to the pin record, plus seven files that are unmodified against HEAD. The authoring Lua files are not in it.
- **R5.** The C01 `red/RED.md` has the "Limits (added by C02, R5)" paragraph: the snapshot is identified by the
  `candidate.diff` and `SHA256SUMS` hashes rather than identity.py, and C01-46 has no red-first record.
- **R6.** The `capture.cppm` `@note` gives the D-C02-17 classification. "Bisects" became "secant search"
  (`authoring_prompts.t.cpp:2047`). The orchestrator comment now names "package I-B, wave 3 of the C01 plan"
  (`authoring_orchestrator.t.cpp:206`). Concerns 2 and 8 are restated as non-blocking in the C02 record.
- **T1 diff base.** `base/` is not a pre-C02 copy for the five test files (F-C02-38), so I looked for C01 bytes
  elsewhere. `tests/native/v5-02-ra01-b-w2-c01-evidence/red/snapshot-2/` holds `authoring_observe.t.cpp` and
  `authoring_orchestrator.t.cpp` at exactly the C01 hashes. Applying that snapshot's `candidate.diff` to HEAD's
  `capture.t.cpp` (in a scratch directory) reproduces the C01 hash `d59633bb...`. The diffs
  (`review/t1-and-product-diff-vs-c01.txt`) show assertion and comment additions only in `capture.t.cpp` and
  `authoring_observe.t.cpp`, and a comment-only change in `authoring_orchestrator.t.cpp` (see R1 below). For
  `authoring_prompts.t.cpp`, the diff against the pre-GR-01 snapshot shows the C01 secant change plus the two C02 comment
  lines with an unchanged line count. `directory.t.cpp` has no C01 copy, and I judged its C02 change by reading the
  three comment lines.
- **Inventory.** Both build trees were current (`ninja: no work to do.`). `ctest -N` gives 1437 names in Debug and in
  RelWithDebInfo, and the sorted lists are byte-equal to each other, to the C02 `inventory.txt` and to the C01
  `inventory.txt`: 0 removed, 0 new. The Linux Debug and RelWithDebInfo inventories are equal to each other and, after
  re-sorting, to macOS (`review/inventory-check.txt`).
- **Gate, read from raw logs.** In `raw/gate2/`: every `.exit` file is 0. Lint checked 171 files. Both macOS ctest logs
  read `100% tests passed out of 1437`, with no Skipped or Not Run lines. The four no-work logs read
  `ninja: no work to do.`. `df` showed 68 GiB free before the Docker build. In `linux/`: the docker build, configure,
  build, ctest, inventory and no-work exits are 0, and `builder-prune.exit` is 0. Both container ctest logs read 1437/1437
  with `x07.two_process_cas_one_winner` and the boundary pin test Passed. The manifests compare equal. `docker images` is
  unchanged except for the age column. My own run of `ctest --test-dir build/rewrite-debug --no-tests=error
  --output-on-failure -j4` exited 0 with 1437 of 1437 passed (`review/ctest-debug-review.log`, paths replaced by `<repo>`,
  `<home>` and `<scratch>`).
- **Invariant table.** The round record's "Complete invariant table" copies every row of `plan/INVARIANTS.md` revision 4
  (C02-01 to C02-41; no C02-34 exists in any revision, so the gap is a numbering gap and not a lost row). It adds rows for
  the wrong-limit and exploratory mutants and the stated limits. Every acceptance criterion and every C01 finding R1 to R6
  has a row. For each mutation row I checked that the mutant breaks the stated invariant and that the named test is the
  one in the raw log's failure list. The rows NOT met (C02-27 review, C02-28 commit, C02-29 roadmap, C02-33, C02-38) are
  honestly marked. C02-27 to C02-29 are later stages by design. C02-33 and C02-38 are process deviations whose substance
  I checked independently (the C02 Linux pin result is in REPORT.md, and the T1 diff base is above).

## Findings

### V5-02-RA01-B-W2-C02-R1 (minor, test comment and records): the orchestrator comment reflow added a line, and the record says line counts are unchanged

The R6 edit at `authoring_orchestrator.t.cpp:205-210` re-wrapped the comment. The old last line ("... deleting a check
is killed by the source") became two lines: one ending at "killed by" and a stub line `// the source` on its own, followed
by the unchanged `// guard "RA01-B2 C01 slot type checks ..."` line. The file grew from 640 to 641 lines against the C01
bytes (`review/t1-and-product-diff-vs-c01.txt`). The change is still comment-only, and the meaning is right. But the round
record's "Amendments" section says "the T1 coder reports comment edits only with unchanged line counts". That is false for
this file. C02-18 requires line counts equal to the pre-C02 copies, and its status says that check "is not possible". It
was possible: the C01 red `snapshot-2` holds this file at the C01 hash. Fix (non-blocking): correct the record. C02-18 is
met for wording and failed for line count (+1, comment only). Optionally, re-join the stub line in a later round. The
round forbids no comment edit, but every test-file edit costs a re-probe, so a records fix is enough here.

### V5-02-RA01-B-W2-C02-R2 (minor, records): carried probe rows point at log paths that moved

The C02 `probes/PROBES.md` copies the first-pass tables "verbatim", including log cells such as `raw/C02-01.log`. The
round record's invariant table cites the same paths (for example, C02-01 cites "`raw/C02-01.log`"). After the survivor
repair those logs live in `probes/superseded-1/raw/`. `probes/raw/` holds only the seven re-run rows. A reader following
the link finds nothing. Fix (non-blocking): add one line to PROBES.md and the record saying that carried-row log paths
are relative to `probes/superseded-1/`.

### V5-02-RA01-B-W2-C02-R3 (minor, records): a red-first raw log was overwritten, and the deviation is not recorded as one

`red/RED.md` says the first attempt (without the `replacement.` prefix) "found no tests and was overwritten by the
corrected run". The standing rule is never to delete raw evidence logs. The overwritten runs matched zero tests, so they
carried no red evidence, and the behavioural red-first is intact. Still, overwriting a log should be recorded as a
deviation, not mentioned in passing. Fix (non-blocking): state it in the round record's "Amendments" or "Remaining
limits".

### V5-02-RA01-B-W2-C02-R4 (minor, contract wording): item 7 says Q1 B overrides the Messages closing paragraph but says nothing about the residual

Item 7 now says that Q1 B overrides "the Messages closing paragraph for `capture_diagnostic` only". The closing paragraph
says authoring capture keeps the messages render or load give for the same bytes. The delegated residual (D-C01-25) also
departs from that paragraph: for example, the native io `Cannot open confined input` after a passed gate becomes
`capture_changed`. Item 7 calls the residual "a delegated decision" but does not say it departs from the same paragraph.
The authority chain is recorded and correct (C01 "Delegated decisions", D-C01-25). The C01 review accepted D-C01-25 as a
sound reading of RA-2. So this is a transparency gap in the contract text, not a missing decision. Fix (non-blocking):
the next authoring-contract amendment can add "with the same `capture_diagnostic`-only departure" to the residual
sentence. A related nit in the same spirit: the `capture.cppm` `@note` phrase "digest or allocation failures (`bad_alloc`,
`Hash allocation failed`, `Canonical allocation failed`)" reads broader than it is, although the next sentence makes
clear that other digest results pass through.

## Evidence

All under `tests/native/v5-02-ra01-b-w2-c02-evidence/review/`:

- `identity-src.txt`, `identity-tests.txt`, `identity-docs.txt`, `identity-pin.txt`: identity.py output for `src`,
  `tests/native`, `docs` (taken before this file existed) and the four boundary pin files.
- `ctest-debug-review.log`: my full Debug run, 1437 of 1437, exit 0, paths replaced.
- `inventory-check.txt`: the inventory comparisons above.
- `t1-and-product-diff-vs-c01.txt`: the C01-to-C02 diffs of the T1 test files where C01 bytes exist (with the hashes
  that identify them and the reconstruction command for `capture.t.cpp`), plus the product and contract `diff -U0`
  against `base/`.
- `redaction-check.txt`: the redaction check over the committable files, this review and `review/`.

Commands I ran: the identity.py calls above; `cmake --build build/rewrite-debug --target planar_tests` and the same on
`build/rewrite-release` (both no work); `ctest -N` on both trees with sorted comparisons; `ctest --test-dir
build/rewrite-debug --no-tests=error --output-on-failure -j4` (exit 0, 1437/1437); `diff -U0` against `base/` and
against the C01 red `snapshot-2`; one `git archive` plus `git apply --include` of `snapshot-2/candidate.diff` in a
scratch directory outside the checkout; `shasum` searches for C01-hash copies of the T1 files; greps of the records,
plans, contract and source; a read of the session transcript's AskUserQuestion calls; and
`python3 scripts/redact-local-paths.py --repo-path <repo> --home <home> check` over the gate's 178-file list plus the C02
`README.md`, this review and every file in `review/`. It exited 0 with no output (`review/redaction-check.txt`). The
red-stage coder logs `red/guard1.log` and `red/guard2.log` still fail redaction, as the gate recorded, and must not be
staged. I applied no mutant. I edited
no file outside this review and the `review/` folder. I ran no script under `tests/native/`.

## Limits

- I did not rebuild or run RelWithDebInfo, and I did not run the Linux container. For those I rely on the `raw/gate2`
  logs, which I read.
- I re-ran no mutant. My probe verdicts come from the prober's raw logs and from reading the tests and the code.
- R1 is guarded by source-spelling guards only: no allocator fault can be injected, and no seam may be added. C01-37r
  (an early `return error;` that leaves the condition as dead code) survives the textual guard and is a stated limit. I
  accept this, as the C01 review accepted the same class.
- E02 and E02b (`fstatat` after a successful listing) and the C02-15a/b sub-window pin changes stay limits. The per-file
  hash comparison against `c02-pin.md` is the real pin, and any later edit of the four files must repeat it.
- `directory.t.cpp` has no C01 copy, so its comment-only C02 change was checked by reading, not by a diff.
- The gate's 178-file redaction list was built before the coordinator sync created the C02 `README.md` and edited the
  records. I re-checked them as described above.
- The root-aware tests record SUCCEED as root, so the Linux container run (root) does not exercise them. They ran for
  real on macOS as non-root.
