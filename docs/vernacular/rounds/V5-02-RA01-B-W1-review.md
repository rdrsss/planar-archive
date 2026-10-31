# V5-02-RA01-B wave 1 independent review (catalog and ABI rows with stubs)

Disposition: **request-changes**. The product structure is mostly sound. The production binary, its root help and
`--help` are byte-identical to HEAD. The eleven-row validator is built without a count check that could mask a
mutant. The six CLI11 families parse the RA-1 signatures. The operator answers (D2/D3 option B, D16 option B) are
honoured.

The round cannot be approved yet, for three reasons:

- The mutation evidence for the final candidate does not exist yet. The prober was still running while I reviewed.
- The D14 reserved-name pre-check does not fully meet gap 6. One reachable widening survives, and it is recorded as
  unreachable. The "malformed entries are skipped" rule that the plan states is neither implemented nor tested.
- The explicit-path stage gives `--registry` a missing-file message that no contract row, plan decision or existing
  verb gives it.

Several record findings also need fixing. Because R1 to R3 need new evidence or source and test changes,
`recordsOnly` is false.

## Exact candidate

I ran `identity.py` from the repository root with the pathspec `src tests/native`. The output is saved at
`tests/native/v5-02-ra01-b-w1-evidence/review/identity-candidate.txt`:

| identity | value |
|---|---|
| HEAD | `307026288615e317d145ba6846c0692929c50a17` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `519dec7b4aa45564700f2c0b1f7d7197a63b6382d2e9eb223898c1ae0acdcc85` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `588df1672b82a46079a07c54d5f171144e9a697229d7401bcf6c1e79bb5922ff` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/CMakeLists.txt` | modified | `a500e63c899b433cf8d253fb5f8dbe80061074b4d728c151940aeb5e9d75a823` |
| `src/cmd/planar/bootstrap.cpp` | modified | `9db6993381e4cbea3cbc2cb432504321b245dd7b2d341dbd2320917aa74f05a8` |
| `src/cmd/planar/bootstrap.t.cpp` | modified | `44684743e0b8f2b92955641dc8400baa8fb8f03da82bdcda72de05d3cc583bf9` |
| `src/cmd/planar/response.cpp` | modified | `233af0d52232e51855d0dfb00000fe20b5981e36c399aaf252cc6ee3ead74cb6` |
| `src/cmd/planar/response.cppm` | modified | `c007ebbfe83bd2b9f88234294b29fe9f704e279982a5eac61323d7befbf1cf31` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `076de8e73b8bd3cf93e2b442ff573e7a656a590820914bfd2d0899349af0da26` |
| `src/engine/runtime/runtime.cpp` | modified | `50c6c0efb0f917125c7e87bd4a0ba752d0e3b888d39b9ed2b8bd37ac51069c3f` |
| `src/engine/runtime/runtime.t.cpp` | modified | `f8d3a1fde07a32c472f200f4c6807f0608bcccf982ccce0cc6144b69855216bc` |
| `src/stdlib/planar/commands.lua` | modified | `23b597fa889f5d05869169f9ba9980e2cb242efd6a0316ada8ab8df08988072f` |
| `src/cmd/planar/authoring.cpp` | untracked | `e0e0eb11a5654e4492727918c843cb350d4f0d142ecd37e38afc423a1a9eb506` |
| `src/cmd/planar/authoring.cppm` | untracked | `4e56274a57a410a00fda867ea1474ce1c02a892a12cce0b0e26b762a075b2d09` |
| `src/cmd/planar/authoring.t.cpp` | untracked | `22ce075df56edf3b21e616c6489a93d98da89b824b332d57a6814f8875593a6c` |
| `src/cmd/planar/authoring_catalog.t.cpp` | untracked | `110674c27e1a34764eb710f85213590444a5e18263ee61bea3a2d6b205ae52c8` |
| `src/cmd/planar/handlers/critique/command.cpp` | untracked | `7b153955067b45b52769dae6c79b2d34656a6dd30a125a7c3ce2f883c3c2d8cb` |
| `src/cmd/planar/handlers/critique/command.cppm` | untracked | `379fb8be986b24d469de67e4853d7d76af90ce16b74a15e6834d9c6b28354a69` |
| `src/cmd/planar/handlers/explain-rejection/command.cpp` | untracked | `7af9d33c599e3557677c836075a6b917ce59c7a20e618f35675bb520a95760e3` |
| `src/cmd/planar/handlers/explain-rejection/command.cppm` | untracked | `dbfe243381fbc299c1014049b1f7103b96a60a6f214381efa3c591756103a58c` |
| `src/cmd/planar/handlers/ingest/command.cpp` | untracked | `ae1935f5e0f177aa374b84cec4f4027d7c1e4857c63e1a27bbdf87c21f3ac0dd` |
| `src/cmd/planar/handlers/ingest/command.cppm` | untracked | `e061a72ff6b6e3bc3487cb6b459791ad2d4b0beea81bc8f6a52128c4dc4e27ec` |
| `src/cmd/planar/handlers/observe/command.cpp` | untracked | `ef5485db8381cbc7d600eeebeeb2d140944dd7f21bc0fb4e0a4bbc0b42ec272d` |
| `src/cmd/planar/handlers/observe/command.cppm` | untracked | `4a7c212beec81cfb1c66e68e15e3f6864cdec001afeb8bf0961e0fc341023883` |
| `src/cmd/planar/handlers/orchestrator/command.cpp` | untracked | `f903ec66255b5d6e853ca0788660affc006cdf7cd176bc47e2d626c36e1cbc94` |
| `src/cmd/planar/handlers/orchestrator/command.cppm` | untracked | `59f63fa6e29478543b1a652cb993d4c11935408ae92935632d21765ff9626dc7` |
| `src/cmd/planar/handlers/spec-prompt/command.cpp` | untracked | `0a836e7a89bcf76ddc3c996086f0c4a2563c0589be39e553acbb43356cac515a` |
| `src/cmd/planar/handlers/spec-prompt/command.cppm` | untracked | `a603095ed5196e7169d49d3ab0e9762835a3e9186d2c1ed3200cf0055266ebb1` |

These are the same values as the round record's "Candidate identity (final)" table and gate run 6
(`raw/run6/identity.txt`). The Linux context manifest (`raw/run6/linux/container/context-manifest.txt`) has the same
hashes for `CMakeLists.txt`, `authoring.cpp`, `bootstrap.cpp`, `external_registration.t.cpp`, `runtime.cpp` and
`commands.lua`. So the gate and the container ran on these bytes.

For the documentation pathspec (`docs/architecture.md`, `acceptance-matrix.md`, `authoring-contract.md`,
`profile-preparation-architecture.md`, `v5-alignment.md`, `x07-addendum.md`), `identity.py` gives the combined sha256
`a1b0134f7155d1d2f3fc30fece7023aeda9eb998b78a393fcd382529b84c9a88`. That matches the record.

## What I checked and found sound

- **Production is unchanged where it must be.** Both build trees were current: `cmake --build` printed
  `ninja: no work to do.` for `planar_tests` and for the default target, in Debug and in RelWithDebInfo. The Debug
  and RelWithDebInfo `planar --help` bytes are identical to the five-line literal extracted from
  `git show HEAD:src/cmd/planar/bootstrap.cpp`. Each of the six verbs gives exit 2 with the existing usage envelope,
  and so does `observe --help`. `ninja -t commands planar`, and the same for `planar_runtime` and `planar_command`,
  contain no `PLANAR_AUTHORING_BUNDLE` in either configuration. `planar_authoring_stack` contains it 8 times (G15).
  `git status` shows `runtime.cppm`, `bootstrap.cppm` and `main.cpp` unchanged. `create_authoring` and
  `invoke_authoring_bundle` appear only in the absence checks at `bootstrap.t.cpp:3375-3376` (G13, G14).
- **Validator structure (gap 1).** `seal_catalog` has no count comparison. There is one statement per check, and a
  presence loop over `bundle_rows(c.authoring)`. `seal_environment` sizes its roots by the 64-entry registration cap
  and loops over `entries.size()`. So the expected row count follows the bundle (5 or 11), and a permissive mutant
  cannot index out of range. In `probes/raw/C-I12a.log` the prober re-adds the HEAD-style count check
  (`entries.size() != bundle_rows(...).size()`) with I12a applied, and all 66 tests pass. That proves the count check
  would mask the mutant. I14 is killed in the final run, by `validator refuses a duplicated authoring row` at
  `authoring_catalog.t.cpp:144` (`expected refusal, got a sealed VM`). Every negative test first requires the control
  variant to seal.
- **Rows, kinds, capabilities and option signatures** in `runtime.cpp` and `commands.lua` match the RA-1 table
  byte for byte. That includes the underscore handler IDs, `observe_v1` mapping to the engaged empty envelope kind,
  and the sorted option lists. The six root-help lines in `authoring.cpp` are byte-equal to the RA-1 code block.
- **commands.lua layout (gap 8).** There are six named local stub functions and six single-line
  `registration.add(authoring_descriptor("<verb>"),<name>_command)` lines inside `if registration.authoring then`.
  Native code sets `registration.authoring` only under the definition and only for non-profile VMs (D3).
- **Stages and precedence (gap 3).** `run_stub` runs `prepare_inputs` first and only then refuses. The selector
  stage runs only for spec-prompt, critique, ingest and orchestrator. explain-rejection and observe call neither
  reader. The cwd is read lazily, at most once. In the stack, usage and catalog sealing come before dispatch. This
  matches Common rule 7.
- **With-profile (RA-9).** The probe stays five. I45 uses malformed tails under a malformed or spinning profile, so
  adding the six to the probe would change the output: gap 7 is closed by a named observable. The D14 pre-check sits
  between `validate` and `history::validate_catalog`, uses only `find` and kind tests, and returns the sorted index so
  that the existing remap reports the registration ordinal (I48b and I51b test both orders).
  `projection_reserved_ids` grows to 23 entries (D16 option B).
- **Tests and inventory.** My own focused runs of `ctest -R RA01-B1 --no-tests=error` matched 66 tests and passed 66
  in Debug and in RelWithDebInfo, with no skips. The `ctest -N` inventories of the two configurations are identical
  (1339 names). Against `tests/native/v5-02-ra01-a-evidence/inventory.txt`, 0 names were removed and 66 were added,
  every one containing `RA01-B1`, and they equal `new-names.txt`. My lint run
  (`python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`) exited 0 over 163 files.
- **Full gate logs.** `raw/run6/{debug,release}.full.log` show 1339 of 1339 passed. The X06 B3 C test (#609) passed
  in 16.49 s in Debug and 5.05 s in RelWithDebInfo, and `x08.timeout_headroom_531` passed. The Linux container logs
  show 1339 of 1339 in both presets. `x07.two_process_cas_one_winner` (#1330) passed in both, and the two Linux
  inventories are identical. The only "skipped" hit is a test name.
- **Red-first.** The `red/snapshot-2/` hashes match `RED.md` (`candidate.diff` `1e6dfd3a...`, `SHA256SUMS`
  `d23c1470...`). The red failures are assertions, not compile breaks.
- **Documentation.** The x07-addendum line 126 and `profile-preparation-architecture.md:232` are byte-identical (G12).
  Section 1.2 has six definite admission rows, 1.3 the embedded-only paragraph, and 1.6 the compile-time paragraph
  without a new creation site. The three status words read "approved and committed at 30702628".

## Findings

### RA01-B-W1-R1 (blocking, evidence): no final probe evidence

`probes/PROBES.md` does not exist. While I reviewed (17:41 to 17:57), the prober was still running the 35-mutant
re-probe list in its scratch copy. `results.jsonl` had 19 entries. I06 was recorded as `patch-failed`, because the
fresh_options repair moved its Lua anchor. I35 survived (see R4). E07, E08, E15 and E17 survived.

The rows outside the re-probe list rely only on `probes/superseded-1/`, which ran on a different candidate:
`commands.lua` and `external_registration.t.cpp` differ, with combined sha256 `0f276a2b...` against `588df167...`.
The record correctly marks acceptance 6 as not met. Acceptance 7 (each new test killed by its named mutant, from
`PROBES.md`) and the invariant-table check this review must do therefore cannot be completed.

Fix:

- Finish the run, re-attempting I06 with an anchor that exists.
- Write `PROBES.md` with one raw log per mutant and the killing tests named.
- For every row not re-probed, state the carry-forward argument: which files the mutant touches, and that those
  files and the killing test are byte-equal between the two candidates.
- Then update the record's invariant table from `PROBES.md`.

### RA01-B-W1-R2 (blocking, test and source): the D14 pre-check pin is incomplete

Gap 6 asks that the pre-check act only on exact reserved strings, skip malformed entries, and come with a pin that
catalogs without reserved names keep their current diagnostics. Two parts are not met.

(a) **E17 is reachable but unpinned.** E17 makes the pre-check also test `response_kind`, and it survived in both
probe runs. The record calls it "unreachable in practice". That is not so. `register_external`
(`runtime.cpp:1280`) checks only the argument types, and `exact()` checks only the key set. So a profile can
register `response_kind='observe'`. At HEAD and in the candidate, `history::validate_catalog`
(`effective_catalog.cpp:246`) refuses it as `internal` / `Invalid external profile protocol`. Under E17 it would
become `reference` / `External registration identity conflict`.

This catalog has no reserved name in the RA-9 sense, yet no test pins its diagnostic. Add a test in which reserved
strings appear only in non-reserved fields: `response_kind`, `help`, an option name and a capability. It must assert
the unchanged HEAD diagnostic, and it must kill E17.

(b) **"Skip malformed entries" is not implemented and not tested.** D14 and gap 6 say malformed or ill-typed entries
are skipped. `reserved_authoring_identity` skips only non-record items. It reads an ill-typed field as empty and
still tests the other fields. So `{name='observe', handler_id=5, ...}` is refused as an identity conflict instead of
being skipped.

I53's test uses only the non-reserved name `mine`, where skipping and not skipping give the same result. So I53
cannot observe this property: its mutant pins only that ill-typed fields are not reserved. Choose one of two fixes:

- Make the code skip any entry with an ill-typed name, family or handler.
- Amend D14 and the I53 row to the implemented rule ("ill-typed fields read as not reserved; a well-typed reserved
  field refuses the entry").

Either way, add a test for an entry that has a reserved name and an ill-typed second field.

### RA01-B-W1-R3 (blocking, source and test, unspecified product choice): the missing `--registry` message

In `prepare_explicit` (`authoring.cpp:84-95`), a missing leaf that `resolve_explicit_file` reports as
`io` / `Explicit input must be a regular file` is turned into `io` / `Cannot resolve explicit input path`. This
happens for every explicit file, `--registry` included, and I42 pins it for `--registry` ("`--registry` missing as
`--diagnostic`"). Nothing decides this for `--registry`:

- The Messages table remaps only `--diagnostic` and `--from`.
- D8 says the stage uses `resolve_explicit_file` for `--registry` and `--diagnostic` "with their existing messages".
- Common rule 5 classifies `--registry` "like the existing options".

I ran the candidate binary with a missing `--registry` in a scratch directory:

| verb | message for the missing `--registry` |
|---|---|
| `render` | `Explicit input must be a regular file` |
| `load` | `Cannot inspect confined input` (lexical capture) |
| authoring stage | `Cannot resolve explicit input path` |

So the new mapping matches neither precedent. Choose one of two fixes:

- Keep the unmapped `resolve_explicit_file` message for `--registry`, which is D8 read literally and matches render.
  Limit the remap to `--diagnostic` as the Messages table directs, and update I42.
- Raise the choice to the operator with these options.

The `--diagnostic` and `--from` remaps follow the more specific Messages rows. Common rule 5's "exactly like
`resolve_explicit_file` ... messages" contradicts them for a missing leaf. The record should state which clause it
follows.

### RA01-B-W1-R4 (blocking, records): I35 is a hollow row and I52 is out of date

The I35 mutant (skip the `--name=value` preflight in the stack) survived in both probe runs
(`probes/raw/I35.log`, 66 of 66 pass). The parsed-token count check already refuses equals syntax, so the mutant
does not break the invariant. Only I35b, which removes both checks, is killed. Restate the I35 row with the I35b
mutant, and record I35 as an equivalent mutant.

The I52 row still says that the projection `Orchestrator` "stays registrable". The test now asserts that it is
refused with a non-collision diagnostic (`external_registration.t.cpp`, the uppercase block). The red failure at
`:5290` was that very case. Correct the row text, and note that the test body changed after red.

### RA01-B-W1-R5 (blocking, records): the I68 re-pin has no traceable authority and is only half described

The record says the operator re-pinned the X08 C01 outputs-off boundary on 2026-10-30, by 68 bytes. `REPORT.md:4`
calls it "the orchestrator's re-pin". The operator answers passed to this run contain only D2/D3 option B and D16
option B, and nothing about this pin.

The diff against HEAD also moves the pin a first time, from 228586 to 227593 on macOS (228612 to 227619 on Linux).
That move followed the gate run 1 failure (`raw/run2/REPORT-run1-prior.md`). It appears only in the test comment and
is absent from the record, the plan (D20 says "Budgets: none new") and the I68 row.

Cite the decision verbatim with its source, or record the re-pin as a coordinator re-measurement with its rationale.
Describe both moves (993 and then 68 padding bytes), with the run that measured each.

### RA01-B-W1-R6 (blocking, records): D13 is missing from the record scope; the owned-file list is incomplete

The plan records D13 with the corrected `prepare_invocation` reason, and it carries a "Wave-2 note (paste into the
round record scope and the wave-2 brief)". The record never pasted that note. Its "Work content" item 4 still says
that the opaque slots `diagnostic` and `from` are "added to the closed slot list" in this wave, with no supersession
marker. So acceptance 8 is only partly met: D13 is in the plan only, apart from the gap statement.

Also, the sync edited `docs/vernacular/rounds/V5-02-RA01-A.md`, a committed and approved record. That file is not in
the "Owned files (final)" list.

Fix: mark item 4 as superseded by D13, paste the wave-2 note into the record, and add `V5-02-RA01-A.md` to the owned
list, or justify the edit there.

### RA01-B-W1-R7 (blocking, evidence): the redaction gate skipped committed evidence

G08 covers "the evidence files and docs this round commits". The gate's explicit list (`raw/run6/redact-files.txt`)
has only sources, docs and the record. I ran `redact-local-paths.py check` with `--home "$HOME"`. Sources and docs
pass, and so does `V5-02-RA01-A.md`. These committable evidence files fail:

- `REPORT.md`: lines 68-69 contain the home path.
- `plan/INVARIANTS.md`: line 142 contains the workflow script path.

Redact them, and re-run the check on every evidence file that will be staged: the report, the README, the plan,
`PROBES.md`, `RED.md` and the inventories.

### RA01-B-W1-R8 (required, documentation): architecture.md omits production with-profile changes

This wave already changes production `with-profile` behaviour:

- A profile that registers any reserved command, family or handler name, `orchestrator`, or one of the five
  `orchestrator_*` projection IDs is now refused as `External registration identity conflict`.
- The section-ID union grows from 18 to 23.

The new paragraphs in `docs/architecture.md` (`:767-781` and `:1613-1621`) describe only the embedded-only verbs and
the compile-time bundle. Add one sentence each on the reserved-name refusal and the extended section-ID union, which
are now live in the production binary.

### RA01-B-W1-R9 (advisory, records): `OP` tags and plan IDs

DECISIONS tags D7, D8, D13 and D14 as `OP`, but the operator answers contain only D2/D3 and D16. These four rows trace
to the dispatch gap list (gaps 3, 6 and 9) and the dispatch acceptance text. Retag them, or cite the source.

The new x07-addendum 1.2 rows cite "D23", which names no round. Write it as "RA01-B wave 1 plan D23".

## Limits of this review

- I did not apply mutants: only the prober does that. I re-ran no full ctest and no Linux gate. Those results come
  from the run 6 raw logs, which I read rather than took from the summary.
- The probe run was incomplete while I reviewed (R1), so I could not check the invariant-table kill column against
  `PROBES.md`.
- I67 has no red record. Its failing evidence is E18 killed in the final run
  (`replacement.RA01-B1 reserved command is refused when the profile also registers a projection`), which is
  acceptable once R1 lists it.
- I ran the candidate `planar` binary only with temporary `HOME` and ledger paths in the session scratchpad. I ran no
  script under `tests/native/`.
