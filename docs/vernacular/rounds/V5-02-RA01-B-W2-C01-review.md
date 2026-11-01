# V5-02-RA01-B-W2-C01 independent review (corrective for RA01-B wave 2)

Disposition: **request-changes**. Most of this corrective is good. R3 is implemented properly, R4 is accurate, the
seven survivors named by the wave 2 review are now killed by small named tests, and the wave 2 inventory and gate hold.
But three findings block approval. R1 is the same class of defect the round exists to close: a `bad_alloc` while
`file_digest` canonically encodes the bytes produces `resource_limit` / `Canonical allocation failed`. That message is
off the Messages table, the round's remaps do not catch it, and the site table, which claims to be complete, does not
list it. R2 is a hollow invariant row. The 21st test (`C01-46`) claims a mutant its own prober showed it cannot kill, and
its comment says the same wrong thing. R3 is about amendment authority: the contract row and Pointer amendment item 7
cite operator decisions (`D-C01-05 A`, `Q2 A`, and `Q1 B` for the post-gate remap) that the round record never records
as questions or answers. R1 and R2 need source or test edits, so the fix is not records-only. R4 to R6 are minor.

I reviewed the final frozen candidate (wave 2 plus C01, uncommitted on HEAD `3343ad38`) against the round record (the
dispatch and the governing "Final record" section), `plan/INVARIANTS.md` and `plan/DECISIONS.md`, `probes/PROBES.md`
with `probes/superseded-1/`, `red/RED.md`, `c01-only.patch`, `c02-pin.md`, the gate `REPORT.md` with `raw/run3/`
(including `raw/run3/linux/`), the authoring contract (RA-2 to RA-7, Messages, Pointer amendments), and the wave 2
review findings R1 to R8. I read the C01 product delta in full: `capture.cpp`/`.cppm`, `directory.cpp`/`.cppm`, the
`invocation.cpp` digest helper and call site, and the two Lua slot checks. I also read every new C01 test and the
comments on the edited existing tests.

## Exact candidate

identity.py (the workflow script), run from the repository root, pasted verbatim. Pathspec `src`
(`tests/native/v5-02-ra01-b-w2-c01-evidence/review/identity-src.txt`, which also holds the per-file table):

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src), tracked only | `94eef45128431dd7bcd333cb9eb3e52099fd14a3a71eed763e8116fe30145db4` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `d7cbf9d6022e513412616f0fed32284b871d3a9f211f09ca747cbce78e4cdec6` |

Pathspec `docs/vernacular/authoring-contract.md docs/architecture.md docs/vernacular/command-contract.md`
(`review/identity-docs.txt`):

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- docs/vernacular/authoring-contract.md docs/architecture.md docs/vernacular/command-contract.md), tracked only | `1353fd08f15e9f684846189fb435ea4ac13c15f8b6965606d82d54c164d7b80c` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/authoring-contract.md docs/architecture.md docs/vernacular/command-contract.md, evidence excluded | `1353fd08f15e9f684846189fb435ea4ac13c15f8b6965606d82d54c164d7b80c` |

| file | state | sha256 |
|---|---|---|
| `docs/architecture.md` | modified | `0d7678071c9b986c472322be05b47e6fa833c2d12a51fa66a07810f039aa50ac` |
| `docs/vernacular/authoring-contract.md` | modified | `6fef9d65214fc6c3f4deb8bba4bc4a84462cf3e49718475488e828567be346dd` |
| `docs/vernacular/command-contract.md` | modified | `185bc18a35c8dda9314fdf2d788160ecba34488781401a72ab3b2fef7b3ac30b` |

The `src` identity and every per-file hash equal the round record's "Candidate identity (final)" `src` block. The gate's
run 3 and the focused re-probe ran on the `src tests/native` combined identity `1f30a26e...`, and the record shows that
it carries the same per-file hashes. So the gate and the probes ran on these bytes. I compared the per-file table with the
wave 2 review's `identity-src.txt`. Exactly seven product files and five test files changed in C01, and they are the
ones the record lists. `runtime.cpp`, `commands.lua`, `schema.lua`, `semantic.lua`, `src/cmd/planar/CMakeLists.txt`,
`authoring_orchestrator_variants.t.cpp` and `external_registration.t.cpp` are byte-identical to the reviewed wave 2
candidate. The seven files in `red/snapshot/` are byte-identical to the wave 2 reviewed bytes, so `c01-only.patch` is
the whole C01 product delta.

