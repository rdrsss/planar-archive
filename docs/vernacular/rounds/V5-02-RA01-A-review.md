# V5-02-RA01-A independent review (authoring contract)

Disposition: **request-changes (records only)**.

The candidate is well scoped, and most of it is right. It touches only owned files and changes no source, test, build
or Lua file. The CLAUDE.md edit is one authority-list line, and every link resolves. The root help block is a pure
insertion before `planar --help\n`. The six handler IDs fit the handler grammar. Two problems remain. First, several
clauses are not yet exact: a test author would still have to make a design choice (criterion 1), and in two places
the text contradicts the code or contracts it relies on. Second, the record leaves the plan-versus-candidate
differences and all 58 invariant rows undispositioned.

Every blocking finding below can be fixed by editing documents. That means `authoring-contract.md`, the pointer
paragraphs, the round record and, for R4, possibly a pointer sentence in `x07-addendum.md`, which would then be added
to the owned-file list. No source, test, evidence data or build change is needed.

None of the findings needs an operator question if it is fixed the way suggested. R8 has one exception. If the
author wants to keep new message strings instead of reusing existing ones, the E16 scope needs operator confirmation
under Q18.

## What holds

- **Scope and identity (criteria 4, 5 and 7).** `git diff HEAD --stat` shows 7 tracked files, 29 insertions and 4
  deletions. The untracked files are `authoring-contract.md` and the round record. `git diff HEAD -- src tests
  scripts cmake CMakeLists.txt` is empty.
- **CLAUDE.md and AGENTS.md.** The CLAUDE.md diff is exactly one line in authority item 2, with no status text.
  `AGENTS.md` is still a symlink to `CLAUDE.md`.
- **Deleted lines.** The four deleted lines are the A35 and A36 evidence cells in `acceptance-matrix.md` and two
  `v5-alignment.md` rows. Each is replaced by the same text plus a pointer, so nothing is narrowed.
- **Diagnostics.** No existing diagnostic message or X-LIMIT row is edited.
- **Redaction.** The redaction `check` on the nine explicit files exits 0, read unpiped. A probe file containing a
  home path exits 1, so the check does detect paths.
- **Identity.** HEAD, the tracked-only hash `08f488e5...f937a`, the combined hash `0b0de25f...cebf` and all eight
  per-file hashes match the record's Final candidate section. I re-ran the workflow `identity.py` myself.
- **Citations (criterion 6).** I read every HEAD line the record cites, and each says what the record claims:
  - `command-contract.md:94-96`, `:180-182` and `:37-60`;
  - `external-profile-contract.md:133-134`, `:270-273` and `:366-373`;
  - `load-command-contract.md:28-33`.

  My resolver checked 9 files, including anchors, and found 0 bad links (`review/links.log`).
- **Catalog and help (RA-1).** The catalog table, capabilities and response kinds match design RA-1. The underscore
  handler IDs match `command-contract.md` grammar `[a-z][a-z0-9_]*(\.[a-z][a-z0-9_]*)+`, and the deviation is tagged
  NEW. The option table matches the E1 rows (`execution-design.md:398-403`). The root help placement matches
  `src/cmd/planar/bootstrap.cpp:331-339`.
- **Pointer amendments (criterion 3).** The pointers in the four contracts agree with the authoring contract on:
  - eleven rows at the RA01 integrated gate;
  - root help gaining six lines;
  - the `diagnostic` and `from` slots;
  - the shipped-owned `orchestrator` projection ID;
  - the five reserved `orchestrator_*` section IDs.

  The exceptions are R4 and R9.
- **Unchanged commands.** RA-8 (init unchanged, Q10 B) is right, and so is the statement that render stays
  database-free (Q14 A).
- **Future-label list.** It is exactly the twelve non-RA01 verbs of the E1 table.
- **Acceptance checklist.** It covers every bullet of design section 4.
- **Prompt envelope (RA-3).** The `inputs` file entry matches the render file entry without `bytes_hex`
  (`render-contract.md:311`).

## Exact candidate

I reviewed in the shared checkout on `dev/planar-v2-vernacular`, uncommitted on HEAD
`a3364b8afae5ec7a75f2b19474df3fc89de9b078`. Output of the workflow `identity.py` over the eight product documents,
run by me from the repository root:

