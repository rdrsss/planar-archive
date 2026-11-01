# V5-02-RA01-B-W2 independent review (prompts, observe, orchestrator projection)

Disposition: **request-changes**. The candidate is close. The six verbs work, the frozen `commands.lua` layout holds,
the production binary still has no authoring path, all 1344 earlier test names are kept and the suite passes. But four
findings block approval. R1 is a contract conformance gap: `observe` has a reachable failure site whose message is not
in the authoring contract's Messages table, and the round's own decision D-13 said a coder must stop in that case. R2
is about the probe evidence: several surviving mutants are labelled "not proven equivalent" or "test gap", but they can
be killed with small engine-level or CLI tests, so acceptance 5 is not met. R3 is a hollow invariant row: W2-06's test
says it relies on a handler assertion that does not exist. R4 is stale reference documentation. None of these can be
fixed by editing round records or evidence READMEs alone. R5 to R8 are minor and do not block.

I reviewed the uncommitted candidate on HEAD `3343ad38` against authoring-contract RA-2 to RA-7, its Messages table and
acceptance checklist, execution-design section 5, the round record, `plan/INVARIANTS.md`, `plan/DECISIONS.md`,
`probes/PROBES.md` (including `probes/superseded-1/`), `red/RED.md`, `c02-pin.md` and the gate `REPORT.md` with
`raw/run3/`. I read the source diff and the new files in full: `runtime.cpp`/`.cppm`, `capture.cpp`/`.cppm`,
`directory*.cpp*`, `invocation.cpp`/`.cppm`, `bootstrap.cpp`, `authoring.cpp`/`.cppm`, the six handlers, `commands.lua`,
`schema.lua`, `semantic.lua`, `prompts.lua`, `observe.lua` and `orchestrator.lua`. I read the new tests selectively
(A35, A36, the opaque-slot test, the directory tests, the variant tests, the I25 test).

## Exact candidate

identity.py (the workflow script), run from the repository root on `src`
(`tests/native/v5-02-ra01-b-w2-evidence/review/identity-src.txt`):

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src), tracked only | `645326b50052e5f27e132a1ad6149cacad676a32300be8985115f2318e2e1431` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `b23aac9f30f0daa9f540aafe784a9d9a191faff027adbef31e474a67b77734ac` |

These match the round record's `src`-scoped identity. The per-file hashes match the record's table, the gate's run 3
table and the probe pass table (`probes/raw/identity-before.txt`). So the gate and the focused re-probe ran on these
bytes.

## What I checked and found sound

- **Frozen `commands.lua` layout and disjoint hunks.** The only change to `commands.lua` replaces the refusal helper and
  the six stub bodies with six one-line delegates (`prompts_spec_prompt`, `prompts_critique`, `prompts_ingest`,
  `prompts_explain_rejection`, `observe_directory_command`, `orchestrator_command_impl`). `authoring_rows`, the six
  registration lines and the `registration.authoring` gate are unchanged. Each verb's line is its own hunk, so package
  ownership by line is disjoint. The new Lua files are embedded only in the authoring stack and its variants
  (`authoring_lua_files`, D-04). In the production bundle the delegates name nil globals behind a gate that is false.
- **Production binary has no authoring path (I25).** Every authoring addition in `runtime.cppm`/`.cpp` and
  `invocation.cppm`/`.cpp` sits inside `#ifdef PLANAR_AUTHORING_BUNDLE`: the intent type, the five service virtuals,
  `invoke_authoring`, `prepare_authoring_invocation`, service operations 7 to 10, the response row and
  `execute_authoring`. The definition is added only to `planar_authoring_stack_invocation`, next to the existing runtime
  and bootstrap stack targets. The widened test `RA01-B1 production targets never receive the authoring definition`
  keeps its production-binary checks. In its interface scan it allows the token only on `#ifdef` lines and in doc
  comments. `RA01-B1 default invoke and production binary refuse all six authoring verbs as usage` still pins exit 2 and
  the usage bytes for the production entry.
- **D13 slots.** The `diagnostic` and `from` slots, along with `workspace`/`registry`, are created per verb in
  `prepare_authoring_invocation`, the same way the `path` lambda of `prepare_invocation` creates them. Each becomes an
  opaque userdata in `execute_command`, and unused intent members are refused. This is a separate invocation builder,
  not a branch inside `prepare_invocation`. That follows the reviewed decisions D-03 and D-07, which keep
  `command_options` closed at five as RA-9 item 5 requires. I accept it as recorded. See R3 for the observability claim.