## What I checked and found sound

- **Q-C01-1 amendment, mechanically.** `git diff` of `authoring-contract.md` shows one added Messages row and Pointer
  amendment item 7 for C01. Nothing else in the Messages table changed, apart from the wave 2 RA-2/RA-7 hunks that were
  reviewed before. The row maps to the existing `io` / `Cannot inspect confined input` (exit 4). It adds no code and no
  message text, so the Messages introduction stays true. The row is wider than the dispatch's Q-C01-1 wording: it also
  covers the root, a race-swapped symlink root and a failed read. See R3 on where that authority is recorded.
- **Q-C01-2 and the race rows in `capture_diagnostic`.** ELOOP is `path_denied` / `Explicit input file is a symbolic
  link`. Any other failed open, and a failed `fstat`, is `io` / `Cannot resolve explicit input path`. A non-regular file
  is `io` / `Explicit input must be a regular file`. The outer `bad_alloc` gives `capture_limit("diagnostic_bytes",
  maximum)`. In `scan_once`, `bad_alloc` gives `observation_total_bytes` 4194304 through the private `scan_error`
  allocation flag. The recheck carve-out uses that flag, not the limit name, and C01-39 shows that a real second-scan
  overflow still reads as membership changed. All four message pairs already exist in HEAD product source
  (`response.cpp:189-192`, `source_capture.cpp:131`).
- **Delegated decision D-C01-25.** It maps every post-gate `add_input`, `read` and `finish` failure (other than a
  detail-less allocation result) to `capture_changed` / `File changed during read`. That is a sound reading of RA-2: the
  gate has just proved the same path openable, regular and bounded. The `sub/../diag.json` fixture of C01-34 drives the
  remap deterministically without a seam. The record marks this decision as taken under the standing delegation, not as
  a live operator answer.
- **Existing diagnostics byte-identical.** I recomputed the literal diff of `red/snapshot/` against the seven final
  files myself. The only removed message literals are `Cannot inspect confined input` at the two `capture_diagnostic`
  gate sites, `Only regular files can be captured` at the two race sites, and the bare detail-less `Capture limit
  exceeded` pair. Each was replaced by the decided row. The added literals are the decided pairs, the `Hash allocation
  failed` comparison and the two `Invalid embedded command protocol` slot refusals. No other diagnostic changed, the
  external-profile message table is untouched, and the full suite passes.
- **R1 site table against the source.** I checked the `directory.cpp`, `capture.cpp`, `invocation.cpp` and Lua
  citations in the "R1 site table (final source)" against the final bytes. Every line I sampled matches: `directory.cpp`
  :112-135, :151, :172-177, :187, :204-236, :257-258, :267-269, :277-296; `capture.cpp` :109-138; `invocation.cpp`
  :121-125, :391, :694, :703, :705; `observe.lua:4` and :63; `prompts.lua:335`. Concern 1 (a failed child-directory
  `fstat` is folded into the identity-change test) is source-only and reads defensibly as row 502. I accept it as a
  limit. The one gap is R1 below.
- **R2 survivors.** Each named mutant is killed by a named test, and I read the raw logs. Each log shows the exact-anchor
  diff, the build, a `1 tests failed out of 1` summary that names the test, and a byte restore with `verified=True`:
  X16 (`C01-09`, `capture_diagnostic refuses a symlink as a symbolic link`; dangling, 70000-byte and small-file links),
  X17 (`C01-05`, `observe refuses a symlink root`, engine level with a positive control), X03 (`C01-18`/`19`, 8192
  accepted and 8193 refused, with the canonical text typed in the test), X04 (`C01-20`/`21`, exactly 1048576 accepted
  and 1048577 refused), X05 (`C01-22`, with a control), X07 (`C01-23`, five non-digit forms plus a resolving control)
  and W2-51 (`C01-07` no-sort and `C01-08` reverse-sort, two disjoint name sets; both sections fail under no-sort on
  macOS, `raw/C01-07.ctest.txt`). X17, W2-51 and every row whose test file changed were re-run on the final candidate.
  X16, X03, X04, X05 and X07 are carried from the `afc66baa...` pass. That is valid, because the mutated file and the
  killing test file are byte-identical in the final candidate (the only change after that pass is `directory.t.cpp`).