| identity | value |
|---|---|
| HEAD | `a3364b8afae5ec7a75f2b19474df3fc89de9b078` |
| sha256(git diff --binary HEAD -- CLAUDE.md docs/vernacular/acceptance-matrix.md docs/vernacular/command-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md docs/vernacular/v5-alignment.md docs/vernacular/authoring-contract.md), tracked only | `08f488e5dc385cc98e1949cf0a85510f4f8965c0d30801d25fdc4eaeda0f937a` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec CLAUDE.md docs/vernacular/acceptance-matrix.md docs/vernacular/command-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md docs/vernacular/v5-alignment.md docs/vernacular/authoring-contract.md, evidence excluded | `0b0de25f2c844360079e7e1bf83429866f1cf70048e4cacfa395e7825e81cebf` |

| file | state | sha256 |
|---|---|---|
| `CLAUDE.md` | modified | `f9cce3976da1d6f1fb5b8aa873a7b00ca755b189a5b2f47bede5b1f8665d195e` |
| `docs/vernacular/acceptance-matrix.md` | modified | `18c850dcbf3c871f0960760c784ebc0df25ee081208997b60dfb21dabf714965` |
| `docs/vernacular/command-contract.md` | modified | `ffdb020a3a30810185674fc4dd3fe56a185971864b458050d3447640a62a0ede` |
| `docs/vernacular/external-profile-contract.md` | modified | `1377d5deeebb2a13a3e377eb31f3ed467d5fc071f8d5aa8487f51f39befcb35d` |
| `docs/vernacular/load-command-contract.md` | modified | `d72870d6446aee8e5396c05f62ac68b16850a52974a2cf837bbec129487a30d3` |
| `docs/vernacular/render-contract.md` | modified | `5695452c7b19aa88e1e1d822ae61f61f325ff7a777b6fdd067a052bba71d629b` |
| `docs/vernacular/v5-alignment.md` | modified | `837688c03ff19e7cf2a3a356ce0ef0564b3f4bc03665240819d30b4c9a634543` |
| `docs/vernacular/authoring-contract.md` | untracked | `283510a3195f8904b6be2ad335f4245331fdc53ff28893082cc7eb356d7b9e53` |

The same run with the round record added is saved in `review/identity-candidate.txt`. In that run the round record
hashes to `156a185a...dfef1f`.

## Findings

All of R1 to R12 are blocking unless marked otherwise.

### V5-02-RA01-A-R1 (high): RA-5 field-path resolution never resolves a real load field path

- **Problem.** RA-5 resolves a path "in a schema when each segment names a field of the current record". It tries
  the `workspace`, `spec` and `job` schemas in turn.
- **Why it fails.** Every field path that the validator emits starts with the schema name:
  - `ctx.normalize(..., "workspace")`, `"spec"` and `"job"` at `src/stdlib/planar/schema.lua:252`, `:254` and `:269`;
  - for example, `workspace.root.id` at `schema.lua:171`.

  Take a load stderr field such as `job.roles[1].work`. Its first segment `job` is not a field of the job record. So
  the rule as written never yields an excerpt, and every A36 prompt would carry `No schema excerpt applies to this
  field.`
- **Consequence.** That contradicts design RA-5 ("the schema excerpt for that field path"). A test author must choose
  whether the first segment selects the schema.
- **Fix.** State that the first segment names the schema (`workspace`, `spec` or `job`; `registry` and
  `observations` give no excerpt). State that the remaining segments descend from that record.

### V5-02-RA01-A-R2 (high): RA-5 claims byte-equality while RA-4 says the raw-error block is escaped

- **The conflict.** RA-4 says data blocks, "raw error" included, use the presentation helper's labelled-data
  rendering "so control characters are escaped". RA-5 then embeds the file "verbatim ... as a labelled data block, so
  that the embedded raw error is byte-equal to load's stderr".
- **Why both cannot hold.** `presentation_data` (`src/stdlib/planar/presentation.lua:11-36`) rewrites these bytes as
  `\u00xx`:
  - DEL (0x7F);
  - C1 controls (UTF-8 `C2 80` to `C2 9F`);
  - any byte below 0x20 other than TAB, LF and CR.

  When it escapes, it also changes the label. Canonical JSON escapes only U+0000 to U+001F
  (`render-contract.md` "Canonical JSON transport"), so a canonical diagnostic can contain raw DEL or C1 bytes. For
  such a file, A36 byte-equality and the RA-4 escaping rule cannot both hold.
