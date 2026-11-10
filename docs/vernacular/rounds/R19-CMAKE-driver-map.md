# R19 CMake task: external-driver map

Read-only mapping, 2026-11-09, baseline `58128234`. The operator has now named
the real project and task: split the Planar engine's CMake declarations into a
`CMakeLists.txt` in every engine directory, composed by the engine's top-level
file. This supplies the real subject that was previously unselected. It does
not by itself establish that R19's complete acceptance exercise has run.

The prescribed Claude search model is unavailable in this harness. An ordinary
current agent performed this bounded mapping instead. No product, workspace,
ledger, registry or build was changed or executed by this search; its only
authored file is this report. The checkout is shared and other edits are retained.

## Authority and scope

| Choice | Disposition and source |
| --- | --- |
| Real subject | Operator-selected Planar repository and actual CMake task; Q17 A remains quota in both groupings plus one real spec (`remaining-steps-design.md`, Q17 and its recorded decision). |
| New source-built candidate only | Explicit `build/rewrite-debug/bin/planar`; no installed executable or old database (`AGENTS.md`, Database safety; `cli-reference.md`, opening). |
| Driver manages state and source views | Logical source root is the actual working `src/engine` subtree; immutable base is its HEAD snapshot, with isolated ledger/check-scratch outside both roots. Planar does not create a checkout (`planar-design-spec-v5.md`, Scope; `execution-contract-r17.md`, R17-3/R17-4). |
| Reference orchestrator | Use the source-built authoring-stack executable, not the production binary (`src/cmd/planar/CMakeLists.txt:324`, `:447`; `cli-reference.md`, Draft execution workflow). |
| Trusted real checks | Accepted registry commands must actually verify the selected source/build, and write a truthful result document. Do not copy the smoke fixture's constant success command as evidence of the CMake task (`execution-contract-r17.md`, R17-3). |
| Approval | Independent reviewer decides the exact observed outputs. Reviewer-role verdicts are available; authenticated human approval is not (`execution-contract-r18.md`; acceptance row A24). |

The repository prohibition on **old** Planar tools/skills still applies. Running
the new explicit candidate for the operator's requested actual task is a product
exercise, not invoking the legacy orchestration system. No old Planar skills or
agent roles are needed.

## Concrete proven lifecycle and schemas

`tests/native/r17-r18-cli-draft.py` is the smallest concrete public-CLI example:
it writes declarations, initializes format3 state, loads/activates a job, claims
a worker, checks outputs, claims an independent reviewer, submits the exact
verdict vector, retries saved mutations and inspects derived acceptance.
`tests/native/r15-r18-c04-profile-cli.py` provides a larger separate example of
accepted declaration export/history and replay. Those fixtures are examples of
syntax, not substitute evidence for this task.

The minimal declaration forms are:

```lua
-- workspace.lua
local p = planar
local L = p.list
return p.workspace {
  version = 1, id = "engine_cmake",
  root = { id = "source", base = "baseline_58128234", excluded = L {} },
  spec = "spec.lua",
  jobs = L { { id = "combined", file = "combined.lua" } },
  registry = { id = "engine_cmake_tools", revision = "v1" },
  toolchains = L { "python" }, checks = L { "cmake_verification" }
}
```

