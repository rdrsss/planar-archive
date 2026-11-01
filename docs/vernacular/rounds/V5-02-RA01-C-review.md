# V5-02-RA01-C independent review (RA01 integrated gate, XD6)

Disposition: **request-changes, records only**. The gate holds up. The two source edits are comment-only. Every changed
line in `src` is a `//` or `///` line, and `runtime.cpp`, `commands.lua`, `schema.lua` and `semantic.lua` are byte-identical
to HEAD. The inventory is the same 1437 names in macOS Debug, macOS RelWithDebInfo, Linux Debug and Linux RelWithDebInfo,
with no names added or removed against the committed afb927dc baseline. All four runs pass 1437 of 1437 with no skips,
and `x07.two_process_cas_one_winner` passes in all four. The no-work builds are clean, lint passes, and redaction is
clean on every committable file. The acceptance-row table covers every bullet of execution-design.md section 4, the
authoring-contract acceptance checklist (:570-590) and the matrix rows A01, A04, A07, A35, A36 plus the two unnumbered
deliverables. Every test name it cites exists in the inventory, and I found no invented coverage.

Two records defects need fixing before the commit, because the commit carries the acceptance-matrix A07 text and the
staged set. R1: finding RA01-C-F1 misdescribes the engine code it cites and leaves out half of the contract gap. R2: the
staged set and the owned-file list leave out `tests/native/path-redaction.json`. Both can be fixed in the round record
and evidence text alone. RA01-C-F1 is a real contract gap and stays open after this round. I recommend the product fix
(R3). RA01 must not be called complete until it is fixed or the contract is amended.

I reviewed the uncommitted candidate on HEAD `afb927dc` against the [round record](V5-02-RA01-C.md), `plan/INVARIANTS.md`
and `plan/DECISIONS.md` (revision 4), `REPORT.md`, `acceptance-rows.md`, `inventory.txt` and the raw logs under
`raw/gate2/` and `raw/gate2/linux/`. I also checked execution-design.md RA01 sections 4 and 5, the authoring contract
(Sections paragraph at :422 and acceptance checklist at :570-590) and acceptance-matrix.md :14-29 and :57-58. On the
product side I read `capture.cpp:67-141`, `invocation.cpp:120-125`, `orchestrator.lua:112-160`, `runtime.cpp:340-440`,
`directory.cpp:263-299` and the tests behind rows C-15g, C-15r and C-14.

## Exact candidate

identity.py (the workflow script) output, run from the repository root with pathspecs `src tests/native` and pasted
verbatim (also saved as `tests/native/v5-02-ra01-c-evidence/review/identity.txt`):

| identity | value |
|---|---|
| HEAD | `afb927dc72d6ff4b85a44985dd2cba565ae21352` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `1c52c3d07d47ea5e581aadc943da858d6e2a7c2abf91887480142258706b196d` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `1c52c3d07d47ea5e581aadc943da858d6e2a7c2abf91887480142258706b196d` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/authoring_orchestrator.t.cpp` | modified | `8e649c01784485b0cadd61e3b86ca57f3bd972257393a9b2aa83a536b1accd4c` |
| `src/engine/capture/capture.cppm` | modified | `4d05dab7b864adb943825e6c092847e1ce3a016153919c4b6e083274b0b11bd8` |
| `tests/native/path-redaction.json` | modified | `952c239ec19ca8b94d3baf8234099d27986d49d81ddbbe516fd9d1355c99c45d` |

This output matches `raw/gate2/identity.txt` and the round record. The src-only figure `git diff HEAD -- src | sha256` is
`0cfcd4f7227c01d4dea5ca041af165cb8fa2ddc5e7ff5b680246a08aaeed288c`, which matches the record. The run 1 identity
(`raw/identity.txt`) has the same two source-file hashes, so the cold build logged in run 1 (`raw/build-debug.log`,
`raw/build-release.log`, 759 compile and link lines each) compiled these exact bytes. Run 2's builds are no-ops on top of
it. Note that the raw timestamps cross the 2026-11-01 daylight-saving change. Run 2's `01:23` (EST) is later than run
1's `01:39` (EDT).

## The two comment-only edits

`git diff -U0 HEAD -- src` (saved with counts in `review/comment-only-check.txt`):

- `authoring_orchestrator.t.cpp`: old lines 208 and 209 ("... deleting a check is killed by" / "// the source") become one
  line, "... deleting a check is killed by the source". The words are kept exactly, line 208 is exactly 130 columns (the
  clang-format limit), and lines 205-207 and 210 are unchanged. The `future_names` and `catalog_names` lists
  (lines 199-202) are outside the hunk, and execution-design.md section 5 lists RA01-C as the gate only. So the future
  list is unchanged and the check is recorded (C-14). The dispatch said "remove"; the record keeps the words instead
  (D-C-02). That is the better reading, and it is recorded as an amendment.
- `capture.cppm:96`: "the post-read bound and digest or allocation failures (`bad_alloc`, ...)" now reads "the post-read
  bound and the mapped failures (`bad_alloc`, `Hash allocation failed`, `Canonical allocation failed`)". I checked this
  against `capture.cpp`. `bad_alloc` reaches `diagnostic_bytes` through the catch at :139 and through the detail-less
  `resource_limit` remaps in `post_gate_failure` (:78) and `seal_failure` (:84). The two digest messages reach it through
  `digest_failure` (:93-96), and every other digest error passes through. The new wording therefore names exactly the
  mapped failures, and the note's later sentence ("any other `finish` or digest result passes through unchanged") is
  still correct. `invocation.cpp:120-125` maps the same two digest messages for observation entries, but that path is
  documented elsewhere and is not covered by this note.

My scripted check counts zero changed lines in `src` whose first non-blank token is not `//`. This closes the round's
stated limit that no comment-stripped proof was saved (D-C-04). No source guard or script reads either file's text. A
grep over `src`, `scripts` and CMake finds them only in the CMake registration lines.