- **Fix.** Choose one rule and state it exactly:
  - a verbatim fenced block with no escaping (fence length rule cited from `presentation.lua`), with equality defined
    as block bytes equal to file bytes; or
  - escaping, with A36 equality redefined over the unescaped content and the limit recorded.

  Tag the choice in the decisions table.

### V5-02-RA01-A-R3 (high): critique rule `contract_consumer_unavailable` can never fire

- **What the contract says.** RA-2 produces a prompt only for inputs that pass validation, and critique runs the
  spec validation.
- **Why the rule cannot fire.** The current spec validation already refuses a contract consumer without a
  `start/contract/available` dependency, as `schema`:
  - `src/stdlib/planar/semantic.lua:245-251` ("Consumer requires a start/contract/available dependency");
  - `declarations.md:208-210`.

  RA-4 rule 6's second rule is exactly that predicate, so on any spec that reaches critique it yields no finding.
- **Consequence.** A dead rule cannot be tested or mutation-probed. It is the same defect that halted the wave-1 coder
  (record, "Round history"). The design's three examples are "for example" (RA-4), so the contract is free to choose.
- **Fix.** Replace the rule with one that a valid spec can trigger, give it a trigger recipe, and record why the hard
  validation examples were not used.
- **The other two rules are reachable.** `facet_dependency_without_output_vector` can fire because critique
  validates no job; the matching hard check sits in the per-job loop at `semantic.lua:306`.
  `prescription_unchecked` can fire when registry `selection` is `not_applicable`.

### V5-02-RA01-A-R4 (high): RA-9 and the with-profile mechanics are undecided and self-contradictory

RA-9 says three things:

1. `with-profile ... COMMAND`, with COMMAND one of the six verbs, stays `usage` "with the diagnostic the closed
   factory already gives".
2. "The effective catalog under `with-profile` is unchanged by RA01."
3. Nothing about which bundle the six rows belong to.

The approved texts do not settle the outcome. X-REG rejects "shipped command/family/handler ID reuse"
(`external-profile-contract.md`, X-REG). X-CLI step 3 builds the embedded probe from "all shipped families", and a
known shipped signature failure "returns usage immediately, never falls back to extension". There are two readings.

- **The six verbs are shipped under with-profile.** The effective catalog changes (contradicting point 2). The probe
  parses `spec-prompt` as shipped. A malformed `with-profile P observe` is then usage before profile capture, and
  workspace-name rules run at step 4.
- **The six verbs are not shipped under with-profile.** An external profile may register a command named `observe`.
  `with-profile P observe` then runs that command, contradicting point 1. An unknown-command usage also comes only
  after profile admission, so a broken profile takes precedence.

`x07-addendum.md` section 1.3 also says the `command_options` variant "is closed at five". It says a sixth member
needs an architecture revision, an admission row and a reviewed amendment. The contract does not say whether the six
families join that variant or form a separate embedded-only set, or which round owns the revision. RA01-B wave 1
would have to decide all of this.

The capacity and namespace consequences are not recorded either:

- X-LIMIT counts 64 commands and 64 projections "including shipped" (`external-profile-contract.md:512`).
- The five new reserved section IDs and the shipped-owned `orchestrator` refuse external catalogs that are valid
  today.

**Fix.** State the probe membership, the parse and admission precedence, the collision rule for the six names and
the `orchestrator_*` IDs under with-profile, and the family/variant decision with its owner. Record the capacity and
namespace consequences as decided by E1 and Q12 A. If `x07-addendum.md` gets a pointer, add it to the owned files.

### V5-02-RA01-A-R5 (medium): observe bounds contradict EX-8, and the decisions table gives a false basis

- **The design's numbers.** EX-8 (`execution-design.md`, "Observe directory (RA01)") gives ≤ 1024 entries, each file
  ≤ 64 KiB, total ≤ 4 MiB, and output ≤ 1 MiB.
- **The candidate's numbers.** RA-6 uses 4096 names per directory, 1024 files each in `checks` and `findings`, and
  1 MiB total read. It reuses `observation_bytes` for both the total-read site and the output site.
- **The false basis.** The decisions table's basis says "RA-6 names bounds without numbers", which is not true.
- **Fix.** Adopt the EX-8 numbers, or record the deviation from EX-8 with reasons. Also state whether the two
  `observation_bytes` sites are meant to share one limit name.

### V5-02-RA01-A-R6 (medium): observe stdout wrapper is an untagged choice that drops EX-8's stated property