- **Native protocol.** `call_service_native` offers exactly the services each verb needs. It checks call order
  (`calls`), that the `include` matches the verb, and that the orchestrator `job` equals the retained intent. It refuses
  a provider result with an extra key, and it appends the sorted sealed command names after the provider returns (D-18,
  RA-2 as amended). `validate_authoring_response` checks the RA-3 envelope (kind per verb, `bytes` equal to the Markdown
  length, digest shape) and the bare observations shape for `observe`.
- **Contract amendment.** The RA-2 and RA-7 sentences and the command-contract pointer are exactly the D-39 text. The
  amendment cites the operator answer of 2026-10-31 (D-18 option A) and leaves the Messages table unchanged.
- **Production-embedded Lua.** The `mode` parameter in `schema.lua` and `semantic.lua` changes nothing when it is
  `nil`. Every guard is `mode~="workspace"` or similar, so `nil` keeps the old branches, and no diagnostic string
  changed. The guard test W2-76 shows render and load bytes are unchanged.
- **Diagnostics.** I extracted every message literal from the added product lines and the new Lua files. Each one
  already exists in HEAD product source: "Invalid string", "Invalid JSON", "Malformed output digest", "Duplicate value",
  both checkpoint messages, "Directory membership or identity changed", "Hard-link input alias", "Invalid captured
  path", "Links and special files forbidden", "File changed during read" and "Unknown selected job". "Expected object"
  is the normalizer's `"Expected "..schema.kind`. No existing message was edited. The problem is the mapping of some
  sites to the Messages table, which is R1.
- **Observe mechanism.** The root is opened `O_DIRECTORY|O_NOFOLLOW`. Each listing goes through a fresh `openat(".")`
  descriptor, and names are sorted with `std::string` ordering, which is unsigned byte order. The 1024-name counter is
  shared across all listings and counts dot names and subdirectories. Each file gets a `fstatat` no-follow kind check,
  then the hard-link alias check by device and inode, then a no-follow open with identity and size checks against the
  descriptor. The size and total limits are checked before the buffer is allocated, and a short or extra read is
  `capture_changed`. The second full scan compares listings, directory identities, entry identities and bytes, maps any
  difference to the two contract messages, and does not retry. Entries are re-sorted by category and then name, so the
  output does not depend on readdir order. Precedence is sequential in sorted name order. That satisfies "an unsafe
  entry takes precedence over a later size limit", and the size pre-pass mutant is killed. In `observe.lua`, validation
  order (checks, findings, checkpoint), the relative field rewrite, digest shape, duplicates and checkpoint bounds match
  RA-6.
- **Prompts.** I checked `prompts.lua` against RA-3 to RA-5: schema text generated from the `schema.lua` tables with
  sorted fields and nesting, checks as the allow-list intersected with the registry, the shape catalog order, the
  literal critique-step line, the three critique rules with sorting and the fixed lines, the job-tier constructor
  statement, the strict diagnostic envelope and code grammar, the `max(3, L+1)` fence with one added LF, and field-path
  resolution with `[N]` stripping and the fixed fallback line.
- **Orchestrator.** `orchestrator_context_v1` has exactly the RA-7 keys. Roles and work/target lists are sorted, the
  stage order is preserved, prescriptions follow the scope rule, and the graph comes from the validated job model.
  `status` is `available` exactly when the name is in the native `commands` list and `future` otherwise, and the future
  list is the twelve E1 verbs. Variant 20 seals a twelfth `status` row, and `RA01-B2 orchestrator label flips to
  available for a catalog stub verb` shows the flip. W2-73 keeps `orchestrator_context_v1` out of the external inputs.
- **A35 and A36.** The A35 test spawns two fresh stack processes per prompt kind, compares the full envelopes, and
  compares Markdown, bytes and digest against a fixture with reversed declaration order. The A36 test runs a real
  rejected load in a scratch ledger, embeds its stderr byte for byte, keeps load at exit 3 with identical bytes before
  and after, and leaves the ledger unchanged. The DEL/C1 and `job.roles[1].work` parts of the checklist are covered by
  W2-38 and W2-36 with synthetic diagnostics, because a real load stderr carries neither. I accept that.