## Gate evidence, verified from raw logs

| check | what I read | result |
|---|---|---|
| lint | `raw/gate2/lint.exit`, `lint.log` | exit 0, "Formatting and Doxygen checked 171 active source files" |
| macOS Debug | `raw/gate2/ctest-debug.log`, `.exit` | "100% tests passed out of 1437", 1437 `Passed` lines, no Not Run, Failed or Skipped lines, x07 #1428 passed |
| macOS RelWithDebInfo | `raw/gate2/ctest-release.log`, `.exit` | same: 1437 of 1437, no skips, x07 passed |
| `--no-tests=error` | `raw/gate2/run.sh` | present on both macOS ctest commands; present in `scripts/linux-gate.Dockerfile:101` |
| no-work builds | `raw/gate2/nochange{1,2}-{debug,release}.log` | all four `ninja: no work to do.`; my own no-work run in Debug also did no work |
| inventories | `raw/gate2/ctestN-*.log`, `inventory.txt`, Linux `*-inventory-sorted.txt` | 1437 names each; Debug, Release, both Linux lists and `inventory.txt` are equal as sorted sets; `comm -3` against `git show afb927dc:tests/native/v5-02-ra01-b-w2-c02-evidence/inventory.txt` is empty (0 new, 0 removed) |
| Linux Debug and RelWithDebInfo | `raw/gate2/linux/container/*.exit`, `*-ctest.log`, `*-nowork.log` | all ten container exits 0; 1437 of 1437 each with no skips; x07 passed (0.98 s, 0.45 s); both no-work logs `ninja: no work to do.` |
| Linux candidate | `host-manifest.txt`, `manifest-compare.txt` | the manifest has the three candidate hashes above; host before and after equal; host and container context equal |
| Docker hygiene | `df-before.txt`, `df-after.txt`, `builder-prune.exit` | about 67.6 GiB free before (floor 15), about 56.8 GiB after, prune exit 0 |
| redaction | my own run of `scripts/redact-local-paths.py --repo-path <repo> --home <home> check` | exit 0 on all eleven files under the evidence directory outside `raw/`, the round record, both source files and the manifest; a grep for `/Users/` over the same set is empty |