- **What the candidate does.** RA-6 prints `{"version":1,"kind":"observations","observations":O}`.
- **What the design says.** Design RA-6 says Lua "assembles canonical observations v1 ... on stdout". EX-8 bounds the
  output so "observe never emits a file that `--observations` would refuse".
- **Why it matters.** With the wrapper, observe's stdout is always refused by `render --observations` (unknown
  fields), so a driver must extract `O`. The wrapper is consistent with the response-kind-to-envelope-kind pattern,
  so either reading is defensible.
- **Fix.** The choice is missing from the decisions table, which is required by criterion 2. Tag it as NEW, state the
  EX-8 deviation, and say that the 1 MiB bound applies to `O`. Alternatively, print the bare document as the plan did
  (D25/I17).

### V5-02-RA01-A-R7 (medium): observe validation codes and edge cases are not exact or not consistent with render

- **Duplicate IDs.** RA-6 refuses duplicate check or finding IDs as `schema`, field `id`. The approved render error
  table defines `reference` as "Missing/duplicate ID" (`render-contract.md`, "Errors and verification boundary"), and
  render's own observations validator reports duplicates as `reference` (`schema.lua:103` via `keyed`). Observe would
  give a different code from render for the same bytes.
- **Checkpoint bounds.** RA-6 says checkpoint-bound violations are `schema`. Render reports the 32-path and 8,192-byte
  checkpoint bounds as `resource_limit` (`checkpoint_files`, `checkpoint_bytes`; `src/stdlib/planar/observations.lua`).
- **Target digest format.** It is not stated whether the target digest format check ("Malformed output digest") is
  structural and so applied by observe.
- **Missing cases.** The outcome is not stated for:
  - a missing `--from` directory;
  - a `--from` that is a file or a symlink;
  - entry names that are not valid UTF-8 or contain controls, which would have to appear in the error `file` field.
- **Fix.** Pin each of these.

### V5-02-RA01-A-R8 (medium): the messages table relies on E16 outside its operator scope

- **E16's scope.** E16 A's text is "a scoped amendment of XD4 for execution only" (`execution-design.md`, E16
  option A). The design review says XD4 "forbids new public codes or messages for every remaining step"
  (`execution-design-review.md` E16 row).
- **What XD4 requires.** XD4 (`remaining-steps-design.md:67`) still requires every new native site to use existing
  messages.
- **What the candidate does.** The Messages table adds nine message strings tagged E16, and the decisions table cites
  "E16 A". Existing native messages already cover most of the native sites, for example "Capture limit exceeded",
  "Directory membership or identity changed", "Hard-link input alias" and "Explicit input file is a symbolic link"
  in `src/engine/capture/`. "Unknown job" duplicates render's existing "Unknown selected job" (`selection.lua:95`).
- **Fix.** For native sites, map each one to an exact existing message and say that no new message is needed. Keep
  new text only for Lua semantic messages, and say why XD4 does not cover them. If the author wants new native
  strings, the record must get the operator to confirm that E16 extends to RA01. The round brief's paraphrase is not
  that confirmation.

### V5-02-RA01-A-R9 (medium): command-contract keeps "no directory listing is exposed", and service results are unspecified

- **Unamended sentence.** `command-contract.md:196-197` (HEAD `:189-190`) still says "No ... subprocess or directory
  listing is exposed". The pointer adds `read_observation_directory` without amending that sentence, so the two
  documents disagree (criterion 3).
- **Unspecified results.** The authoring contract does not state the result value `V` of:
  - `read_diagnostic({})`, which returns only "the file bytes";
  - `read_observation_directory({})`;
  - `capture_orchestrator`, which the contract gives only by reference to `include:"spec"`.

  The finite-service protocol (`command-contract.md`, "Invocation values and finite native service protocol") fixes
  exact argument and result maps for every family. RA01-B wave 1 implements those ABI rows and would have to invent
  these shapes.
- **Fix.** Add the scoped amendment sentence and the exact result shapes.

### V5-02-RA01-A-R10 (medium): explain-rejection refuses real load diagnostics whose codes it does not list

- **What the contract accepts.** RA-5 accepts only render-table codes and the ledger contract's codes.
- **What load can emit.** `with-profile ... load` can emit the X-LIMIT codes `path` and `unsafe_path`
  (`external-profile-contract.md`, X-LIMIT paragraph). E4 A is "over load's stderr JSON".
- **Consequence.** A genuine load rejection would come back as `schema` "Invalid diagnostic file".
- **Fix.** Include those codes, or accept any code that matches the code grammar, and say how later rounds' codes
  (EX-7) are handled.

