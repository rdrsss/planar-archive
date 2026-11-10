# R19-AUTHORING source map

Search baseline: `d6c989ed9e196bf9b357019ea946e6ff18c47f18` on `dev/planar-v2-vernacular`.
This is a bounded search report, not implementation or approval. The prescribed Claude model is unavailable;
the coordinator's ordinary current-model substitute performed the search. No candidate binary, build, test,
installed Planar command, ledger or operator database was invoked. Only this file and the two owned plan tables
were written. Read the canonical [decisions](../../../tests/native/r19-authoring-evidence/plan/DECISIONS.md) and
[invariants](../../../tests/native/r19-authoring-evidence/plan/INVARIANTS.md) alongside this map.

## Finding and authority

The prior genuine trial failed exactly as recorded in
`tests/native/r19-cmake-evidence/driver/authoring/FAILED.json` and `003-activate.stderr`: authoring init/load
succeeded, then activate returned exit 3, `interpretation_mismatch`. The saved driver candidate and command
records identify the actual source-built authoring executable. This is prior evidence, not the new round's
required executable red regression.

`src/cmd/planar/bootstrap.cpp:580-620` creates and validates one VM, but its twelve execution-family branches
call fully qualified original handlers without the optional VM argument. `load`, `export`, `history`, `init`
and `render` pass `&*vm`. Each omitted argument defaults to null; `invocation.cpp:2291-2315` then creates a
fresh VM. In an authoring composition that fresh VM is the original production runtime, while load accepted
under the generated authoring runtime. The digests differ and EX-6 correctly refuses activation.

This was an explicit draft shortcut, DD27 (`V5-02-R15-C-DRAFT.md:253-264`), carried into the activate module's
opening comment and the command CMake comment. DD27 explicitly left generator extension to fixing rounds.
It is not authority to weaken the approved command bootstrap lifetime rule: `command-contract.md:175-189`
requires binding and executing the callable from the same validated bundle in the same VM. The defect crosses
product source and verification composition wiring; it is not merely a bad Lua fixture or a bogus EX-6 check.
Production happens to create two byte-identical default VMs, hiding the defect there.

EX-6 (`execution-contract.md:726-730`) requires exact accepted/current stdlib identity after early replay.
D148 (`:1466`) excludes authoring from production. RA-1 (`authoring-contract.md:75-76`) retains all callables
in the same bundle. D134 and RA-9 still exclude the six authoring verbs from with-profile. A repair restoring
these existing rules needs no product contract amendment. A runtime bundle switch, digest exception, new
public authoring mode, profile admission of authoring, or an interpretation exemption would be a new choice
outside this plan and must return to the operator.

## Family and type inventory

There are **twelve** execution/read CLI families, not thirteen. Including load, the acceptance-to-execution
path has thirteen families. All twelve handler interfaces declare an optional
`engine::command_vm* vm = nullptr`; all implementations pass it to `execute`. Their direct with-profile
functions call `execute_with_profile` without that embedded VM. Locations are
`src/cmd/planar/handlers/<directory>/command.{cppm,cpp}`.

| CLI family / directory | Module suffix under `planar.cmd.execution.` | Intent alternative in `runtime.cppm:69-159` | Bootstrap today |
| --- | --- | --- | --- |
| activate | activate | command_activate_options | Original handler, no VM |
| status | status | command_status_options | Original handler, no VM |
| preview | preview | command_preview_options | Original handler, no VM |
| claim | claim | command_run_options | Original handler, no VM |
| takeover | takeover | command_run_options | Original handler, no VM |
| refresh | refresh | command_recovery_options | Original handler, no VM |
| checkpoint | checkpoint | command_recovery_options | Original handler, no VM |
| release | release | command_recovery_options | Original handler, no VM |
| recover | recover | command_recover_options | Original handler, no VM |
| check | check | command_check_options | Original handler, no VM |
| check-recover | check_recover | command_check_options | Original handler, no VM |
| verdict | verdict | command_check_options | Original handler, no VM |
| load | `planar.cmd.planar.handlers.load` | command_load_options | Composition-local handler, selected VM |

There is no distinct verdict intent alternative. The operation field selects check/check-recover/verdict;
claim/takeover and refresh/checkpoint/release similarly share alternatives. The other ordinary alternatives
are render/init/history/export. Authoring options are separate and compile guarded, outside command_options.

`bootstrap.cpp:98-195` stores the twelve original option types, attaches their original CLI11 families and
counts their actual parsed metadata. Preserve this parser behavior and option lifetime. Its current `intent()`
only handles the original five families. Several later handlers keep intent construction private, so routing
all execution through `families.intent()` is not a one-line substitute and would duplicate family logic unless
properly refactored.