- **Inventory and gate.** I ran `ctest -N` on `build/rewrite-debug` and got 1416 names. Sorted, they equal
  `inventory.txt`. Against the committed W1-C01 inventory (1344 names), 0 are removed and 72 are added. The run 3 Debug
  and RelWithDebInfo sorted lists are byte-equal. `cmake --build build/rewrite-debug --target planar_tests` gave
  `ninja: no work to do.` My own full run, `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure
  -j4`, passed 1416 of 1416 with exit 0 and no skipped or not-run tests (`review/ctest-debug-review.log`). The run 3
  Linux logs show 1416 of 1416 in both configurations.
- **Boundary pin.** `c02_off_boundary` moved from 227525 to 227275 on macOS and from 227551 to 227301 on Linux. Both
  values are recorded in the test comment, `c02-pin.md` and the round record, under the 2026-10-30 operator decision.
  The measured production-embedded bytes (`runtime.cpp`, `commands.lua`, `schema.lua`, `semantic.lua`) equal the final
  candidate. The `orchestrator.lua` hash in `c02-pin.md` is not production-embedded. The test passes in all four gate
  configurations.
- **Red-first.** `red/RED.md` records 69 assertion failures against the wave 1 stubs and 10 passing guards, with the
  exact per-test command and a snapshot hash. These are behavioural reds, not compile failures.
- **Redaction.** I ran `scripts/redact-local-paths.py check` on the round record, both changed contracts, the evidence
  README, REPORT, `c02-pin.md`, the plan files, PROBES, RED, `rows.tsv`, the name lists, the new Lua and directory
  sources, the new tests and my own review files. It exited 0. `probes/raw/*.log` contain host paths and must not be
  staged, as the evidence README already states.

## Findings

### V5-02-RA01-B-W2-R1 (blocking): observe and diagnostic-capture failure sites outside the Messages table

Common rule 4 says: "Every RA01 failure site uses an existing native or Lua message, mapped in Messages". The round's
decision D-13 says: "No site is left without a row; a coder who finds one stops that package". Several sites violate
this:

- `directory.cpp` returns `cannot_inspect()`, which is `io` / `Cannot inspect confined input`, when `openat`,
  `fstat`, `fdopendir`, `readdir` or `fstatat` fails. One case is easy to reach: a `checks` directory or a check file
  that the user cannot read. I reproduced both with the authoring stack. Each gives `{"code":"io","details":
  {"operation":"capture"},"message":"Cannot inspect confined input"}` and exit 4 (`review/repro.txt`). The Messages
  table has no row for an observation entry that cannot be inspected.
- `capture_diagnostic` (`capture.cpp`) returns `path_denied` / `Only regular files can be captured` for a symbolic link
  or a non-regular file, and `io` / `Cannot inspect confined input` for a failed open or `fstat`. Here the contract maps
  a symbolic link to `Explicit input file is a symbolic link` and a non-regular file to `io` /
  `Explicit input must be a regular file`. The path stage catches the ordinary cases first, so these native sites are
  reached only when the file is replaced after the path stage. Even so, the race site uses messages, and for a
  non-regular file a code, that differ from RA-5 and the Messages table. The engine test pins
  `Only regular files can be captured`. An engine-level symlink root in `read_observation_directory` likewise gives
  `io` / `Cannot inspect confined input`.
- `scan_once` and `capture_diagnostic` turn `std::bad_alloc` into `resource_limit` / `Capture limit exceeded` with no
  `{limit,maximum}` details. RA-6 and the Messages table require these details on every bound refusal. Existing capture
  code has the same pattern, but the record should say whether it is meant to be a row.

The round record's acceptance and limits never mention these sites, and DECISIONS.md D-13 claims full coverage.

Fix: map each site to an existing Messages row. For example, a file or entry that changes kind or identity between the
path stage and the read could be `capture_changed`. Where no row fits, add the rows through a scoped, reviewed
amendment of the Messages table with the authority that requires. Then add a test per site; an unreadable `checks`
directory is cheap to build. This changes source, tests and possibly a contract, so it cannot be done in the records.

### V5-02-RA01-B-W2-R2 (blocking): killable survivors recorded as unproven or as test gaps

Acceptance 5 requires every named mutant to be killed or explained as an equivalent mutant. The record leaves the
following to the reviewer. My verdict: they are not equivalent, and small tests can kill them.