### V5-02-RA01-A-R11 (low): RA-9 decides an R15 binding question

- **The sentence.** "None of the RA01 verbs is such a read" classifies RA01 verbs under R15's E3 binding exemption.
  The database-free RA01 verbs have no accepted revision to bind to, and R15's ledger form of `orchestrator` is not
  specified here.
- **Why it matters.** That is R15 behaviour (criterion 5).
- **Fix.** Delete the sentence, or say that R15 decides it.

### V5-02-RA01-A-R12 (medium, records): the record leaves the plan differences and all 58 invariant rows undispositioned

- **What the record says.** "Plan versus candidate" lists conflicts as "not fixed here, the reviewer or a repair
  decides". All 58 rows of the copied invariant table say "not run" and are "owed to the reviewer".
- **What I checked.** I confirmed that the ID, invariant and mutant cells equal `plan/INVARIANTS.md`.
- **Why that is not enough.** For a docs round the rows are document checks, but the record must still state, for
  each row, whether the final contract satisfies it, supersedes it (with the reason) or fails it.
  - This review settles the conflicting rows I17, D23, D17/I51, I43/I44/I58, I46 and I47 through R1 to R10.
  - The record must also record the disposition of D13, D49 and D57, and of D61 (no `x07-addendum.md` pointer).
- **Fix.** After the repair, update the Final candidate identity and per-file hashes from a fresh `identity.py` run.
  Before approval, also update the acceptance-criteria table's status column (criteria 1, 3, 5 and 6).

### Non-blocking notes

- **Schema text types.** RA-4 rule 1's type list has no rendering for the `version` primitive, which is present in
  all three schemas, or for bounded `string` primitives such as `preferences.model`. State these when fixing R1, so
  that exact schema-line tests need no choice.
- **Same-error claim.** RA-2's "same typed error ... that load would give" holds only when the listed jobs exist and
  are valid. Load also captures and validates every job, and reports the first sorted error. A qualifying clause
  would help test authors.
- **Status text.** The contract header and the A35, A36 and v5-alignment pointers say "pending review". After
  approval the coordinator should update them, along with the roadmap.

## Evidence

Reviewer files, newly added under `tests/native/v5-02-ra01-a-evidence/review/`:

| file | content |
|---|---|
| `identity-candidate.txt` | `identity.py` over all nine candidate files, exit 0 |
| `links.py`, `links.log` | relative link and anchor resolver over the nine files, `bad 0`, exit 0 |
| `redact-check.log` | `redact-local-paths.py check` over the nine files, exit 0 |
| `diffstat.log`, `status.txt`, `agents-symlink.txt` | scope checks; `AGENTS.md` -> `CLAUDE.md` |

I read the gate runner's `REPORT.md` and `raw/gate-*` logs, and they agree with my runs. The `ctest -N` inventory
(1273 names, identical to X07-C02) is relevant only to show that no test changed.

## Limits

- **No build or tests.** This is a records-only review of a docs-only round. Nothing was built or run.
- **No red or probe wave.** No red-first or mutation-probe wave exists for this round (the record says so honestly).
  The invariant rows were not executed as scripts. Where a row bore on a finding, I checked it by reading.
- **What the citation check covered.** It covered every line the round record cites and the source lines named
  above. The design's own citations were re-read only where the contract depends on them.

## Records re-check (2026-10-30)

Disposition: **request-changes (records only)**. Eleven of the twelve findings are closed and R12 is closed
except for two small record wording issues. The fix brought in one new false claim, RV1 below. RV1 is a
one-sentence edit to `authoring-contract.md`. No source, test or evidence-data change is needed, and no
operator question is raised.

### Candidate and scope

I re-ran the workflow `identity.py` from the repository root over the same eight product documents
(`recheck/identity-candidate.txt`, exit 0). The output equals the record's "Final candidate" table byte for byte:

- HEAD is `a3364b8afae5ec7a75f2b19474df3fc89de9b078`.
- The tracked-only hash is `dc810d7b64fe46e7ce6b1d8cbef566d2e4bd8d82625976b72b323b40c22a7f59`.
- The combined hash is `f4192ad13b8df95863e92d79b2260098fbd60dca5dc64583e6151b61b68edc10`.
- Compared with my review, three per-file hashes changed and the other five are identical:
  - `command-contract.md` is now `a0c42317...4c83`;
  - `external-profile-contract.md` is now `0ebdff64...be50`;
  - `authoring-contract.md` is now `fa953571...a778`.