```lua
-- spec.lua: simplest real-task version; expand exact acceptance text/checks.
local p = planar
local L = p.list
return p.spec {
  version = 1, id = "engine_cmake",
  goal = p.md "Compose engine targets through directory-level CMake files.",
  work = L {
    { id = "engine_build", summary = p.md "Split engine CMake declarations.",
      paths = L {
        "CMakeLists.txt", "acceptance/CMakeLists.txt", "budget/CMakeLists.txt",
        "canonical/CMakeLists.txt", "capture/CMakeLists.txt",
        "execution/CMakeLists.txt", "export/CMakeLists.txt", "hash/CMakeLists.txt",
        "history/CMakeLists.txt", "json_codec/CMakeLists.txt", "ledger/CMakeLists.txt",
        "lua/CMakeLists.txt", "process/CMakeLists.txt", "runtime/CMakeLists.txt",
        "runtime/closure_guard/CMakeLists.txt", "value/CMakeLists.txt"
      }, checks = L { "cmake_verification" } }
  },
  milestones = L {
    { id = "complete", work = L { "engine_build" }, acceptance = L {
      { id = "preserved", text = p.md "Every engine directory owns its CMake declarations; target membership, dependencies, tests, generated files and build behavior are preserved." }
    } }
  },
  constraints = L {}, prescriptions = L {}, contracts = L {},
  facets = L {}, dependencies = L {}, frozen = L {}
}
```

```lua
-- combined.lua
local p = planar
local L = p.list
return p.job {
  version = 1, id = "combined", shape = "fanout_review", budget_bytes = 131072,
  roles = L {
    { id = "builder", kind = "worker", work = L { "engine_build" } },
    { id = "reviewer", kind = "reviewer", targets = L { "engine_build" } }
  }
}
```

These are unexecuted examples, not a reviewed final spec. The 16 explicit
boundaries are relative to the actual `src/engine` logical root; they permit
CMake edits and no C++ edits. The real check set must be derived from the CMake
source map. Coordinator reference/planning documents remain outside this
engine-root trial and need separate review; do not count them as engine outputs.
A meaningful real-task combined/split trial can
use two non-overlapping units: exact child-directory CMake files, and the exact
engine-root composition file. The existing quota
`workspace/{spec,combined,split}.lua` demonstrates meaningful two-unit grouping,
facets and complete-phase dependencies. Merely renaming an identical one-unit
job is not evidence of different grouping behavior.

Registry shape: `{version:1,id,revision,toolchains:[{id,description,command,
environment:[{name,value}]}],checks:[{id,toolchain,argv,coverage,inputs,
timeout_ms,max_output_bytes,selection}]}`. For the real check, an absolute Python
executable plus an absolute reviewed adapter path is straightforward. Set
`inputs:["own_output"]`, `selection:"nonempty"`, truthful coverage, sufficient
bounded time/output and the explicit environment required by CMake/CTest.
The adapter must run the actual graph/build/test checks and write
`{"version":1,"matched":N,"passed":P,"failed":F}` to the reserved
`PLANAR_CHECK_RESULT`; report nonzero N, actual failures and actual subprocess
status. R17 permits at most 3,600,000 ms and 16,777,216 child output bytes.

## Driver sequence

All paths below are placeholders supplied by the external driver. `BIN` means
the absolute path of this checkout's source-built `src/cmd/planar/planar_authoring_stack`
for a consistently selected live-orchestrator trial; `AUTH` is the same binary.
The production `bin/planar` can drive a separate consistently production-only
trial, with no `orchestrator` verb. `STATE`, `WORKSPACE`, `CURRENT`, `BASE`
and `CHECK_SCRATCH` are driver-created isolated directories. Persist each stdout,
stderr and exit status, exact argv, candidate binary hash, accepted input hashes
and the driver-owned baseline identity. Never manually edit the ledger.

1. Prepare the real task declarations and reviewed trusted registry. Keep the
   workspace/state outside the checked source view. Bind `CURRENT` to the actual
   repository's `src/engine`; preserve that subtree at `58128234` in an immutable
   ordinary-file `BASE` snapshot before implementation edits. Freeze the exact
   16 owned CMake paths and hashes. `CHECK_SCRATCH` is a physically separate
   sibling driver directory, not inside either source tree.
2. Invoke `BIN init --ledger STATE/ledger.db --workspace-id engine_cmake`.
3. Invoke `BIN load --ledger STATE/ledger.db --workspace WORKSPACE/workspace.lua
   --registry WORKSPACE/registry.json --expect-revision none --request-id
   cmake.load.1 --actor driver`. Read `revision` from the actual saved response.