The only "skipped" text in any ctest log is the name of test #717, which passed. My own focused run (`ctest
--no-tests=error -R "RA01-B2 orchestrator|RA01-B2 diagnostic and from|capture_diagnostic|RA01-B2 C01 slot type"`)
passed 21 of 21 (`review/focused-ctest.txt`). The reviewer-side counts are in `review/inventory-check.txt`.

## Acceptance-row table

`acceptance-rows.md` has 41 rows, the same as the C-15 rows in `plan/INVARIANTS.md`. I extracted every backticked string
and checked each one against `inventory.txt` with and without the `replacement.` prefix. Every string that names a test
matches exactly. The strings that do not match are messages, paths, fields and code identifiers. Against the sources:

- Execution-design.md section 4: A35 (C-15a), A36 (C-15b, C-15p), schema drift guard (C-15c), the ten observe items
  (C-15d, e, f, f2, g, h), orchestrator labels and stub flip (C-15i), absent job file (C-15j), invalid spec (C-15k), and
  the hard-to-observe bullet (C-15q). All are present.
- Authoring-contract checklist (:570-590): adds the raw DEL and C1 bytes (C-15s), the indexed excerpt (C-15t), the three
  critique rules and the fixed line (C-15u to C-15x), dot names (C-15y), root help, eleven rows and `with-profile`
  (C-15l1 to l4), reserved names and broken-profile order (C-15z1 to z5), and RA-8 per command (C-15o1 to o6). All are
  present.
- Acceptance matrix: A35, A36, the two unnumbered deliverables (C-15m) and the preserve rows A01, A04, A07 (C-15n1 to n3,
  C-15r). All are present.

I spot-checked the reasoning behind four rows. C-15c: `tests/native/v5-02-ra01-b-w2-evidence/probes/PROBES.md:116` is
W2-18, killed by the cited test. C-15g and F5: the hook in `directory.cpp:273` fires between two full scans, so a change
inside the first scan between listing and reading is not exercised, and F5 is accurate. C-15r: I read
`authoring_orchestrator.t.cpp:416-460` and it has no `mandatory` assertion. C-15i: the variants test lives in
`authoring_orchestrator_variants.t.cpp`. The eight findings are correctly classified: F1 is a contract gap and F2 to F8
are coverage or proof-strength gaps. Rows marked "no finding" do not hide an unobserved clause that I could find.

## Invariant table

The record copies all 61 rows of `plan/INVARIANTS.md` (revision 4); I counted 61 in each. `expectRed` is false on every
row, and no `red/RED.md` or `probes/PROBES.md` exists. That is right for a comment-only round with no new test and no
mutant dispatched, and the record says so. The described mutants would each break their invariant. For example, C-01's
"delete without re-joining" fails its grep and diff checks, C-03's non-comment token fails my zero-count check, and C-12
and C-13's "exit-only no-work" is guarded by the `no work to do` text requirement. Rows C-16, C-17 and C-20 (review,
commit, roadmap and matrix) are correctly marked not met at this sync.

## Findings

### RA01-C-R1 (blocking, records): F1 misstates the engine reader and leaves out the `version` wrapper

`acceptance-rows.md` row C-15r and finding F1 say that "`runtime.cpp:411-414` reads `mandatory` only when the key is
present (default false)". `REPORT.md` and the round record copy that text. It is wrong. The projection reader in
`runtime.cpp` requires the top-level keys `version` and `sections` (`fields != 3` refuses, :366). It also requires `id`,
`text` and `mandatory` in every section (`fields != 7` refuses, :420). A missing `mandatory` is a refusal, not a false
default. The orchestrator command never passes its sections through that reader. `orchestrator_command_impl` joins
`s.text` directly (`orchestrator.lua:155-157`). F1 also leaves out the other half of the same contract sentence:
authoring-contract.md:422 says "The projection returns `{version:1,sections}`". `project(c)` (`orchestrator.lua:112-140`)
returns a bare section array with neither `version` nor `mandatory`.

This matters because the F1 text is what goes into the A07 owner text (D-C-17, row C-20) and into the corrective brief.
As written, it suggests the gap is one missing key that the engine quietly defaults. In fact the orchestrator projection
is not in the contract's projection shape at all.

Fix (records only): correct the F1 description in `acceptance-rows.md`, in the carried copy in `REPORT.md` (append a
dated correction note rather than rewriting the gate runner's text), and in the round record's findings table. The
correction states (a) the reader's real behaviour and that the orchestrator path bypasses it, and (b) that both
`version:1` and `mandatory:true` are missing from the returned projection. The A07 matrix text written at close must use
the corrected description.

### RA01-C-R2 (blocking, records): `tests/native/path-redaction.json` is missing from the owned set and the staged set

The redaction repair added four manifest entries to `tests/native/path-redaction.json`. The round record's file list
includes it, but the dispatch owned-file list does not, and the Amendments section does not record the edit outside the
owned set. D-C-15's staged set ("the two src files, the round record, roadmap.md, acceptance-matrix.md and the
committable evidence files") also leaves it out. If the commit follows D-C-15, it would commit the four redacted
`DECISIONS.md` files without the manifest entries that record their original and redacted hashes. f78f8eb4 committed the
manifest alongside its evidence.

Fix (records only): add an Amendments bullet to the round record. It says the manifest edit was needed by the redaction
convention and that the manifest is staged, and it adds this review file and `review/` to the staged set. Do not edit
`plan/DECISIONS.md` to change D-C-15. Its redacted sha256 is pinned in the manifest (`29048c07...`), so an edit would
need a matching manifest update. The round record governs where the two differ, as its own status section says.

### RA01-C-R3 (carried, not a blocker for this round's commit): RA01-C-F1 disposition

F1 is a real conformance gap against an approved exact contract. It does not change any prompt bytes today, because the
orchestrator prompt has no budget. Contracts are authority level 2 under the repository instructions and stay binding
until a scoped, reviewed amendment, so no product choice is needed. My recommendation is option (a) of F1, as a corrective
round. `project(c)` returns `{version=1,sections=...}` with `mandatory=true` on all five sections, and a test observes
both. Option (b), an amendment, would be the deviation.

The corrective brief must settle how the flags are observed without adding product fields only for testing (standing
lesson 7). One path is a Lua-level test through the existing runtime projection reader. Another is a source guard of the
kind this lineage already uses. Until the gap is fixed or amended, the roadmap follows D-C-16(a): "RA01 gate green,
findings open", naming F1 with its corrective. Next work does not read "R15-A1 and R15-A2 docs". This overrides the
dispatch's "RA01 complete" wording, and the record already records that amendment.

### RA01-C-R4 (minor, records): `acceptance-rows.md` still reads `pending gate` in every Result cell

That file is staged as committed evidence. Add a one-line header note that the gate result for every row is in the round
record's invariant table and `REPORT.md` (gate run 2: four runs, 1437 of 1437 each). The note keeps a reader from taking
the rows as unverified. This is optional before the commit.

### RA01-C-R5 (minor, closed by this review): comment-stripped proof

The record's Limits section says the scripted proof (D-C-04) was not saved. `review/comment-only-check.txt` now holds
`git diff -U0`, the zero count of non-comment changed lines and the empty diff for the four pinned files. The coordinator
may cite it and drop the limit.

## Not findings

- Linux runs one no-work build per preset in a single container build with one prune. The record states this as an
  amendment of the "two per configuration" wording, and the script's design dictates it.
- `REPORT.md` notes a difference of 64 against 70 in the runner's count of name strings. My extraction found every
  test-shaped string present, so no cited name is missing.
- The C-15q, F2 claim that RA01 capture does not reach the admission receipt code agrees with `capture.cpp:103-141`,
  which uses `add_input`, `read` and `finish` without a receipt.

## Limits

- I did not re-run the full suites. I relied on the raw logs, my own no-work build and a focused 21-test Debug run. No
  mutation probing was run in this round, and I applied no mutant.
- I read the bodies of the tests for C-15g, C-15r and C-14 in full. For the other rows I checked that each name exists
  and spot-checked the cited lines, without re-deriving every body.
- Linux evidence is arm64 only; x86_64 was not run.

## Review evidence

New files under `tests/native/v5-02-ra01-c-evidence/review/`, all redaction-clean (check exit 0): `identity.txt`,
`comment-only-check.txt`, `inventory-check.txt`, `focused-ctest.txt`. Commands I ran: the workflow identity.py over `src
tests/native`; `git diff -U0 HEAD -- src` with the comment-line count; `git diff --quiet HEAD --` over the four pinned
files (exit 0); the inventory comparisons above; `ninja -C build/rewrite-debug planar_tests` (no work); the focused
ctest run (exit 0, 21 of 21); `scripts/redact-local-paths.py ... check` on the explicit file list (exit 0). I edited
nothing outside this review file and `review/`.

## Records re-check (2026-11-01)

Disposition: **approve**. Both blocking findings are closed by the records fix. No source, test or evidence-data byte
changed since my review, and the corrected text agrees with the product code. Three minor records advisories remain;
none blocks the commit.

### Candidate unchanged

I re-ran the workflow identity.py from the repository root with pathspecs `src tests/native`. The output is identical
to the identity recorded above and in `review/identity.txt`: HEAD `afb927dc72d6ff4b85a44985dd2cba565ae21352`, combined
sha256 `1c52c3d07d47ea5e581aadc943da858d6e2a7c2abf91887480142258706b196d`, and the same three per-file hashes
(`8e649c01...`, `4d05dab7...`, `952c239e...`). `git diff HEAD -- src | shasum -a 256` is still `0cfcd4f7...`. The
evidence data files have the same sha256 as before: `inventory.txt` `7b1a82d3...`, `plan/INVARIANTS.md` `d9906f63...`,
`plan/DECISIONS.md` `29048c07...` (the value pinned in the manifest). `find raw plan inventory.txt -newer
review/identity.txt` is empty, so no raw log or plan file was touched. The only evidence files modified after my review
are `acceptance-rows.md` (now `4e440044...`) and `REPORT.md` (now `6a05c3c7...`), as the fix section says. The four
`review/` files are unchanged, and `roadmap.md`, `acceptance-matrix.md` and `authoring-contract.md` are unchanged against
HEAD. Redaction `check` on the round record, `acceptance-rows.md` and `REPORT.md` exits 0.

### R1: closed

- `acceptance-rows.md` row C-15r and finding F1 now say what I asked for. The reader refuses a missing `version` or
  `sections` (`fields != 3`, runtime.cpp:366) and a section missing `id`, `text` or `mandatory` (`fields != 7`, :420),
  and it does not default the flag. `orchestrator_command_impl` never calls it and joins `s.text` directly
  (orchestrator.lua:155-157). `project()` (:112-140) returns a bare array without `version:1`. I re-read
  `runtime.cpp:355-430`, `orchestrator.lua:108-160` and `authoring-contract.md:422` against each clause. All of them
  hold, including `:114` building `{id=,text=}` and `:142-158` as the command body. Both cells carry a "Corrected after
  review R1" marker.
- The A07 owner text in `acceptance-rows.md:90` now names both missing parts (`version:1` and `mandatory:true`) and cites
  `orchestrator.lua:112-140`. The round record's R1 row points the matrix edit at that text.
- `REPORT.md` keeps the gate runner's text and adds a dated "Note after review (2026-11-01)" with the correction, as I
  asked.
- The round record's findings table row RA01-C-F1 is corrected and agrees with `acceptance-rows.md`.
- A grep for `411`, `default false` and `only when the key` over the round record and the evidence markdown finds the old
  claim only in `REPORT.md` table rows 84 and 104 (superseded by the dated note) and in `plan/DECISIONS.md` D-C-21's
  "near :411". That is a planner's pointer, not a claim about behaviour, and it must stay byte-identical because of its
  pinned hash.

### R2: closed

The Amendments section now has a "Review R2" bullet. It records that the manifest edit fell outside the dispatch owned
set and D-C-15, stages `tests/native/path-redaction.json` with this review file and `review/`, and leaves
`plan/DECISIONS.md` unedited because of its pinned hash.

### R4 and R5

R4 is closed by the header note in `acceptance-rows.md`. R5 is closed: the Limits bullet is marked closed and cites
`review/comment-only-check.txt`.

### Advisories (minor, records, non-blocking)

1. `REPORT.md`'s "Inputs for the acceptance-matrix edit" still carries the old A07 owner text (line 124: "sections not
   declared mandatory:true ... orchestrator.lua:114"). The dated note says only that the corrected text is in
   `acceptance-rows.md` for row C-15r and F1. At close, the coordinator must take the A07 text from
   `acceptance-rows.md:90`, as the round record's R1 row says, and not from `REPORT.md`.
2. The dated note in `REPORT.md` says "the limit above about a missing scripted proof is closed". `REPORT.md` has no
   such limit; the limit was in the round record. The reference is wrong, but it makes no claim about the product.
3. Some cells in the round record are now stale. The first row of the Acceptance table still says "stripped-compare
   script output not saved (Limits)". The Round file list still says "none outside the owned set was changed" and does
   not list this review file or `review/`, although the R2 amendment now records both. The per-file sha256 values for
   `acceptance-rows.md` and `REPORT.md` are labelled "at this sync" and no longer match. The coordinator can tidy these
   at close; the amendment and fix sections govern.

RA01-C-R3 stays carried as before. RA01-C-F1 is open, and the roadmap follows D-C-16(a).

### Commands run in this re-check

The workflow identity.py over `src tests/native`; `git diff HEAD -- src | shasum -a 256`; `shasum -a 256` over the
evidence files listed above; `find ... -newer review/identity.txt`; `git diff --quiet HEAD --` over `roadmap.md`,
`acceptance-matrix.md` and `authoring-contract.md` (exit 0); the grep above; `scripts/redact-local-paths.py
--repo-path <repo> --home <home> check` on the three fixed files (exit 0); reads of the cited product lines. I edited only
this section. I created no new evidence file and applied no mutant.