- **X16** (`capture_diagnostic` gate opened without `O_NOFOLLOW`). The record calls it probable defence in depth that
  "needs an injectable handler or a different harness". That is wrong. `capture.t.cpp` already calls
  `capture_diagnostic` directly. Take a dangling symlink: the original fails with `ELOOP` and returns `path_denied`.
  The mutant fails with `ENOENT` and returns `io` / `Cannot inspect confined input`. Take a symlink to a regular file of
  70000 bytes: the original returns `path_denied`, and the mutant follows the link and returns `diagnostic_bytes`.
  Either case kills the mutant. The no-follow property is a security property.
- **X17** (observation root opened without `O_NOFOLLOW`). `directory.t.cpp` can call
  `read_observation_directory_impl` with a symlink to a directory. The original refuses it and the mutant reads the
  target. The record itself says there is "no engine-level root-symlink test".
- **X03** (checkpoint canonical bound 8192 versus 8193), **X04** (a prompt of exactly 1048576 bytes accepted), **X05**
  (a `"global"` prescription with only `not_applicable` checks must fire `prescription_unchecked`) and **X07**
  (a non-digit `[x]` index in a field path must give the fixed excerpt line). Each is an exact contract boundary or
  rule (RA-6 checkpoint bounds, RA-3 `prompt_bytes`, RA-4 rule 6, RA-5 path resolution), and PROBES.md records each as
  an open test gap.
- **W2-51** (listing sort removed). The row says the mutant is observable on Linux and that the prober would record a
  kill in the container. It survived on both platforms. The record now says the sort is observable only through error
  precedence. That is true, and it is also testable. Put an early-named unsafe entry and an early-named oversize entry
  in one listing, together with many later-named oversize siblings. Readdir hash order then almost certainly breaks the
  sorted precedence that the current two-entry fixtures happen to keep. Either strengthen the fixture or write a
  reasoned equivalence argument that the plan accepts. As it stands, the row is neither killed nor equivalent.

X01 and X11 are defensive guards against a provider or Lua that the shipped stack never produces. I accept them as
stated limits. The remaining survivors that the record classes as equivalent (W2-23, W2-67, X12 to X15, W2-15,
W2-66b, W2-77c, and the redundant single off-by-ones of W2-33 and W2-79) agree with my reading of the code.

Fix: add the tests above. The prober then re-runs X16, X17, X03, X04, X05, X07 and W2-51 in a scratch copy and records
the killing test names. This changes tests and probe evidence.

### V5-02-RA01-B-W2-R3 (blocking): W2-06 depends on a handler assertion that does not exist

INVARIANTS row W2-06 states: "the handler assertion refuses internal when the slot is text". Decision D-07 says:
"Handlers assert `type(invocation.options.<slot>)=="userdata"` and refuse `internal` otherwise". The test comment in
`authoring_orchestrator.t.cpp` (lines 205 to 206) says: "The handler assertion then refuses internal exit 4". No such
assertion exists. Neither `prompts.lua` nor `observe.lua` reads `invocation.options.diagnostic` or
`invocation.options.from`, and `grep userdata src/stdlib/planar/*.lua` finds nothing. The test `RA01-B2 diagnostic and
from reach Lua only as opaque slots` is therefore a positive smoke test. It cannot detect the mutant it is named for,
and the record confirms that the narrow mutant W2-06n survives the whole RA01 suite. The record does list this as a
limit. But the row, the decision and the test comment still state a mechanism that does not exist, and that is exactly
the kind of hollow row the review brief tells me to flag.

Fix: either implement D-07, a one-line type check in the two Lua handlers that is within the round's scope (the test
then kills W2-06n and the prober records it), or correct the W2-06 row, D-07 and the test comment to describe a
documented limit. Either fix edits a source or test file.

### V5-02-RA01-B-W2-R4 (blocking): reference documentation is stale

`docs/architecture.md` (around lines 779 to 785) still says: "Until their behaviour lands, each authoring handler is a
stub: after the selector and path stages it exits 4 (internal) with `Invalid embedded command protocol`." That is false
for this candidate. The document also never mentions the new `planar_capture_directory` module, the separate
`invoke_authoring` / `execute_authoring` path, or `capture_diagnostic` in the capture module. CLAUDE.md requires that
affected reference documentation be updated with behaviour or architecture changes. D-27 assigns this update to the
coordinator, and `docs/architecture.md` is on the round's candidate file list. The round record admits the text is
stale and defers it to "the commit step or a docs edit". It should land with the reviewed candidate, not after
approval.