## Compiled composition boundary

`src/cmd/planar/CMakeLists.txt:324-455` writes source copies only when bytes change. It renames runtime module
and namespace to `planar.engine.authoring_stack` / `planar::engine::authoring_stack`, command bootstrap and
invocation modules/namespaces to `planar.cmd.authoring_stack.*`, and only the old
`planar.cmd.planar.handlers.` prefix. The runtime copy owns distinct C++ command_vm, command_options and
command_services types. Passing the generated VM into an original execution handler is ill typed; a cast is
not a repair.

The current authoring source/target lists copy render/init/history/export/load/extension only. The twelve
`planar.cmd.execution.*` imports and fully qualified `::planar::cmd::handlers::*` names escape the rename,
explaining DD27. The straightforward typed repair is to copy the execution families too, rename their module
prefix and qualified namespace references into the composition, and dispatch with the retained selected VM.
Keep namespace/module transformation ordered and reversible. Update the stale DD27 explanatory comments.
The generated handler code must remain an exact renamed source copy, with no separate semantic implementation.

The invocation rewrite protects `import planar.engine.runtime; // X07-REAL-RUNTIME` because native services
also use original runtime facilities. It then supplies explicit `using` bindings for composition-local service
and option types. Existing bindings cover service/result, render/init/history/selector/export/load and the two
authoring types. Audit unqualified vocabulary after extending composition wiring; add the seven execution
option type aliases only where actually required (activate/status/preview/run/recovery/recover/check). Do not remove the real-runtime protection or bind unrelated engine primitives to a new
owner. Dependency edges for new copied handlers must reach their local invocation and CLI11. New test copies
remain EXCLUDE_FROM_ALL and reachable from planar_tests. Product authoring macro sites stay exactly as today.

Four further active command-stack generators clone bootstrap and must remain type correct if the common
source now forwards its VM. Their path lists, ordered replacement tables, CMake family targets and bootstrap
links are a coupled change:

| Composition | Generator | Native target wiring |
| --- | --- | --- |
| six B2 variants | tests/native/v5-02-b2-support/generate.py | src/cmd/planar/CMakeLists.txt:125-178 |
| load CLI probe | tests/native/r14-d7-b-support/generate.py | same CMake file:188-238 |
| X07 variants | tests/native/v5-02-x07-support/generate.py | same CMake file:240-321 |
| X08 | tests/native/v5-02-x08-support/generate.py | tests/native/v5-02-x08-support/x08.cmake |

The B2/X07/X08/D7 generators record reverse-identical substitutions and source hashes. Preserve/retest them;
do not grandfather mismatched generated source. `v5-02-b2-support/audit-variants.py`, `r14-d7-b-support/audit.py`
and `v5-02-x08-support/validate.py` inspect actual generated byte identity. Existing hashed historical reports
remain historical. Authoring's twenty-two malformed catalog variants (`CMakeLists.txt:458-612`) clone runtime
only, not bootstrap/handlers; they are not another handler-generator family to expand.

## External profile and identity controls

`bootstrap.cpp:435-518` separately handles with-profile. Its twelve execution branches currently use original
family `*_with_profile` functions; the old five go through composition-local execute_with_profile. Whichever
typed composition strategy is selected, preserve final parsed option state, selector normalization, closed
profile owner, effective stdlib capture and byte equality with the current production profile path. Merely
renaming all qualification strings affects this branch too and needs explicit regression coverage.

`invocation.cpp:2460-2565` normalizes once, performs native format/early replay before opening the profile,
then opens a closed profile_invocation and runs exactly the admitted options. `runtime.cpp:1197` documents
that profile VMs never select authoring; `:6658-6675` owns the closed factory/run_known checks. The authoring
macro is not permission to include RA01 rows in profile VMs. Existing RA01 bootstrap tests compare authoring
and ordinary with-profile responses and require profile error precedence for an authoring tail.

Do not alter digest computation or EX-6 refusal sites. Exempt reads remain exactly D108/D142: status without
role, history, export and authoring orchestrator ledger form. Bound status --role and preview remain bound.

## Existing verification to reuse

- `src/engine/runtime/commands.t.cpp:323` replaces a callable and its later lexical binding; `:780,802` prove
  retained load routing/provenance and altered callable bytes/digest. These test the runtime seam, not the
  missing bootstrap execution VM argument.
- `tests/native/v5-02-b2-support/generate.py` and `cli.py` replace real init behavior after identical parsing.
  Add an execution-family replacement control to catch fallback to a different VM; runtime-only replacement
  tests cannot detect the bootstrap defect.
