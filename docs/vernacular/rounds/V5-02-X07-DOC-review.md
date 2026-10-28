# V5-02-X07-DOC independent review (design addendum before X07)

Disposition: **request-changes (records only)**.

Most of the addendum is right, and it is well grounded. Four blocking findings remain. Each one is a gap or an
unsupported attribution in the addendum's text. Each can be fixed by editing `docs/vernacular/x07-addendum.md`, and for
R4 possibly the round record. No contract, design, source, test, script or CMake change is needed, and none of the
findings needs an operator question. If the fix for R1 chose to widen the exported `command_vm::invoke` seam rather than
keep it, that would be a new authority choice. The round would then halt under Q18 instead of being fixed in the
records.

What holds:

- **Outcome table (criterion 2).** It has exactly 44 envelope rows (E01 to E44) and 12 opaque rows (O01 to O12), in the
  same order and with the same case names as the
  [spike argv tables](../external-envelope-spike.md#exact-argv-and-results). I derived every row by hand from
  X-CLI steps 1 to 8, the exit paragraph, the X-REG text rule (contract `:242-243`) and the C workspace-name rule
  (`command-contract.md:289-300`). All 56 exit, class, accessed and clause cells agree with that derivation. Section 2.4
  calls out the verb-as-value change (E06: PRB, exit 2, profile not accessed, X-CLI step 3), and also E11 and E19. The
  KSA derivation for E02, E35 and O07 is supported. X-CAP `:172` keeps the existing family capture error for a missing
  supplied path, `:179-182` and `:195` require every supplied slot to be checked before external Lua, and today's
  family error for a missing ledger is `ledger_not_found`, exit 3 (`tests/acceptance/bootstrap_history.py:131`,
  `bootstrap_export.py:113`). The sibling `trusted/` fixture keeps E34 and E35 free of accidental overlap.
- **Diagnostic table (criterion 3).** It has 69 rows. I checked every literal message against the source. Each one
  exists byte for byte at the cited `file:line`, or for G23 to G30 and G68 in the X-LIMIT closed table (contract
  `:531-538`). The `failed()` exit mapping (`response.cpp:162-170`), the forced exit 4 for export_incomplete
  (`invocation.cpp:479-481`) and `detail::usage` (`response.cpp:175`) are cited correctly. No new code or message
  appears. The runtime string `Invalid extension invocation` is excluded from public output (A-04), as the X07 must-not
  list requires.
- **Halt checks.** The uncertain D7-2 init reading was taken to the operator and recorded as Q-INIT A, with the
  contract-literal outcome: an alias check only, with no containment refusal. That matches X-CAP `:169-174` and X-STORE
  `:647-650`. The D7-10 reading is confirmed with real citations (`src/cmd/planar/CMakeLists.txt:107-111` runs
  `tests/native/v5-02-c-support/cli.py`, and `:156-158` runs `tests/acceptance/bootstrap_load.py`).
- **Carried limits (criterion 5).** The CL-1 to CL-6 and L-* quotes are byte-identical substrings of the roadmap's
  Carried limits row. I checked this independently. No limit is repaired or reclassified. The CL-5 basis change is
  recorded as a fact and left to the coordinator.
- **Scope (criteria 6 and 7).** Only the two owned docs are modified, plus the untracked addendum and round record.
  `git diff HEAD -- src tests scripts cmake` is empty. The architecture diff is exactly the pointer, the one replaced
  sentence and one delivery-sequence row. The implementation diff is one pointer paragraph. I checked 149 links
  independently with 0 bad, and the redaction check exits 0.

## Exact candidate

I reviewed in the shared checkout on `dev/planar-v2-vernacular`. The candidate is uncommitted. Identity tool output for
pathspec `docs/vernacular`, run by me from the repository root (`tests/native/v5-02-x07-doc-evidence/review/identity-docs.txt`):

| identity | value |
|---|---|
| HEAD | `25704378472f3120baa6755c922894188fa94db5` |
| sha256(git diff --binary HEAD -- docs/vernacular), tracked only | `9f88be5b2ebfda87bf4e77bf8b0298cb220c6c606fa3c4fdc274c6989653d10f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular, evidence excluded | `ae9cb9b3d3a94b9a440d1dabee6c7cf876ca4e8ca9d8e7d11f3789fcd5553dac` |

| file | state | sha256 |
|---|---|---|
| `docs/vernacular/external-profile-implementation.md` | modified | `1a76f1dc700b70733a44f9bb00d915db3e9926855a999f64980f59a15e6ba5af` |
| `docs/vernacular/profile-preparation-architecture.md` | modified | `0cc2d7eebd2f5642d6e0945be65056d2f7acbd36cbb59fd417bf060d3f1f8c35` |
| `docs/vernacular/rounds/V5-02-X07-DOC.md` | untracked | `a12331c1c6abcb448bba3d69a1b558213374419288bca957ca93df3537b0bc6c` |
| `docs/vernacular/x07-addendum.md` | untracked | `5e87c2ad9a35248975e64f080086fd219a9fd9ca49131c23ff0237d18766d175` |

HEAD, the tracked-only hash and the three product-doc hashes match the round record's Final candidate section. The
record's combined hash (`ce1eff73...`) differs from mine. The record says it would: the record is part of the combined
input and changes with every edit of the record.

## Findings

### X07-DOC-R1 (required, records): the profile invoke seam is left open

Section 1.1 says init, history and export "need an invoke path on profile VMs" because `command_vm::invoke` refuses
profile owners (`runtime.cpp:4844-4845`). The addendum does not say what that path is. Two shapes are possible:

1. Lift the refusal in the exported, public `command_vm::invoke`.
2. Keep that refusal, and add a module-private invoke that only the closed factory reaches.

The two are different authority seams. `command_vm::create(profile_snapshot)` is also exported and public
(`runtime.cppm:225`). Under option 1, any in-tree caller could build a profile VM and invoke shipped callables on it.
That caller would skip the known-slot admission and receipt reverification the factory exists to enforce. The D7-12
source scan covers only `src/cmd/`.

The addendum leans both ways. D7-11 lists `X05 A3 profile owners refuse invocation before provider effects`
(`external_registration.t.cpp:301`, assertion at `:308`) as a test whose assertion the cutover may change, which only
happens under option 1. Section 1.7 says the factory must not expose `command_vm` and adds no exported constructor, but
says nothing about exported `invoke`. Meanwhile the architecture still says "Generic `invoke` explicitly refuses
profiles" (`profile-preparation-architecture.md:46`), and the `create(profile_snapshot)` doc comment says "Generic
command invocation refuses". The design's must-not list also says "No widening of the X06-C seam".

So this is an authority choice that an X07 coder would otherwise make alone, and acceptance 1 asks for zero unmarked
choices. Fix: add a decision row (and a sentence in 1.1 or 1.7) stating the following.

- Exported `command_vm::invoke` keeps refusing profile owners.
- The shipped-callable invoke for init, history and export is private to `planar.engine.runtime` and reachable only
  through the closed factory's owner.
- No exported signature changes.

Cite the architecture `:46`, the `runtime.cppm:221-225` doc comment, 1.7 and the X07 must-not list. Then remove the
`:308` test from the D7-11 candidates, or say why it stays. If the author instead wants option 1, that widens an
exported seam: it is a new authority choice and must halt for the operator.

### X07-DOC-R2 (required, records): the load arm's input conflicts with the architecture

The 1.1 sketch gives the factory `open_with_profile(std::string_view profile_selector, const command_options& family)`,
and 1.1 and 1.3 say its only inputs are the selector and the parsed family options. The load arm then "routes to
`open_known_load`". But `open_known_load` takes a codec2 `request_intent` plus a native ledger path
(`runtime.cppm:466-467`). The architecture is explicit: it "requires codec2 profile intent and extracts the profile
selector itself; no independent profile argument can disagree"
([constructor protocol](../profile-preparation-architecture.md#constructor-parse-and-consumer-protocol), `:205-208`).

Under X-CLI steps 4 and 5, the request_v2 intent must be framed once, before profile capture, for the early saved-body
lookup. The intent bound in the owner must be that same intent, and the final parse must equal it (clause 12). As
sketched, the load arm has a selector argument that is independent of the intent. It also does not say whether the
factory re-frames the intent or receives the step-5 intent. Both gaps are exactly what the architecture forbids or
leaves no room for.

Fix: state that the load arm receives the one request_v2 intent framed at step 4 and used at step 5, plus the native
ledger path. The selector comes only from that intent, with no separate selector argument for load and no second
framing. For example, give the sketch a load-arm signature or a variant input that carries the intent. Mark the row
`design` with the architecture citation.

### X07-DOC-R3 (required, records): the render admission row omits the declaration-file alias check

The 1.2 render row lists "Workspace declaration root and selected declaration file" as known slots. Its check cell names
root overlap for the declaration, source and base roots, the observation rules and the registry alias. It never says
that the selected declaration file is checked for selected-file aliases with the profile and its modules.

X-CAP requires that check ("Reject distinct selected files sharing device/inode, including profile versus module and
existing registry/observation/declaration files", contract `:154-155`). The architecture's known-load factory
metadata-admits the declaration file with `admission_file_role::declaration` for exactly this reason: "Disjoint
directory roots do not exclude a hard-link alias between a module and the selected declaration". The design's seed rows
also include "A hard link between the module and the workspace declaration refuses before registration".

The admission table is the X07 coder's specification. A render path admitted from this row could leave the hard-link gap
open, even though the architecture closed it for load. Fix: add the selected declaration file alias check, refused as
`unsafe_path` (G26) before registration, to the render check cell, citing X-CAP `:154-155` and the architecture's
known-load paragraph.

### X07-DOC-R4 (required, records): A-10 is marked `operator` without a recorded decision

Row A-10 (the fixture layout with a trusted sibling `trusted/`, cwd `work/` and selector `../trusted/p.json`) is marked
`operator` and cites "Operator instruction of 2026-10-28 (fixture fix)". Section 2.1 repeats this. No question number is
given, as acceptance 1 requires for operator rows. The round record records only one operator decision, Q-INIT A. Its
Repairs section calls this change "the fixture change F-FIXTURE" without operator attribution. The only source is the
plan-side `plan/DECISIONS.md`, which calls it an "operator-requested fix".

Operator decisions rank first in the authority order, so an `operator` mark must be traceable to a record. Fix one of
two ways:

- Record the operator instruction in the round record, as was done for Q-INIT, with an identifier, and cite that
  identifier in A-10 and 2.1.
- Re-mark A-10 as `design`. It is a test-fixture choice that follows from X-CAP `:169-171`, not a product choice.

### X07-DOC-R5 (minor, non-blocking): the D7-11 candidate list is imprecise

Four of the named tests fail at registration, not through invocation. They assert `Invalid external profile protocol` for
descriptor or namespace refusals: `X05 B2 A wrong namespace ...` at `:177`, `X05 A3 C01 malformed sort keys ...` at
`:461`, `X05 A3 C01 handler and option controls ...` at `:477`, and `X05 A3 C02 sealing ...` at `:543`. Neither clause 26
nor any profile invoke path changes them. The row labels them "clause 26 or the profile-invoke refusal".

The list is honestly marked as candidates for X07 search to confirm, but an inaccurate list invites assertion edits
under XD5 cover. Narrow it after R1 is settled.

### X07-DOC-R6 (minor, non-blocking): text defects

- The D7-2 Choice cell and the A-07 Choice cell repeat the Q-INIT sentence ("decided by Q-INIT A (2026-10-28): Operator
  Q-INIT A (2026-10-28): ...").
- The history row cites X-CAP `:174-175` for "history/init have no inferred workspace root". The text is at `:173-174`.
- G35's exit is "by code (failed())". D7-7 in the design says a semantic error maps to the typed E "with exit 3".
  Under `failed()` the two agree only while no semantic E carries `usage`, `io`, `internal` or a ledger I/O code. One
  sentence would remove the ambiguity.

## Invariant table, red and probes

The round record's invariant table equals `plan/INVARIANTS.md` revision 2 row for row (I1 to I35). The only difference
is backtick formatting in the test column. Every acceptance criterion maps to at least one row. The round closes no
earlier findings, so no closure rows are owed.

- **Red.** `red/RED.md` is a not-registered result (ctest exit 8, "No tests were found"), not a behavioural red. The
  record says so. The behavioural red is `check/red.log`: in Run A the addendum is absent and 34 checks FAIL. Run B
  uses a headings-only stub, and each content check fails on its own reason. `red.log` has no snapshot hash. The record
  reports that gap, and the starting tree is identified by HEAD with an empty `src tests/native` diff.
- **Probes.** None were run. The round was dispatched with probe false, and there is no `probes/PROBES.md`. Every
  mutant in the table is unapplied, so no "killed by" claim exists to verify. I did not apply mutants: the workflow
  reserves that for the prober. This is a recorded limit, not a finding.
- **What the checker can and cannot show.** The checker (`check/doc_check.py`) compares the addendum against
  `reference.json`, which comes from the same search output's Appendix A and B. A PASS therefore proves consistency with
  search's own reference, not correctness against the contracts. My manual derivation of all 56 outcome rows and the
  source check of all 69 diagnostic rows supply the correctness evidence. The record also notes that the checker changed
  after its author's report, from `a76c5e00...` to `0a3c6ba4...`, with no recorded reason. The final 35-PASS run uses the
  final file.

## Commands I ran (from the repository root, unpiped; logs under `tests/native/v5-02-x07-doc-evidence/review/`)

| Command | Exit | Result |
| --- | --- | --- |
| `python3 <workflow>/scripts/identity.py docs/vernacular` | 0 | table above (`identity-docs.txt`) |
| `python3 -I tests/native/v5-02-x07-doc-evidence/check/doc_check.py .` | 0 | 35 PASS, 0 FAIL, 0 skipped (`doc_check.log`) |
| `python3 -I review/links.py` over the addendum, the round record and the two edited docs | 0 | 149 relative links and anchors, 0 bad (`links.log`) |
| `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME"` on the same four files | 0 | clean (`redact-check.log`) |
| `git status --short`, `git diff --stat HEAD` | 0 | two modified owned docs, two untracked docs (`status.log`, `diffstat.log`) |
| `git diff --binary HEAD -- src tests scripts cmake \| wc -c` | 0 | 0 bytes (`nonDocs-diff-bytes.txt`) |

I also ran scripted checks without saved logs:

- Each G-row message against `src/**/*.cpp{,m}` and against its cited line.
- Each CL and L quote against `docs/vernacular/roadmap.md`.
- The cited `runtime.cppm` and `runtime.cpp` lines: 68, 225, 281, 409, 455-460, 466-471 and 496 in `runtime.cppm`;
  4016 and 4844-4845 in `runtime.cpp`.
- The D7-11 test names against `external_registration.t.cpp`.

## Limits

- This is a records-only review. No product build, ctest run or no-change build was performed, because none applies to a
  documentation-only candidate.
- Mutation probing was not performed (probe false). The 35 checks' power to discriminate is argued, not demonstrated.
- My outcome derivations depend on reading CLI11 behaviour from the spike. In two rows the class depends on CLI11 detail
  but the exit code and profile access do not: E19, where PFX and PRB would both be exit 2 with no profile access, and
  E09. X07's production matrix test is the real proof.
- The roadmap and the review file are coordinator-owned and follow approval. The roadmap is unchanged at review time,
  as planned.

## Records re-check (2026-10-28)

Disposition: **request-changes (records only)**.

The records fix closes R1 in substance, and it closes R3, R4 and R5. R2 is still open: the load arm's new wording
contradicts the sketch signature and two other sentences. The fix also introduced three new problems. The CL-5 relation
no longer records the reach change, G35 now attributes to D7-7 an exit that D7-7 does not state, and the round record
still claims 35 PASS. The checker now fails 3 of its 35 checks. Every remaining item can be fixed by editing the
addendum and the round record.

### Identity and scope

Identity tool output, run by me from the repository root (`tests/native/v5-02-x07-doc-evidence/review-recheck/identity-docs.txt`):

| identity | value |
|---|---|
| HEAD | `25704378472f3120baa6755c922894188fa94db5` |
| sha256(git diff --binary HEAD -- docs/vernacular), tracked only | `9f88be5b2ebfda87bf4e77bf8b0298cb220c6c606fa3c4fdc274c6989653d10f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular, evidence excluded | `1af88db28384384ff45a80d63c02bd296dd1448daf9ca9762bebd97484f53c5c` |

| file | state | sha256 |
|---|---|---|
| `docs/vernacular/external-profile-implementation.md` | modified | `1a76f1dc700b70733a44f9bb00d915db3e9926855a999f64980f59a15e6ba5af` |
| `docs/vernacular/profile-preparation-architecture.md` | modified | `0cc2d7eebd2f5642d6e0945be65056d2f7acbd36cbb59fd417bf060d3f1f8c35` |
| `docs/vernacular/rounds/V5-02-X07-DOC-review.md` | untracked | `68b9ab7e4c11ee9412765454ca7b9d1d1599bcb533c88837fb14306ca00ac293` |
| `docs/vernacular/rounds/V5-02-X07-DOC.md` | untracked | `0d3f7c60efac09abacdc4b404df03a3eab100e48587d7ff1ecd68dd863a13bf9` |
| `docs/vernacular/x07-addendum.md` | untracked | `39457ba4c8e0f04a5d74b0875cae7283440b60ba30a14ac5a741a894a2cf2e05` |

(The review-file hash above is this file before this section was appended.)

- HEAD and the tracked-only hash are unchanged since my review. The two modified product docs have the same per-file
  hashes as before. The addendum hash `39457ba4...` matches the value in the record's "Records fix after review" section.
- For pathspec `src tests scripts cmake CMakeLists.txt`, the identity tool prints the empty-input hash for both the
  tracked diff and the combined value, and lists no files (`identity-nondocs.txt`). `git diff --binary HEAD -- src
  tests scripts cmake` is 0 bytes. No source, test, script or build file changed.
- Evidence data is unchanged. `check/doc_check.py` still hashes to `0a3c6ba4...` and `check/reference.json` to
  `a78e9fc3...` (`checker-hashes.txt`). No file under `tests/native/v5-02-x07-doc-evidence/` is newer than my review,
  apart from my own review logs.
- Only the addendum and the round record changed, as the record states.

### Finding status

| Finding | Status | Basis |
| --- | --- | --- |
| R1 | Closed in substance. One residue, required (R1a below). | 1.1 now keeps the exported `command_vm::invoke` refusal and makes the profile invoke path module-private and reachable only through the factory. It cites architecture `:46` (verified: "Generic `invoke` explicitly refuses profiles") and `runtime.cppm:221-225` (verified: "Generic command invocation refuses" at `:224`). It records the other option as not chosen, and says choosing it would halt. 1.7 has a matching bullet. The `:308` test is out of D7-11. |
| R2 | **Open**, required. | See R2 below. |
| R3 | Closed. | The render row now has the selected declaration file alias check, refused as `unsafe_path` (G26) before registration. It cites X-CAP `:154-155` (verified: lines 154 to 155 are the device/inode sentence) and the architecture hard-link gap (verified at `profile-preparation-architecture.md:216-218`). G26 is `unsafe_path` / `External profile overlaps a supplied input`. |
| R4 | Closed in the addendum. It breaks a check (N2). | A-10 is `design` and cites X-CAP `:169-171`. 2.1 no longer claims an operator instruction. The citation says "made by the round author". The plan-side `plan/DECISIONS.md` row F-FIXTURE still says "operator-requested fix". The two are not in contradiction: no operator decision is recorded, and `design` is one of the two fixes I offered. |
| R5 | Closed. Minor residue. | :177, :308, :461, :477 and :543 are out. Residue: the row says each candidate carries "clause 24, 25 or 26", but the `:2763` candidate (in `X05 C2 B2 maximum section text ...`, which starts at `:2703`) carries no clause. Non-blocking. |
| R6 | Partly closed. One new defect (N3). | The history row now cites `:173-174` (verified). The D7-2 cell still repeats the sentence: "The init cell is decided by Q-INIT A (2026-10-28): Init is decided by Q-INIT A (2026-10-28), stated in full ...". The record's statement that the cell was deduplicated is therefore not accurate for D7-2. A-07 is fine. G35: see N3. |

### R2 (still open, required, records): the load arm cannot receive the intent the text says it receives

1.1 and the sketch comment now say the load arm "takes the single request_v2 intent framed at step 4". The sketch
signature is unchanged: `open_with_profile(std::string_view profile_selector, const command_options& family)`. Its
`command_load_options` member (`runtime.cppm:58-66`) holds the ledger, workspace, registry, expect_revision, request_id,
actor and expected_head fields. It has no request intent. So no input of the sketched factory carries the intent.

Two sentences also still contradict the new load bullet:

- The last bullet of 1.1: "The factory takes the admitted envelope selector and the parsed family options".
- 1.3: "its only inputs are the envelope-admitted selector and the parsed family options".

The load arm also still receives a `profile_selector` argument that the comment says it ignores. That is the
independent profile argument the architecture rules out (`:205-207`, "no independent profile argument can disagree").

An X07 coder reading this would have to choose a shape. Fix: change the sketch so the load arm's input carries the
step-4 `acceptance::request_intent`, with the ledger path from `command_load_options::ledger` or alongside it, and no
separate selector. Possible shapes are a load overload, or a variant input whose load arm holds the intent. Then amend
the last 1.1 bullet and 1.3 so they state the per-arm inputs. Citation: architecture `:205-208`. Status: `design`.

### R1a (required, records): the invoke-path choice has no section 3 row

The addendum's opening says every choice is marked and "The choices are listed in section 3". The module-private
invoke path is a choice, but section 3 has no row for it. My review asked for a decision row. Fix: add the choice to an
existing row's text (A-01 or D7-12), marked `design` with the architecture `:46` and `runtime.cppm:221-225` citations.
A new A-13 row would also work, but the checker's I15 may treat it as an unknown ID.

### N1 (new, required, records): the CL-5 relation drops the reach change

The fix rewrote the CL-5 relation to say that the roadmap basis "still holds for the exported `command_vm::invoke`". As
a sentence about the exported name, that is true. But the roadmap basis is "profile VMs refuse invoke", and after X07
profile VMs are invoked through the private path. Whether that path runs the command-service response code at
`runtime.cpp:4888-4891` on a profile VM is left open.

This conflicts with three things:

- CL-1 and CL-4 each get an "Added reach" statement.
- The plan-side F-CL5 finding says the basis no longer holds.
- The round's own invariant I19 requires the CL-5 entry to record that the basis "changes under Q1 B without
  reclassifying".

As written, the cell implies CL-5's basis is preserved, which could mislead the coordinator's reclassification call.
Fix: state the added reach without reclassifying. The exported refusal remains, but under Q1 B profile VMs gain a
private invoke path for init, history and export. If that path shares the response handling at `4888-4891`, that code
becomes reachable on profile VMs. Reclassification is for the coordinator.

### N2 (new, required, records): the record's 35 PASS claim is stale

I re-ran `python3 -I tests/native/v5-02-x07-doc-evidence/check/doc_check.py .` on the fixed tree. It exits 1 with 32
PASS and 3 FAIL (`review-recheck/doc_check.log`):

- `diagnostic_rows_use_closed_messages` (I9): the G35 exit cell no longer equals the reference `by code (failed())`.
- `decisions_rows_marked` (I15): the reference expects A-10 `operator`.
- `limits_not_reclassified` (I19): the CL-5 entry lacks "Q1 B" and the basis-change statement (N1).

The round record still says, in Test counts, Gate results and acceptance criterion 1, that the final tree passes all 35.
Its "Records fix after review" section does not say the checker was re-run.

Fix: re-run the checker and record the exact result. N1 and N3, fixed as described, restore I19 and I9. The I15 failure
for A-10 is a direct consequence of R4, because the reference predates the review. Record it as an expected reference
mismatch, and amend the I15 row text in the record's invariant table so it states A-10 `design`. Updating
`reference.json` would also be acceptable, but that is an evidence-data change and is not needed to close this finding.

### N3 (new, required, records): G35 cites D7-7 for an exit D7-7 does not give

G35's exit cell now reads "the exit of the mapped inner code per D7-7 and 2.5 (3 for `unsafe_path`, `resource_limit`
and similar; 2 for `usage`)". D7-7 in the design (`remaining-steps-design.md:261`) says
`profile_preparation_error.semantic` "maps to the existing typed E with exit 3". It does not give exit 2 for any code.
The edit therefore attributes a claim to D7-7 that D7-7 does not make, and the ambiguity R6 raised is still there.

Fix: restore the cell to `by code (failed())`, which is also what the reference expects. Add one sentence below the
table: under `failed()` (`response.cpp:162-170`), a semantic E maps to D7-7's exit 3 except when its code is `usage`
(2) or `io`, `internal` or a ledger I/O code (4). X07 search must confirm which codes a trusted semantic E can carry. If
a semantic `usage` E is possible, which of exit 2 or exit 3 applies is an open question for the operator, not a choice
for the coder.

### Commands I ran (from the repository root, unpiped; logs under `tests/native/v5-02-x07-doc-evidence/review-recheck/`)

| Command | Exit | Result |
| --- | --- | --- |
| `python3 <workflow>/scripts/identity.py docs/vernacular` | 0 | table above (`identity-docs.txt`) |
| `python3 <workflow>/scripts/identity.py src tests scripts cmake CMakeLists.txt` | 0 | empty-input hash, no files (`identity-nondocs.txt`) |
| `python3 -I tests/native/v5-02-x07-doc-evidence/check/doc_check.py .` | 1 | 32 PASS, 3 FAIL (`doc_check.log`) |
| `python3 -I tests/native/v5-02-x07-doc-evidence/review/links.py` over the addendum, the round record and the two edited docs | 0 | 150 links and anchors, 0 bad (`links.log`) |
| `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME"` on the same four files | 0 | clean (`redact-check.log`) |
| `git status --short`; `git diff --binary HEAD -- src tests scripts cmake \| wc -c` | 0 | two modified docs and three untracked docs; 0 bytes (`status.log`, `nonDocs-diff-bytes.txt`) |
| `shasum -a 256` on `check/doc_check.py` and `check/reference.json` | 0 | unchanged (`checker-hashes.txt`) |

I also checked the following by reading, without saving logs:

- The cited lines `external-profile-contract.md:154-155`, `:169-174`; `profile-preparation-architecture.md:46` and
  `:205-218`; `runtime.cppm:58-69`, `:219-225` and `:455-471`.
- The `:2703`/`:2763` test name in `external_registration.t.cpp`.
- D7-7 at `remaining-steps-design.md:261`.

I applied no mutants and edited no file other than this section and the new `review-recheck/` logs.

## Records re-check 2 (2026-10-28)

Disposition: **approve (records only)**.

Records fix 2 closes every open finding of the records re-check (R2, R1a, N1, N2, N3 and the R6 residue). I found no new
false claim. One non-blocking nit is listed below.

### Operator decision recorded in this step

The operator answered Q-G35 on 2026-10-28: option A, map by code. As instructed, I recorded it, and that is the only
content edit I made outside this section:

- `x07-addendum.md`: the opening now says no operator question is open and that Q-G35 was decided as A. The note under
  the 2.5 table states the `failed()` mapping (`response.cpp:162-170`: `usage` 2; `io`, `internal`, `ledger_corrupt`,
  `ledger_busy`, `storage_full` 4; else 3), cites `command-contract.md:147`, and says D7-7's literal "exit 3"
  (`remaining-steps-design.md:261`) is superseded on this point. Section 3 gains row A-13 (`operator`, Q-G35 A,
  2026-10-28). The section 3 preamble now says A-01 to A-13. The G35 cell (`by code (failed())`) is unchanged.
- `rounds/V5-02-X07-DOC.md`: acceptance 1 says Q-G35 is decided. The N3 row in Records fix 2 has a superseded note.
  The Q-G35 heading reads decided, a decision paragraph precedes the question as it was asked, and the closing
  sentence of the recommendation now records the answer.

I15 checks only missing or duplicate IDs and the status of reference IDs, so the extra A-13 row does not affect it. D7-7
stays `contract-decided`, as the reference expects. The checker result is the same before and after the edit (below).

### Finding status

| Finding | Status | Basis (bytes read) |
| --- | --- | --- |
| R2 | Closed. | The 1.1 sketch has two `open_with_profile` overloads. The load overload is `(const acceptance::request_intent&, std::string_view ledger_path)`, which is the exact parameter list of `open_known_load` (`runtime.cppm:466-467`). It has no selector, does no second framing and takes the ledger from `command_load_options::ledger` (`runtime.cppm:58-66`: verified, a `std::string ledger` and no intent field). The selector overload refuses a `command_load_options` alternative before capture. That matches architecture `:205-208` ("requires codec2 profile intent and extracts the profile selector itself; no independent profile argument can disagree"; ledger selector native-only). The text says the overload is an X07 plan, not a current type, and that no intent field is added. The last 1.1 bullet and 1.3 now state the inputs per arm. A-01 carries the load-arm input as `design`. |
| R1a | Closed. | A-01 (`design`) records the module-private invoke path for init, history and export. The path is reachable only through the closed factory, the exported `command_vm::invoke` refusal is kept, and no exported signature changes. A-01 cites architecture `:46` (verified: "Generic `invoke` explicitly refuses profiles") and `runtime.cppm:221-225` (verified: "Generic command invocation refuses" at `:224`). The refusal is at `runtime.cpp:4844-4845` (verified). |
| N1 | Closed. | The CL-5 relation says "Added reach": the basis changes under Q1 B and the exported refusal remains. If the private path shares `runtime.cpp:4888-4891`, that code becomes reachable on profile VMs. It does not reclassify and leaves reclassification to the coordinator. The roadmap quote cell is unchanged, and I19 passes. |
| N3 | Closed. Decided in this step. | The G35 cell is `by code (failed())`, as the reference expects, and I9 passes. Every citation in the note checks out: `semantic.lua:374` returns `internal` "Invalid trusted shape result"; `:384-400` passes any nonempty string code through; contract `:521` makes unexpected trusted Lua errors `internal` exit 4; `runtime.cpp:5415-5416` and `:5435-5436` pass the routed or prepared error through as `semantic`. The note now records the operator decision. |
| R6 residue | Closed. | D7-2 states the Q-INIT decision once. The `:2763` candidate is gone, with a reason that is correct: `:2703` opens `X05 C2 B2 maximum section text ...`, which ends at `:2718`, and `:2763` is the `Invalid extension invocation` message assertion inside the helper `extension_usage`. The row now says "clause 24 or 25". |
| N2 | Closed. | The record reports 34 PASS and 1 FAIL (I15, A-10). My re-run gives exactly that (below). The I15 row text in the record's invariant table is amended. |

Non-blocking nit (N4): the addendum's D7-11 and the record's R6-residue row give the `extension_usage` helper span as
`2758-2764`. The closing brace is at `2765`. The cited assertion at `2763` is inside the helper, so the reasoning is
unaffected.

I found no new false claim. I checked the R2 sketch against `runtime.cppm:58-69`, `:218-225` and `:455-500` and against
architecture `:200-220`. The load overload is consistent with the ledger path's native-only, alias-only role. The
private `open_admitted` helper is used only for the non-load arms. D7-7 in the design is quoted exactly.

### Identity

HEAD `25704378472f3120baa6755c922894188fa94db5`. The tracked diff for the two modified product docs hashes to
`9f88be5b...` (unchanged). Per-file sha256 values after my Q-G35 edits and before this section was appended are below
(`review-recheck2/hashes.txt`).

| file | sha256 |
| --- | --- |
| `docs/vernacular/x07-addendum.md` | `9852e1ec073d63616304bf9c89d7f014827ba2205ed70474f9ca00e6210be9cf` |
| `docs/vernacular/rounds/V5-02-X07-DOC.md` | `43eb2b36ecc0d11f2b0e9a194de8b24a72ec0d82bed94c421cf5e0f9ca45bd03` |
| `docs/vernacular/profile-preparation-architecture.md` | `0cc2d7ee...` (unchanged) |
| `docs/vernacular/external-profile-implementation.md` | `1a76f1dc...` (unchanged) |
| `check/doc_check.py`, `check/reference.json` | `0a3c6ba4...`, `a78e9fc3...` (unchanged) |

### Commands I ran (from the repository root, unpiped; logs under `tests/native/v5-02-x07-doc-evidence/review-recheck2/`)

| Command | Exit | Result |
| --- | --- | --- |
| `python3 -I tests/native/v5-02-x07-doc-evidence/check/doc_check.py .` (after the Q-G35 edits) | 1 | 35 checks: 34 PASS, 1 FAIL, nothing else. The FAIL is `decisions_rows_marked` (I15): `A-10 status 'design', want 'operator'`, the recorded reference mismatch. I did not run the checker before my edit. The result equals the 34/1 that Records fix 2 records for the tree before the edit, and every check has the same status as recorded there. So the edit affects no check (`doc_check.log`; re-run after this section was appended: same result, `doc_check-final.log`). |
| `python3 -I tests/native/v5-02-x07-doc-evidence/review/links.py` over the addendum, the round record, `profile-preparation-architecture.md` and `external-profile-implementation.md` | 0 | 153 links, 0 bad (`links.log`). Re-run with this review file added after this section was appended: exit 0, 155 links, 0 bad (`links-final.log`) |
| `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME"` over the same four files plus this review file | 0 | clean (`redact-check.log`) |
| `git diff --binary HEAD -- src tests scripts cmake` (bytes) and `git diff --stat HEAD -- src tests scripts cmake` | 0 | 0 bytes, empty stat; the evidence folder is gitignored (`git check-ignore`) |
| `git status --short` | 0 | three modified docs (the two owned docs plus the coordinator's roadmap) and three untracked docs (`status.log`) |

Limit found while checking: the redaction checker exits 0 for a path that does not exist. My first invocation passed all
the paths as one quoted argument, so it checked nothing, and I re-ran it with separate arguments. I confirmed that the
checker works by planting a `$HOME` path in a scratch file and in a temporary in-repo file (exit 1 both times; the
temporary file was removed). The link script failed loudly on the same mistake (exit 1), so that run was never counted.

No product build, ctest run or mutation probe applies. I committed nothing.