4. Invoke `BIN activate --ledger STATE/ledger.db --job combined
   --expect-revision REVISION --expect-job NONE --request-id cmake.activate.1
   --actor driver`. The initial expected-job sentinel is uppercase `NONE`.
5. Use `AUTH orchestrator --ledger STATE/ledger.db`, `BIN status` and
   `preview --role builder` to inspect available actions and accepted scope.
   Also retain actual `render` packets if proving database-free preview.
6. Invoke `BIN claim --ledger STATE/ledger.db --job combined --role builder
   --request-id cmake.claim.1 --actor builder --provenance PROVENANCE.json`.
   Retain the actual `run.id`, `run.token`, `dispatch.work_fp` and packet.
   The worker should receive and use this retained dispatch before doing work.
7. Drive implementation and verification externally. Checkpoint through
   `BIN checkpoint --ledger ... --run TOKEN --observations NOTES.json
   --request-id cmake.checkpoint.1 --actor builder`. Notes shape is
   `{version:1,checks:[],findings:[],checkpoint:{progress,next,unresolved,files:[]}}`
   (see `r15-r18-c08-saved-response-cli.py:19`). Notes do not complete work.
8. In a fresh CLI process, read `recover --ledger ... --role builder --root
   source=CURRENT --base-root source=BASE`, plus `history --request-id` or
   `history --event` for retained facts. This demonstrates process restart;
   it is not a crash/fault matrix by itself. `recover` does not acknowledge drift;
   use `refresh --run TOKEN --expect-fp EXPECTED_W ...` when actual policy requires it.
9. Invoke `BIN check --ledger STATE/ledger.db --run TOKEN --root CURRENT
   --base-root BASE --scratch CHECK_SCRATCH --request-id cmake.check.1
   --actor builder --provenance PROVENANCE.json`. Require `outcome:"completed"`,
   applicable passing real evidence, nonzero matched counts and worker `done`
   from `status`. Command exit 0 alone also permits a recorded failing result.
10. Retry the exact same check intent with the same request ID and verify saved
    response byte equality without re-running the adapter. A check-owned external
    invocation marker can prove no second execution. Preserve all attempts.
11. Have a fresh independent reviewer inspect exact outputs and raw check results,
    then claim `reviewer`. Build the verdict target from the actual completed
    result: `work:"engine_build"`, `output:check.observation.units[*].output`,
    `work_fp:check.completion.work_fp`; take `policy` from the reviewer dispatch's
    `work_fp`. The document is
    `{version:1,targets:[{work,output,work_fp}],policy,decision:"approve",
    findings:[],repair:[],summary:"..."}`. Submit through `BIN verdict --ledger
    ... --run REVIEWER_TOKEN --verdict VERDICT.json --request-id cmake.verdict.1
    --actor reviewer`. Construct the verdict only after actual approval; never
    pre-author a success to force the workflow through.
12. Read actual status review acceptance, history expanded events, revision
    metadata, retained dispatch/check/verdict references, and exported accepted
    declarations. A repeat verdict request must return the saved result.

Source-built authoring targets are `EXCLUDE_FROM_ALL` but included in
`planar_tests`; build only the necessary target if it is absent. The authoring
stack and production runtime use separate interpreted bundle identities, so do
not assume that switching binaries on an existing production-initialized ledger
is transparent. The safe initial workflow choice is a consistently selected
authoring bundle for the whole demonstration when live `orchestrator` is required;
prove compatibility before mixing entry points. The source map should inspect
`src/cmd/planar/CMakeLists.txt:324–455` and the live orchestrator tests before
settling the executable used for a durable trial.

## Capture and build hazards

- `CURRENT`, `BASE`, and `CHECK_SCRATCH` must be physically disjoint, with no
  ancestor overlap or aliases (`execution-contract-r17.md`, R17-3).