- **R3 implemented.** `prompts.lua:335` and `observe.lua:4` refuse `internal` / `Invalid embedded command protocol`
  when the slot is not userdata. That pair already exists in the wave 1 stub and in RA-1. W2-06n (the runtime passes a
  text option instead of the opaque handle) is killed by the existing `RA01-B2 diagnostic and from reach Lua only as
  opaque slots` (`superseded-1/raw/C01-24.log`, `C01-25.log`). Deleting a check is killed only by the source guard
  C01-45, because the check cannot be reached on the shipped stack. The record states this honestly, and the test
  comment at `authoring_orchestrator.t.cpp:205-209` now describes the real mechanism. The W2 INVARIANTS row, the W2
  record rows and their pointers are updated. Neither Lua file is production-embedded (`src/engine/CMakeLists.txt:82-92`).
- **R4.** `docs/architecture.md` now describes the real handlers, `execute_authoring`, the separate
  `command_authoring_options` / `invoke_authoring` path with `command_options` closed at five, `capture_diagnostic`
  (65536 bytes), `planar_capture_directory` and the 22 variants. That matches `src/cmd/planar/CMakeLists.txt`
  (`foreach(variant RANGE 0 21)`, index table rows 0 to 21). The module-table rows Command, Capture and Product policy
  are updated. `directory.cppm` no longer has the `@note Stub`, and its `@return` lists the four codes.
- **R5 to R7 records.** These are stated in "R5 to R8 records": the Common rule 8 reading for `json_depth`, the private
  `{id,text}` projection, and `capture_diagnostic` / `capture_limit` exported unguarded but dead in production. R8:
  `c02-pin.md` closes the `planar_hash` blocker and lists `orchestrator.lua` `a7f5ebb4...`, which equals my identity.py
  run (`review/identity-orchestrator.txt`). See R4 below for the stale Linux rows.
- **No new public surface; production has no authoring path.** `capture.cppm` and `directory.cppm` changed only in
  Doxygen text. Every new helper is in an anonymous namespace. `observation_digest_failure` is inside
  `#ifdef PLANAR_AUTHORING_BUNDLE`. No new public field or method was added.
- **Inventory.** I ran `ctest -N` on both build trees. Each has 1437 names, and the sorted lists are byte-equal to each
  other and to `inventory.txt`. Against the wave 2 baseline (1416), 0 names are removed and 21 are added, and the added
  names equal `new-names.txt`. The run 3 Linux Debug and RelWithDebInfo inventories equal each other and, normalised,
  `inventory.txt` (`review/inventory-check.txt`).
- **Gate, verified independently.** `cmake --build build/rewrite-debug --target planar_tests` and the same for
  `build/rewrite-release` both printed `ninja: no work to do.` My own `ctest --test-dir build/rewrite-debug
  --no-tests=error --output-on-failure -j4` passed 1437 of 1437 with exit 0 and no skipped or not-run tests
  (`review/ctest-debug-review.log`). In the run 3 raw logs: lint exit 0; macOS Debug and RelWithDebInfo 1437/1437; four
  no-work logs; Linux Debug and RelWithDebInfo exit 0, 1437/1437, with `x07.two_process_cas_one_winner` passed in both;
  the host manifest equal to the container context; builder prune exit 0; and a redaction log over 72 explicit files that
  is empty with exit 0.
- **Red-first.** `RED.md` records 11 expectRed tests, each failing on an assertion (not a compile error) against the
  pre-fix snapshot, with the exact per-test ctest command. The behavioural ones (C01-09 to C01-12, C01-14, C01-34) fail
  on the old message pairs. The rest are source-guard assertions.

## Findings

### V5-02-RA01-B-W2-C01-R1 (blocking): a digest allocation failure in canonical encoding still yields an off-table message

`file_digest` calls `hash_value`, which calls `canonical_encode` (`hash.cpp:26-28`). `canonical_encode` catches its own
`bad_alloc` and `length_error` and returns `resource_limit` / `Canonical allocation failed` {`canonical_bytes`,
268435456} (`canonical.cpp:205-208`). `hash_value` passes that error through unchanged. The two C01 remaps match only
the message `Hash allocation failed`: `digest_failure` at `capture.cpp:93` and `observation_digest_failure` at
`invocation.cpp:122`. So an allocation failure while the diagnostic file or an observation entry is being encoded for its
digest reaches the user as `Canonical allocation failed` with limit `canonical_bytes`. Neither the message nor the limit
name is on the authoring Messages table, and neither is the `diagnostic_bytes` / `observation_total_bytes` refusal that
Q-C01-2 decides (as D-C01-27 and D-C01-32 apply it to this digest). The encoder's output buffer and `file_digest`'s copy
of the bytes are allocations of the same size, so this path is about as reachable as the `Hash allocation failed` path
the round chose to remap.