The same applies to `src/engine/capture/directory.cppm`. Its exported `read_observation_directory` still carries
`@note Stub: refuses internal until implemented.`, which is wrong Doxygen on an implemented function.

Fix: update `docs/architecture.md`: the authoring paragraph, the module table entry for the directory read, and the
separate embedded-only path with `command_options` closed at five. Correct the `directory.cppm` note.

### V5-02-RA01-B-W2-R5 (minor): explain-rejection passes the codec's `resource_limit` through instead of `schema`

RA-5 says: "any violation is `schema` exit 3 with `file` absent and `field` starting `diagnostic`". `parse_diagnostic`
copies `decoded.code`, so a diagnostic nested more than 128 levels deep gives `resource_limit` with
`{limit:"json_depth",maximum:128}`, field `diagnostic`, exit 3 (`review/repro.txt`, 601 bytes). Common rule 8 ("the
native conversion limits apply unchanged") supports this behaviour, so I do not treat it as a defect. But the record
should state this reading, or the code should map the codec failure to `schema` if the operator prefers the literal RA-5
text. Records-only unless the behaviour changes.

### V5-02-RA01-B-W2-R6 (minor): the orchestrator projection is a private function, not a `projection_v1` value

RA-7 says the projection "returns `{version:1,sections}` with sections in this order (all `mandatory:true`)". In
`orchestrator.lua`, `project(c)` returns a plain list of `{id,text}`. Nothing outside the function can observe the
difference: the section IDs, their order and the context-only input all hold, and the reserved IDs are enforced at
registration. The record or D-36 should still say that the projection is an internal function shaped by RA-7, with no
registered `projection_v1` value. Records-only.

### V5-02-RA01-B-W2-R7 (minor): `capture_diagnostic` and `capture_limit` are exported unguarded from the production capture module

These two functions are added to `planar.engine.capture`, which the production binary links, outside
`#ifdef PLANAR_AUTHORING_BUNDLE`. Only the guarded `invocation.cpp` calls them, so I25 behaviour is unaffected and
this is dead code in production. D-27 describes the guard as covering "the `capture_diagnostic`/directory use in
`invocation.cpp`", which is accurate, but the record should say that the definitions themselves ship in the production
module. Records-only, unless the round prefers to guard them.

### V5-02-RA01-B-W2-R8 (minor): stale statements in evidence notes

`c02-pin.md` still describes the missing `planar_hash` dependency as an open blocker and lists the old
`orchestrator.lua` hash. The evidence README and the round record already explain both. The new RA-7 sentence reads
"(native passes the sealed command names; native passes them as the `commands` key ...)", which repeats itself
but is the exact D-39 text. Records-only. Nothing to fix beyond what the README already says, unless the commit step
wants to touch the contract wording.

## Evidence

- `tests/native/v5-02-ra01-b-w2-evidence/review/identity-src.txt`: identity.py output for `src`.
- `tests/native/v5-02-ra01-b-w2-evidence/review/repro.txt`: the two unreadable-entry `observe` refusals (R1) and the
  deep-nesting `explain-rejection` refusal (R5), from the Debug authoring stack, with scratch paths replaced by
  `<scratch>`.
- `tests/native/v5-02-ra01-b-w2-evidence/review/ctest-debug-review.log`: my full Debug run, 1416 of 1416 passed with
  exit 0, repository path replaced by `<repo>`.
- Commands I ran: `cmake --build build/rewrite-debug --target planar_tests` (no work); `ctest --test-dir
  build/rewrite-debug -N` (1416); `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j4`
  (exit 0, 1416 passed, 0 skipped); an inventory diff against `git show HEAD:tests/native/v5-02-ra01-b-w1-c01-evidence/inventory.txt`
  (0 removed, 72 added); `scripts/redact-local-paths.py check` on the committable files (exit 0); and the
  reproductions above, run with `HOME` set to a scratch directory. I applied no mutant and edited no checkout file
  outside my review files.

## Limits

- I did not rebuild or run RelWithDebInfo or the Linux container. For those I rely on the run 3 logs, which I read.
- I did not re-run any mutant. My probe verdicts in R2 come from reading the code and the existing fixtures, and they
  name concrete killing inputs.
- I read the tests selectively, not every line of the 4,300 new test lines.
- I did not check the D-18 operator answer against the operator's own message. I rely on DECISIONS.md and the amended
  contract text.