- `src/cmd/planar/activate.t.cpp:337` seeds a foreign stdlib and requires exact interpretation_mismatch.
  Earlier activate tests cover replay/body conflict and no extra event. Keep them passing.
- `bootstrap.t.cpp:3350-3465` checks production authoring refusal, fixed help, allowed macro targets/sites and
  no public injection entry. The scanner allows the macro only on stack runtime/invocation/bootstrap and
  guarded variants. Copied execution handlers need no new authoring macro definition.
- `bootstrap.t.cpp:3466-3521` compares profile behavior, including authoring tails. Source guards at `:1306`,
  `:1980-2185` check no shadow parser or unlisted diagnostics across current command files.
- `tests/native/r15-r18-c04-profile-cli.py` covers genuine profile lifecycle, embedded-vs-profile mismatch,
  profile byte changes and load replay after profile/input deletion.
- `tests/native/r17-r18-cli-draft.py` is a compact happy-path load/activate/claim/check/reviewer/verdict example.
  `r15-r18-c01-runtime.py:56-77` owns a real stopped check process/reservation and successful check-recover;
  reuse that controlled interruption pattern rather than invent a public fault hook.
- `r15-r18-c07-replay-stdout-cli.py` and `r15-r18-c08-saved-response-cli.py` pin historical response replay and
  retained output error behavior. A composition repair must preserve those boundaries.

## Proposed bounded packages and execution

All packages are sequential under the coordinator's brief. This search assigns no approval and executes none
of the commands below. New test paths/names below are proposed, not claimed to exist.

1. Verification coder: new `tests/native/r19-authoring-support/lifecycle.py`, targeted callable replacement in
   existing B2 generator/cli support, and the necessary CMake test registration only. First add a fresh-scratch
   authoring init/load/activate test against the real source-built authoring executable and record its assertion
   failure on baseline. Negative controls must independently pass on baseline where applicable. Do not accept
   compile failure or zero selected tests as the red result.
2. Implementation coder: `src/cmd/planar/bootstrap.cpp`, stale activate interface/CMake comments, command CMake,
   the four generator files and X08 CMake (plus tightly scoped generator validators if their declared input sets
   need extension). Copy/rename execution families and forward retained VM. The twelve handler implementation
   bodies need no semantic changes. Preserve new test assertion files after the red record.
3. Coordinator owns round/roadmap, real workspace Lua/spec/driver, raw evidence and current docs references;
   reviewer/prober has independent report/scratch ownership. Every stage preserves others' checkout edits.

Use the same authoring executable throughout one genuine lifecycle, with new ledger and driver-owned working,
base and scratch roots. Cover live orchestrator advice; worker claim, output observation/check, reviewer claim,
exact-target verdict and accepted status; realistic takeover/refresh/checkpoint/release/recover and stopped
check-recover scenarios may use separate fresh ledgers under the same bundle. Require actual successful
operations, not only the absence of interpretation_mismatch. Every one of the twelve selected-VM branches
must be observed via that workflow, focused tests or a structural source/composition check.

Use the production executable as a genuinely different bundle negative: fresh bound operations on an
authoring-loaded ledger must refuse without writes; the same already-saved authoring mutation request must
replay byte-identically through production before binding, while a changed body conflicts. Bare status/history
remain usable. Add a distinct-byte profile control and retain all existing profile tests. Exact root help bytes,
production seventeen-row catalog and authoring twenty-three-row catalog remain unchanged at this baseline.

Proposed focused commands after the test exists (coordinator owns serialization/build lane):

```sh
cmake --preset debug
cmake --build build/rewrite-debug --target planar planar_authoring_stack planar_command_tests
python3 tests/native/r19-authoring-support/lifecycle.py --authoring build/rewrite-debug/src/cmd/planar/planar_authoring_stack --production build/rewrite-debug/bin/planar
ctest --test-dir build/rewrite-debug --output-on-failure -R 'replacement\.(retained_cli|execution_profile_c04_regressions|execution_cli_workflow|execution_runtime_regressions)|RA01-B1|activate refuses'
```

Before final review, run the registered new lifecycle test by exact name and inventory its nonzero match,
then the Debug and Release complete gate (`make test-all`, `make test-all PRESET=release`), source/format/
Doxygen and no-work repeat as defined in docs/testing.md. Record actual counts, zero skips, command output
and any host limitation. Do not run builds concurrently in one directory. Broad gates run after code is
complete, not after every copied-file edit. Native Linux is a limitation unless separately run. Full R19 quota
and other follow-ups are not closed by this bounded repair.