The round record's site table and D-C01-40 ("No row is silently omitted") claim the table is complete. D-C01-27 and
D-C01-32 state that an allocation failure in the digest "raises `Hash allocation failed`". That is only half true.
`Canonical allocation` appears nowhere in the round record, the W2 record, the C01 plan or the authoring contract
(`review/canonical-allocation-trace.txt`). The Lua digest bridge route (`prompts.lua:223`/`:230`, `orchestrator.lua`)
can return the same message through `runtime.cpp`, and the D-C01-17 pass-through row does not list it either.

Fix: in `digest_failure` and `observation_digest_failure`, map both allocation results, for example by matching
`resource_limit` with limit `canonical_bytes`, which both carry. Extend the source guards (C01-37 in `capture.t.cpp`,
C01-44 in `authoring_observe.t.cpp`) so they pin the new spelling. Add the canonical-allocation rows to the site table:
remapped in `capture_diagnostic` and in observe, and passed through with the D-C01-17 basis in the Lua bridge. Then the
prober re-runs C01-37 and C01-44 and a mutant that drops the canonical branch. This is a product and test edit, not a
records edit. No new message text is needed.

### V5-02-RA01-B-W2-C01-R2 (blocking): the C01-46 row and its test comment claim a mutant the test cannot kill

`RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect` (`directory.t.cpp:341`) says in its
comment (lines 342-344) that it kills the mutant that maps the `fstatat` failure after a successful listing to
`denied_name`. The C01-46 row in the record states the invariant as "entry inspection that fails after a successful
listing is io". The prober showed that both claims are false. A read-only directory without search permission already
fails `list()`'s `openat(dir, ".")`, so the `fstatat` mutants E02 and E02b survive, and only E02c (the `list()` openat
site) is killed. The record admits this under "Open items" item 1, but the committed comment and the row's invariant
column still describe a mechanism the test does not exercise. This is the same kind of defect as wave 2's R3 (a comment
and a row naming an unobserved mutant). The brief's rule that "a missing or hollow row is a finding" applies. The test
also has no red-first record.

Fix: correct the comment only (renaming is forbidden) to say it guards `list()`'s `openat(".")` failure on a readable
but unsearchable directory (E02c). Rewrite the C01-46 invariant and mutant cells to match. Keep E02 and E02b as stated
limits (they need a race or a fault seam). Optionally add a red-first note for E02c. This needs a test-file edit, so it
is not records-only.

### V5-02-RA01-B-W2-C01-R3 (blocking, records): the operator decisions behind the amendment are not recorded in the round record

The Messages row and Pointer amendment item 7 cite "Q-C01-1, D-C01-05 option A and Q2 (operator decisions
2026-10-31)". The row's extra scope comes from `D-C01-05 A` and `Q2 A`: the `--from` root, a race-swapped symlink root
and "read". `D-C01-08` and the post-gate remap rest on "Q1 option B". The round record's "Operator decisions" section
lists only Q-C01-1, Q-C01-2 and the race rows. `DECISIONS.md` marks the others "operator-decided", but no file I could
find records the question, its options or the answer, or says whether it was a live answer or the standing delegation of
2026-10-31. `plan/superseded-1/DECISIONS.md` still has D-C01-05 as an open NEW row. A contract amendment's authority
must be traceable from the round record, as Q-C01-1 is.

Fix: add Q1, Q2 and D-C01-05 to the round record's "Operator decisions" (or "Delegated decisions") section, each with
its question, options, answer, source and date. If any was a delegation rather than a live operator answer, say so in
Pointer amendment item 7 as well. Records only.

### V5-02-RA01-B-W2-C01-R4 (minor, records): the boundary-pin record still reads "pending" for Linux and overstates what the gate shows