Other scope checks:

- **Source, tests and build inputs.** `git diff HEAD --stat -- src tests scripts cmake CMakeLists.txt` is empty,
  and nothing under those paths is untracked (`recheck/scope.txt`). No source, test or build file changed since
  the review.
- **Evidence data.** No file under `tests/native/v5-02-ra01-a-evidence/` outside `recheck/` is newer than my
  review's files (09:20:01, `recheck/evidence-mtimes.txt`), so no evidence data changed.
- **Diff and symlink.** The tracked diff is 7 files, 35 insertions and 5 deletions. The extra deletion is the
  amended "no directory listing" sentence. `AGENTS.md` is still a symlink to `CLAUDE.md`.
- **Links and redaction.** The link and anchor resolver over the nine files reports `bad 0`, exit 0
  (`recheck/links.log`). The redaction `check` over the nine files exits 0, read unpiped
  (`recheck/redact-check.log`).

### Findings, re-read against the bytes

| finding | status | basis (read in the current text) |
| --- | --- | --- |
| R1 | closed | RA-5: the first segment selects `workspace`, `spec` or `job`, and the rest descend. This matches `schema.lua:252,254,269`, and `semantic.lua:87-88` (`spec.<collection>[N]...`). A bare schema name gives the whole record. RA-4 rule 1 now renders `version` and `string MAX`. |
| R2 | closed | One rule. `raw_error` is a verbatim, unescaped block with a `max(3, L+1)` fence, and equality is defined over the block bytes. RA-4 states this exception. Load's error stderr always ends with LF (`response.cpp` `failed()`), so A36 equality holds. |
| R3 | closed | `blocking_question_unanswered` is reachable: `declarations.md:198-200` accepts an unanswered scoped question. The reachability argument for `facet_dependency_without_output_vector` matches `semantic.lua:305-306`, a per-job check that counts role checks. The dead consumer rule is gone, and the reason is recorded. |
| R4 | closed | RA-9 items 1 to 6 settle probe membership (five), precedence (profile errors first, then usage with the existing message at `response.cpp:175`, as in X-CLI step 7), the collision rule (`reference`, the X-REG identity conflict), completeness (five), the variant (closed at five, embedded-only set, RA01-B wave 1 owns the revision) and capacity and namespace. The external-profile pointer cites RA-9. The `#13-family-registry` anchor resolves. |
| R5 | closed | RA-6 bounds are 1024 names in total, 64 KiB per file, 4 MiB read and 1 MiB output. These equal `execution-design.md:331`. The read and output limits have distinct names. The false "no numbers" basis is removed. |
| R6 | closed | Observe prints the bare document plus LF. The RA-1 row, the output paragraph and the decisions row agree. |
| R7 | closed | Duplicate IDs are `reference` `Duplicate value` (`schema.lua:103`). Checkpoint bounds are `resource_limit`, with the details and messages of `observations.lua:57-58`. The digest check is `Malformed output digest` (`observations.lua:28`). A missing, non-directory or symlinked `--from` is pinned, and so is an invalid entry name. See note N1. |
| R8 | closed | No new message remains. Every mapped string exists with the stated code: `response.cpp:189-197`, `host_path.cpp`, `native.cpp:37,335,380,432,447,456` and `selection.lua:95`. E16 is not used. See note N3. |
| R9 | closed | The command-contract sentence is amended and scoped to `observe`, and the pointer says so. `V` is stated for `capture_authoring`, `capture_orchestrator`, `read_diagnostic` and `read_observation_directory`. |
| R10 | closed | Any code matching `[a-z][a-z0-9_]{0,63}` is accepted, and `path` and `unsafe_path` are named. |
| R11 | closed | The classification sentence is deleted. RA-9 leaves the exemption to R15. |
| R12 | closed, two wording nits | Every row I1 to I58 has a disposition. D13, D49, D57 and D61 are dispositioned. The identity, the criteria statuses and the files-edited list are refreshed. The D61 drop is consistent with RA-9 item 5. Nits are in N4. |

### New finding

**V5-02-RA01-A-RV1 (medium, records): RA-5 claims the shared parser refuses trailing whitespace, but it does not.**

- **The claim.** RA-5 says the diagnostic file is "parsed by the same strict parser as other JSON inputs:
  duplicate keys, invalid UTF-8, NUL and trailing data other than one final LF are refused".