- Tree capture scans the whole source view, not just owned paths. It implicitly
  excludes `.git/`, otherwise uses only declared exclusions; untracked and
  ignored files are included. Symlinks/special files/hardlink aliases refuse.
  The chosen actual engine subtree contains none of the repository-root
  instruction symlinks, build trees or bulky evidence, so it avoids those capture
  hazards without a filtered imitation of the changed source. The same engine
  subtree membership is used for the immutable baseline. If the root is later
  widened, explicit deliberate exclusions or driver-owned ordinary-file views
  become necessary; never silently dereference instruction links or copy data.
- A check's cwd is `CURRENT`; its environment is **exactly** accepted registry
  values plus `PLANAR_CHECK`, `PLANAR_WORK` and `PLANAR_CHECK_RESULT`. Neither
  `PATH` nor `HOME` is inherited. Register the toolchain environment explicitly.
- The trusted adapter invokes the actual repository's build gates through
  absolute paths while its cwd remains the bound engine subtree. It supplies
  explicit scratch HOME and trusted PATH, stores gate logs outside the engine,
  and verifies owned source hashes before/after. Existing repository build trees
  are outside the captured engine root; use one serialized runner per tree.
- Real graph comparison must evaluate the actual changed source path, not a
  stale external build configured against another source root. The engine root
  is the working tree itself in this trial, not a filtered source copy.
- R17's built-in `planar.boundary_v1` compares entire engine base/current
  differences against the 16 declared CMake boundaries. Any C++ modification is
  outside and fails. Planning/evidence/reference changes outside this logical
  root remain coordinator-owned and separately reviewed rather than silently
  counted as checked engine outputs.
- The quota registry currently launches unittest directly and does not write
  the R17 result document. Its rendering fixture is not a ready-made real
  execution registry; add a truthful result adapter before executing its checks.

## What remains beyond a successful CMake demonstration

The operator task supplies a genuine subject and can establish real load,
dispatch, work, check and independent verdict usage. Full R19 additionally
requires the quota fixture in **both** groupings (Q17 A), usable reference
orchestrator guidance against live APIs, retained references through restart and
faults, an explicit disposition for every applicable Render/Execution matrix row,
conditional A24 applicability, and minimal A34 trial/baseline provenance
(`roadmap.md:419–433`; `acceptance-matrix.md`, A24/A30/A33/A34).

Use the exact `driver_provenance_v1` shape in
`execution-contract.md:1106–1120`: all keys are required; unknown leaves are
`{"status":"unknown"}`, not fabricated model/session metadata. Record actual
driver/trial/view IDs, model facts known by the driver, baseline hash, accepted
revision and candidate identity. These are unverified driver assertions plus
native derived provenance, not controlled model/grouping experiments.

A24 should be recorded as authenticated-human-policy **not applicable** for the
reviewer-role policy used here; unsupported authority refuses rather than counts
as a successful human approval. Existing targeted fault/negative evidence may
carry only within its exact independently reviewed scope; a normal CMake task
cannot replace the required fault matrix or full gates. Workspace composition
(A21), controlled A34 comparisons, model calls, scheduling and worktree management
remain deferred/outside the product. Do not mark R19 complete solely from the
happy-path task run.

Coordinator path check: the baseline CMake File API resolves the existing
authoring-stack executable to `build/rewrite-debug/src/cmd/planar/planar_authoring_stack`;
only the production executable has the explicit `bin/` output directory.

Coordinator preflight condition: one authoring-stack executable still dispatches
some execution handlers through the original production invocation module
(`bootstrap.cpp`, execution dispatch; `handlers/claim/command.cpp`). It does not
prove a shared runtime bundle. Actual fresh-state initialization, accepted load,
activation, claim, orchestrator and recovery must succeed before implementation.
Preserve any compatibility refusal and record any production-only fallback and
orchestrator coverage gap explicitly; no product fix or R19 acceptance is implied.