`c02-pin.md` lines 19 and 122 still say "to be recorded by the gate stage" and "pending" for the C01 Linux rows. The
round record says the gate-3 container pass "shows equality with the pin". The test checks a window of plus or minus 8
(`external_registration.t.cpp:4595-4596`), so a pass shows the value is within 8 of 227301, not equal to it. I still
accept acceptance 5 for the pin. The pin depends only on `runtime.cpp`, `commands.lua`, `schema.lua` and `semantic.lua`,
and all four are byte-identical to the wave 2 reviewed candidate. The last edit to the production-embedded bytes was
therefore the wave 2 one, measured then at 227275 (macOS) and 227301 (Linux Debug). The round's boundary-pin rule asks
for a re-measure only if C01 changes those bytes. Fix: replace the two stale cells with that statement and the gate-3
window result. Note that Linux RelWithDebInfo has never been measured. Correct "shows equality".

### V5-02-RA01-B-W2-C01-R5 (minor, records): red-first identity is not the workflow identity output

C01-42 requires the pre-fix identity.py output to be pasted verbatim. `RED.md` instead gives sha256 values of
`candidate.diff` and `SHA256SUMS`, computed outside identity.py, and the record notes that `red/snapshot/` has no hash
of its own. I found the snapshot files byte-equal to the wave 2 reviewed bytes, so the red base is sound. The record
should still state the deviation as a limit, together with the missing red-first record for C01-46 (R2).

### V5-02-RA01-B-W2-C01-R6 (minor): small text inaccuracies

- `capture.cppm:96-97` says "any failure after the gate, including at `finish()`, is `capture_changed`". The post-read
  bound and the allocation remaps are `resource_limit` `diagnostic_bytes`.
- The comment above the 1 MiB prompt test (`authoring_prompts.t.cpp:2045-2047`) says the search "bisects". After
  GR-01 the code uses a secant search, as its inner comment says.
- `authoring_orchestrator.t.cpp:206` says the Lua check was "added in wave 2 of RA01-B-W2-C01". It was package I-B,
  wave 3 of the C01 plan.
- Carried I-R concerns 2 (`details={}` on the observe slot refusal, unreachable) and 8 (`docs/architecture.md:131-134`
  bootstrap sentence, older than this round) remain. Neither blocks.

These can be fixed together with R1 and R2.

## Evidence

All under `tests/native/v5-02-ra01-b-w2-c01-evidence/review/`:

- `identity-src.txt`, `identity-docs.txt`, `identity-orchestrator.txt`: identity.py output for `src`, for the three
  changed docs, and for `orchestrator.lua` (R8).
- `ctest-debug-review.log`: my full Debug run, 1437 of 1437, exit 0, paths replaced by `<repo>`, `<home>` and
  `<scratch>`.
- `inventory-check.txt`: the inventory comparisons above.
- `canonical-allocation-trace.txt`: the source lines behind R1.

Commands I ran: the identity.py calls above; `cmake --build build/rewrite-debug --target planar_tests` and the same on
`build/rewrite-release` (both no work); `ctest -N` on both trees with sorted comparisons; `ctest --test-dir
build/rewrite-debug --no-tests=error --output-on-failure -j4` (exit 0); a literal diff of `red/snapshot/` against the
final product files; greps of the record, plan, contract and source; and `python3 scripts/redact-local-paths.py check --repo-path <repo> --home <home>` on this review and every file in `review/` (exit 0). I applied no mutant and edited no file outside
`docs/vernacular/rounds/V5-02-RA01-B-W2-C01-review.md` and the `review/` folder above. I ran no script under
`tests/native/`.

## Limits

- I did not rebuild or run RelWithDebInfo or the Linux container. For those I rely on the run 3 raw logs, which I read.
- I re-ran no mutant. My probe verdicts come from the prober's raw logs and from reading the tests and the code. R1 is a
  source trace: no allocator fault can be injected (no seam may be added), so it cannot be shown by a run.
- The source-guard tests (C01-06, C01-13, C01-35 to C01-38, C01-44, C01-45) pin spellings, not behaviour, and they read
  the source tree at run time. The round states this as a limit, and I accept it.
- Fault-only survivors E01, E02, E02b, E03, E05 and E11 need a race or a syscall fault seam. I accept them as limits,
  along with X01 and X11 from wave 2.
- The root-aware tests record SUCCEED as root, so the Linux container run (root) does not exercise them. They ran for
  real on macOS as non-root, both in the gate and in the probes.
- I did not check the operator decisions against the operator's own messages (see R3).