- **The parser.** That parser is `json_parse` (`json.lua:54`), which calls `engine.json_decode`. It decodes with
  Glaze `validate_trailing_whitespace` (`codec.cpp:112`). That option refuses only trailing non-whitespace.
- **The proof.** The existing test `codec strict validation preserves object values and decoded duplicate
  policy` (`codec.t.cpp:90`) decodes `" \t{...} \n"` successfully, so leading whitespace, and trailing whitespace
  beyond one LF, are both accepted.
- **Why it matters.** The fix removed the canonical-form requirement (R10 row), and that requirement was the only
  thing that could make the sentence true. As written, a test author must choose:
  - test that `{...}\n\n` is refused, which needs a new check, and that check has no existing message (XD4); or
  - follow the parser and accept it.
- **Fix.** State that JSON whitespace before and after the value is accepted, exactly as the shared parser
  accepts it, and keep NUL, invalid UTF-8 and trailing non-whitespace refused. The A36 equality rule is
  unaffected. Evidence: `recheck/rv1-trailing-whitespace.txt`.

### Non-blocking notes

- **N1 (observe duplicate field).** RA-6 says a duplicate ID is refused with field `id`. The next sentence
  defines every field as "the render field path with its ... prefix removed". Render reports a duplicate at
  `observations.checks[N]` (`schema.lua:103`), so that general rule would give `$`. Add "except duplicates" or
  similar to remove the doubt.
- **N2 (output bound boundary).** The output bound applies to `O`, and stdout is `O` plus LF. Render's input
  limit applies to file bytes (`file_bytes` 1048576, `native.cpp:677-678`). At exactly 1,048,576 bytes,
  stdout saved verbatim is one byte over that limit. The contract avoids the problem by saying the driver writes
  `O`, but a sentence would help. `observation_bytes` is a new details value, not render's limit name.
- **N3 (XD4 reading).** XD4 says new native sites use the closed X-LIMIT table. The contract instead reuses
  capture-engine messages as propagated native errors, which is the route R8 asked for. RA01-B should therefore
  raise the observe and diagnostic bounds through the existing capture limit path (`native.cpp:37`) rather than
  through new message sites. In RA-5, "any violation is `schema`" also has one exception: the codec depth
  ceiling of 128 gives `resource_limit` `json_depth` for a deeply nested `details` object.
- **N4 (record wording).** Two dispositions say slightly more than the contract does:
  - the I41 disposition says the twelve verbs are listed "only in RA-7 and RA-9", but RA-9 does not list them;
  - the I53 disposition says the orchestrator text "states no ... check or re-render guidance", but RA-7 says
    only "It states no execution state".

  Neither changes a contract behaviour.

### Evidence

New files under `tests/native/v5-02-ra01-a-evidence/recheck/`: `identity-candidate.txt`, `scope.txt`,
`evidence-mtimes.txt`, `links.log`, `redact-check.log` and `rv1-trailing-whitespace.txt`. The resolver is the
review's `review/links.py`, unchanged. Nothing was built or run beyond these document checks.

## Records re-check 2 (2026-10-30)

Disposition: **approve (records only)**. RV1 is closed. The new RA-5 sentence is true against the parser code
and tests, clause by clause. Only the RA-5 sentence changed in the product documents. No source, test, build or
evidence-data file changed, and the fix adds no new false claim. Two non-blocking notes (N5, N6) are added
below. Evidence is under `tests/native/v5-02-ra01-a-evidence/recheck2/`.

### RA-5, read against the parser

The new text (`authoring-contract.md:272-275`) says the file is parsed by the shared strict parser. It says
"duplicate keys, invalid UTF-8, NUL and trailing non-whitespace data are refused, and JSON whitespace before
and after the value is accepted exactly as the shared parser accepts it". The parser path is `json_parse`
(`json.lua:54-60`) to `engine.json_decode` to `codec_decode` (`runtime.cpp:874`). Each clause:

| clause | code | test |
| --- | --- | --- |
| duplicate keys refused | `codec.cpp:283-285` (key set over the decoded spelling) | `codec.t.cpp:19` `{"a":1,"a":2}`; `codec.t.cpp:95-99` `a` duplicate, top level and nested, code `schema` |
| invalid UTF-8 refused | `codec.cpp:113` `validate_utf8`; outside strings only ASCII grammar bytes parse | `codec.t.cpp:20` lone surrogates, `codec.t.cpp:29` `"\xff"` |
| NUL refused | `codec.cpp:276-277` (strings), `codec.cpp:284` (keys) | `codec.t.cpp:20` `"\u0000"` and `{"\u0000":1}`, `codec.t.cpp:31` raw NUL in a string, `codec.t.cpp:160` `true\0false` |
| trailing non-whitespace refused | `codec.cpp:112` `validate_trailing_whitespace` | `codec.t.cpp:19` `true false`, `codec.t.cpp:160` |
| leading and trailing JSON whitespace accepted | Glaze `whitespace_table` is exactly space, tab, LF and CR (`vendor/glaze/a4a7/include/glaze/util/parse.hpp:74-81`) | `codec.t.cpp:90` decodes `" \t{...} \n"` |

"One UTF-8 JSON value" also holds: empty and whitespace-only input are refused (`codec.t.cpp:21`). The later
claim "The file is valid UTF-8 without NUL (the parser guarantees it)" (`authoring-contract.md:299`) stays true.
The A36 equality rule is unaffected. Evidence: `recheck2/ra5-check.txt`.

### Candidate and scope

- **Identity.** I re-ran the workflow `identity.py` from the repository root over the same eight product
  documents (`recheck2/identity-candidate.txt`, exit 0). HEAD is still
  `a3364b8afae5ec7a75f2b19474df3fc89de9b078`. The tracked-only hash is unchanged,
  `dc810d7b64fe46e7ce6b1d8cbef566d2e4bd8d82625976b72b323b40c22a7f59`. Seven of the eight per-file hashes are
  identical to `recheck/identity-candidate.txt`. Only `authoring-contract.md` changed, from `fa953571...a778`
  to `ef2360f1e5d903000168e2352a8e05ba21398d857ed93e5f6e42588f674bbbe0`. The combined hash is now
  `4259aac66122ffd7fc3e11c4887883033fce679c006972a3789112021f1b84ee`.
- **Only RA-5 changed.** Replacing the new three RA-5 lines (273-275) with the two prior lines (quoted in
  `recheck/rv1-trailing-whitespace.txt`) gives exactly the prior hash `fa953571...a778`. So nothing else in
  `authoring-contract.md` changed.
- **Source, tests and build inputs.** `identity.py` over `src tests/native scripts cmake CMakeLists.txt` gives
  the empty-diff hash and no files (`recheck2/identity-code.txt`). No file under the evidence folder outside
  `recheck/` and `recheck2/` is newer than the re-check (`recheck2/scope.txt`).
- **Other changes since the re-check.** Besides `authoring-contract.md`, `V5-02-RA01-A.md` and this file,
  `docs/vernacular/roadmap.md` changed (09:34:13; it was clean in `recheck/scope.txt`). The diff is two
  status lines: an RA01-A "In progress" entry and the next-round row (`recheck2/scope.txt`). The record's
  Outcome section declares it ("No file was edited by this outcome step except this record and the
  roadmap"). It is a status record, not a contract, so it is in scope. See N6.
- **Record.** "Records fix after re-check" names the one edit and the right citations (`codec.cpp:112`,
  `codec.t.cpp:90`). Its statement "No source, test or evidence-data file changed" is true.
- **Links and redaction.** These checks cover eleven files: the eight product documents, the round record, this
  review and `roadmap.md`. The link and anchor resolver, with URL decoding, reports `bad 0`, exit 0
  (`recheck2/links.log`). The redaction `check` exits 0, read unpiped (`recheck2/redact-check.log`).
  `AGENTS.md` is still a symlink to `CLAUDE.md`.

### Non-blocking notes

- **N5 (where nulls are refused).** In the same RA-5 bullet, the clause "unknown fields, nulls and non-integers
  are refused by the envelope below" is not false, because the envelope also refuses them. But the shared parser
  already refuses `null`, `1.0`, `1e0` and integers outside int32 (`codec.cpp:268-275`; `codec.t.cpp:19`).
  The message therefore comes from the parser, which the "parser or schema normalizer" wording covers. This
  wording predates RV1.
- **N6 (status records).** The roadmap entry and the Outcome section expected a separate records-only corrective
  round, `V5-02-RA01-A-C01`, with its own record. Instead the fix was logged inside the RA01-A record.
  The roadmap line still says RV1 "remains" and "Corrective RA01-A-C01 (records only) is next". The coordinator
  should update that status when RA01-A closes.
- N1 to N4 from the re-check stand as recorded.

Decision: **approve**. RV1 is closed. No blocking findings; non-blocking N5 and N6 are added.
