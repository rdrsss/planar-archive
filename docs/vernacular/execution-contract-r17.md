# Execution contract R17: checks, boundaries and observed completion

Status: unreviewed draft contract (draft mode, operator decision 2026-11-08); binding for draft lanes, reviewed in the fixing rounds.

Scope: round R17-A of the [execution design](execution-design.md#r17-checks-boundaries-and-observed-completion):
R17-1 to R17-8, the outcomes of E7 (C plus A's result file), E8 (A) and E15 (A), Q15 A isolation, and the
fenced-completion choice. It extends [execution-contract.md](execution-contract.md) (parts 1 and 2, approved), cited
here as EC; it is written against the R16 draft contract and binds draft lanes R17-B, R17-C and R17-D. Where it
amends an approved text it says so in [R17-12](#r17-12-amendments). Draft decisions are tagged **DR17-n** and
collected in [R17-13](#r17-13-draft-decisions).

## R17-0 Summary of the data flow

```
check --run TOKEN --root R --base-root B --scratch S
  prelude/replay ─ snapshot ─ plan (Lua) ─ capture R and B (native, digest-only) ─ contract delivery + vector precheck
  ─ RESERVE commit (check_reserved) ─ run checks (process runner, no lock) ─ boundary gate ─ recapture R
  ─ evaluate (Lua) ─ COMPLETE commit (check_recorded: evidence [+ completion, publications, transitions] + request row)
  └ on state_conflict: re-snapshot + re-evaluate, at most 3 retries, then evidence only
check-recover --run-id N --reservation M --confirm-stopped true
  ─ native-only commit (check_interrupted + two request rows)
```

Fenced-completion choice (R17-1 step 3 "Open"): **option (a)**. If the run is no longer current at the complete
commit, `check` refuses `fenced`, writes nothing, and the reservation stays open until `check-recover`. No evidence
is ever attached to a fenced run; no amendment of v5 `:59-60`, `:370` is needed (DR17-1).

## R17-1 Format3 use

No SQL changes. R17 fills the tables labelled `Reserved for R17` in EC EX-1 exactly as stored, plus `events`,
`requests`, `role_state`, `run_state` and `transitions`.

| Table | Written by | Column meaning |
| --- | --- | --- |
| `reservations` | reserve commit | `request_id`: the check request ID; `run_id`: the token's run; `opened_seq`: the `check_reserved` seq; `body_id`: the `request_v3` body record of the check request (kind 4, codec 3) |
| `reservation_state` | reserve (`open`), complete (`completed`), check-recover (`interrupted`) | `last_seq` is the seq of the event that set `state` |
| `evidence` | complete commit (both modes) | one row per (work, check) of the plan plus one `planar.boundary_v1` row per unit of the role (R17-5); `seq` is the `check_recorded` seq; `definition_id` → `check_definition_v1` value record; `vector_id` → `output_vector_v1` value record; `output_id` → file record (mode 420) of the retained output, NULL when 0 bytes were retained; `output_bytes` the total bytes the child wrote (not the retained count); `output_truncated` 1 when retained < written |
| `completions` | complete commit, completion mode only | `dispatch_id`: the run's acknowledged dispatch at `opened_seq` (the run's latest `dispatches` row with `seq < opened_seq`); `work_fp`: that dispatch's `work_fp`; `observation_id` → `check_observation_v1` value record |
| `publications` | complete commit, completion mode only | one row per declared facet whose producer unit is a unit of the role and whose every declared path exists in the verified observation; `semantic` = facet semantic identity (`render-contract.md` "Facet semantic identity"), raw 16 bytes; `payload_id` → directory record of mode-normalized file records of the facet paths (exact bytes) |

The acknowledged W of a reservation is derived, never stored twice: the run's latest dispatch before `opened_seq`.
R16 `refresh` therefore refuses `check_in_progress` while the run has an open reservation (amendment AM-3), so W
cannot move under a reservation of the same run.

`evidence.reason` closed set (completes EC C6; native-writer enforced, reader-verified):

| outcome | reason values |
| --- | --- |
| `pass` | `passed` |
| `fail` | `exit_status`, `zero_selected`, `missing_executable`, `timeout`, `output_limit`, `malformed_result`, `boundary` |
| `inconclusive` | `input_changed` |

`matched` is the result-file `matched` for `selection:"nonempty"` checks with a well-formed result file, else NULL.
`exit_status` is the exit code for a normal exit, the negated signal number for a signal termination, and NULL when
no process was spawned (`missing_executable`) or for a `planar.boundary_v1` row (DR17-2).

Events (fills the EC EX-4 rows labelled R17; `version` 2; payload `operation` is `check` or `check-recover`;
`before` and `after` are equal because these events never change the active revision or job):

| Event | `request_id` | job / role / generation / run | `references` keys |
| --- | --- | --- | --- |
| `check_reserved` | the check request ID | the run's job, role, generation id, run id | `reservation`, `run`, `dispatch` (acknowledged dispatch), and `provenance` only when `--provenance` was given |
| `check_recorded` | the check request ID | same as its reservation | `reservation`, `observation`, and `completion` only in completion mode |
| `check_interrupted` | the check-recover request ID | same as the reservation | `reservation` |

Requests: the check request row is written by the complete commit (outcome `recorded`, `event_seq` = its
`check_reserved` seq, `revision_id` = active revision at reserve, `result` = the saved `check_v1`), or by
`check-recover` (outcome `interrupted`, same `event_seq`). The check-recover request row has operation
`check-recover`, outcome `interrupted`, `event_seq` = the `check_interrupted` seq. `attachment_id` is NULL for both.
EC invariants 1 and 7 hold as written.

Fact versions: `check_recorded` is a bumping event type (EC EX-3 replay rule). Native increments `fact_version` by 1
for every role with a `transitions` row in a `check_recorded` commit, in both modes; `check_reserved` and
`check_interrupted` bump nothing and write no transitions.

Records (new value-record codec IDs in `planar.engine.history.execution_codec`, canonical_v1 values):

- `check_definition_v1`: `{version:1,work,check,gate,registry:{id,revision},toolchain,command,environment,reserved,cwd:{root},timeout_ms,max_output_bytes,selection,inputs,coverage}`.
  For a registry check: `gate` is `"registry"`, `command` is the resolved argv (absolute executable then the
  remaining toolchain `command` elements then `argv`), `environment` the sorted registry variable names, `reserved`
  the sorted reserved names passed (R17-3). For the gate: `gate:"boundary_v1"`, `check:"planar.boundary_v1"`,
  `registry`, `toolchain`, `command`, `environment`, `reserved`, `timeout_ms`, `max_output_bytes`, `selection`,
  `coverage` absent; `inputs` is `["own_output","output_vector"]`; plus `owned`, `frozen`, `excluded` (sorted
  boundaries) and `producers` (sorted work IDs).
- `output_vector_v1`: `{version:1,targets:[{work,output}]}` sorted by work; `output` is the `xxh3_128:` unit-output
  text. Without `output_vector` input it holds exactly the own target; with it, own plus every complete/facet
  dependency producer unit (`declarations.md` registry paragraph).
- `check_observation_v1`: `{version:1,root,base,observation,units,contracts}`: `root` source ID; `base` the base
  identity text; `observation` the observation identity text (`render-contract.md`); `units` sorted
  `[{work,output}]` for the role's own units and every vector producer unit; `contracts` sorted
  `[{id,path,digest}]` of delivered contracts (R17-4). No tree entries are retained (E15 A).
- `boundary_diff_v1`: `{version:1,changes:[{path,change,class}]}` sorted by path; `change` ∈ `added`, `deleted`,
  `modified`, `mode`; `class` ∈ `own`, `producer`, `frozen`, `outside`. Retained as the `output_id` of each gate row
  only when the gate fails (DR17-3).

## R17-2 Process runner (native, `src/engine/process/`)

Module `planar.engine.process.runner` (`runner.cppm`, `runner.cpp`, `runner.t.cpp`) and
`planar.engine.process.resolve` (`resolve.cppm`, `resolve.cpp`, `resolve.t.cpp`). Depends on `std` only (plus the
platform C API). No Lua, no ledger, no capture, no global mutable state; each call is thread-confined.

```cpp
namespace planar::engine::process {
enum class process_error_code { spawn_io, pipe_io, wait_io, resource_limit };
struct process_error { process_error_code code; int sys_errno = 0; };

struct spawn_spec {
  std::string                                      executable;      // absolute path from resolve_executable
  std::vector<std::string>                         argv;            // argv[0] included
  std::vector<std::pair<std::string, std::string>> environment;     // exact, sorted by name, unique
  int                                              cwd_fd = -1;     // open directory descriptor of the bound root
  std::uint64_t                                    timeout_ms = 0;  // 1..3600000
  std::uint64_t                                    max_output_bytes = 0; // 1..16777216
  std::uint64_t                                    retain_bytes = 0;     // <= max_output_bytes (R17-9 per-invocation cap)
  std::uint64_t                                    kill_grace_ms = 2000; // product always 2000; tests may lower it
};
enum class termination { exited, signaled, timed_out, output_limit, not_spawned };
struct run_result {
  termination   how;
  int           exit_code = 0;     // exited only
  int           signal = 0;        // signaled, timed_out, output_limit: the signal that ended the leader
  int           spawn_errno = 0;   // not_spawned only
  std::string   output;            // first retained bytes of merged stdout+stderr
  std::uint64_t output_bytes = 0;  // total bytes read (stops counting at max_output_bytes + 1)
  bool          truncated = false; // output.size() < output_bytes
  std::uint64_t duration_ms = 0;   // monotonic, spawn to leader reap
};
std::expected<run_result, process_error> run(const spawn_spec& spec);

enum class resolve_outcome { found, missing };
std::expected<std::string, resolve_outcome>
resolve_executable(std::string_view command0, const std::vector<std::pair<std::string, std::string>>& environment);
}
```

Rules (each is a test obligation of R17-B):

1. **Resolution.** `command0` containing `/` must be absolute and name a regular file with `X_OK` (`faccessat`
   with `AT_EACCESS`), else `missing`. Otherwise search the `PATH` value of `environment` only (never the host
   environment): split on `:`, skip empty and relative entries, first regular executable file wins. No `PATH`
   variable → `missing`. The resolved absolute path is passed to `posix_spawn` (never `posix_spawnp`).
2. **Spawn.** `posix_spawn` with `POSIX_SPAWN_SETPGROUP` (pgid 0: the child leads a new group),
   `POSIX_SPAWN_SETSIGMASK` (empty mask), `POSIX_SPAWN_SETSIGDEF` (default disposition for every catchable
   signal); file actions: `addfchdir_np(cwd_fd)` (macOS; glibc ≥ 2.29 on Linux), stdin from `/dev/null`
   (`O_RDONLY`), stdout and stderr both `dup2` of one pipe write end (merged in write order). macOS adds
   `POSIX_SPAWN_CLOEXEC_DEFAULT`; Linux relies on every Planar descriptor being `O_CLOEXEC` (the runner opens its
   own descriptors `O_CLOEXEC`). A spawn errno of `ENOENT`, `EACCES`, `ENOEXEC` or `ENOTDIR` yields
   `termination::not_spawned` (a recorded outcome, not an error); any other spawn failure is `spawn_io`.
3. **Environment.** Exactly `spec.environment`; nothing is inherited (Q15 A, `declarations.md` registry).
4. **Timeout.** Deadline = spawn time + `timeout_ms` (steady clock). At the deadline: `kill(-pgid, SIGTERM)`;
   if the leader is not reaped within `kill_grace_ms`, `kill(-pgid, SIGKILL)`. Result `timed_out`.
5. **Output cap.** Read the pipe with `poll`. Retain the first `retain_bytes`. When the bytes read exceed
   `max_output_bytes`, stop retaining, send the group SIGTERM then SIGKILL after the grace, result `output_limit`.
   `output_bytes` saturates at `max_output_bytes + 1`.
6. **Reap and drain.** After the leader is reaped, `kill(-pgid, SIGKILL)` any remaining group member, then drain the
   pipe until EOF or for at most `kill_grace_ms`, whichever is first. A descendant that called `setsid` or
   `setpgid` escapes the group and is not tracked or killed; if it holds the pipe open the drain ends at the grace
   (recorded Q15 A limit).
7. **No sandbox.** No filesystem, network or privilege isolation (Q15 A). Every reference document that mentions
   checks states: "Checks run trusted registry commands with bounded time, output, cwd and environment and no OS
   sandbox; escaping descendants are not tracked."
8. `SIGPIPE` in Planar itself is ignored for the duration of `run` via the pipe's read side only (Planar never
   writes to the child); `EINTR` is retried.

## R17-3 Check execution, reserved environment and result file (E7 C + A, Q15 A)

Per planned check (R17-6 order), the engine module `planar.engine.execution.check` builds a `spawn_spec`:

- `argv` = toolchain `command` then check `argv`, from the **accepted** registry records of the active revision.
- `cwd_fd` = the descriptor of the bound `--root` directory opened at capture (`O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC`).
- `environment` = toolchain `environment` plus the reserved variables, sorted by name:
  `PLANAR_CHECK` (check ID), `PLANAR_WORK` (work ID), `PLANAR_CHECK_RESULT` (absolute path of the result file).
  The prefix `PLANAR_` is reserved: a registry variable whose name starts with `PLANAR_` refuses the whole check
  before reserve with `reference`, message `Registry environment uses a reserved name`, field `registry` (DR17-4).
- Scratch: per check Planar creates `<scratch>/planar-check.<reservation>.<ordinal>` with `mkdirat(…, 0700)`
  (`EEXIST` → `io`, message `Cannot create check scratch directory`); the result path is `result.json` inside it.
  After the child is reaped and the result is read, Planar unlinks `result.json` if it is a regular file owned by
  the effective UID and then `rmdir`s the directory; if `rmdir` fails the directory is left and nothing else is
  deleted (recorded limit, not a failure). `--scratch` must not equal, contain, or be inside `--root` or
  `--base-root` after lexical normalization and `realpath` of the directory descriptors; otherwise `path_denied`,
  message `Scratch directory overlaps a checked root` (before reserve).
- Result file (`selection:"nonempty"` only; `not_applicable` checks ignore it): opened `O_NOFOLLOW`, regular, at
  most 65536 bytes (`check_result_bytes`), Glaze strict JSON `{"version":1,"matched":N,"passed":N,"failed":N}`
  with non-negative integers at most 2^53−1, no unknown, duplicate or missing keys, no trailing bytes. Any
  violation, absence, or `passed + failed > matched`, or `failed > 0` with exit 0, is `malformed_result`.

Outcome rule, first match wins (native, `planar.engine.execution.check::classify`):

1. `not_spawned` or resolution `missing` → `fail`/`missing_executable`.
2. `timed_out` → `fail`/`timeout`.
3. `output_limit` → `fail`/`output_limit` (first bytes retained).
4. `signaled`, or `exited` with code ≠ 0 → `fail`/`exit_status`.
5. `nonempty` and the result file is malformed → `fail`/`malformed_result`; `nonempty` and `matched == 0` →
   `fail`/`zero_selected`.
6. Otherwise `pass`/`passed`.
7. After recapture (R17-5): if the recapture differs, every evidence row of the invocation, including the gate rows,
   becomes `inconclusive`/`input_changed`, keeping its `matched`, `exit_status`, duration and output.

The complete required check set always runs; a failure does not stop later checks.

## R17-4 Capture, contract delivery and the vector precheck (E15 A)

New module `planar.engine.capture.tree_digest` (`src/engine/capture/tree_digest.{cppm,cpp}`), reusing the
refusals of `capture/native` (symlinks, special files, hardlink aliases, case aliases, traversal: existing codes and
messages):

```cpp
namespace planar::engine::capture {
struct tree_digest_spec {
  std::vector<std::string> excluded;      // root.excluded boundaries; ".git/" always excluded
  std::vector<std::string> retain_paths;  // exact bytes retained: facet paths of the role's units, contract paths
  std::uint64_t max_hashed_bytes = 1073741824, max_files = 100000, max_depth = 64;
  std::uint64_t max_retained_file_bytes = 1048576; // facet_file_bytes; per-facet 4 MiB checked by the caller
};
struct tree_digest_entry { std::string path; std::string mode; std::uint64_t size; std::array<std::byte,16> digest; };
struct tree_digest { std::vector<tree_digest_entry> entries; std::map<std::string, std::string> retained; };
std::expected<tree_digest, capture_error> capture_tree_digest(int dir_fd, const tree_digest_spec&);
struct tree_change { std::string path; enum class kind { added, deleted, modified, mode } change; };
std::vector<tree_change> diff_trees(const tree_digest& base, const tree_digest& current); // sorted by path
}
```

Entries sort by path bytes; `digest` is `file_bytes_v1` of the exact bytes (hashed while read, not retained except
`retain_paths`); mode, kinds and enumeration follow `render-contract.md` capture. Each file is read through a stable
descriptor with before/after `fstat` (dev, ino, size, mtime); a change is `capture_changed`, message
`File changed during read`. Limits refuse `resource_limit` with `Execution resource limit exceeded` and the EC
EX-8 names `check_observation_bytes`, `check_observation_files`, `check_observation_depth`.

Unit outputs, base identity and observation identity are the `render-contract.md` recipes over the tree value
built from these entries (`content` = digest); they are byte-identical to render's identities for the same tree.

Before reserve (all refusals here leave no reservation, DR17-5):

1. Capture `--base-root` and `--root` (the same spec; `retain_paths` only for the root).
2. **Contract delivery (E7 C, v5 `:497`).** For every planned check with `expected_contracts`, every contract the
   unit produces or consumes must be present in the root at the workspace path of its `expected` text, with
   bytes equal to the accepted expected bytes (exact comparison of retained bytes). A missing file, unequal bytes,
   or an inline `expected` text (no file path) refuses `revision_conflict` with its existing message and
   `details:{expected:<active revision id>,actual:"working_bytes"}` (DR17-6).
3. **Vector precheck.** For every planned check with `output_vector`, for every producer unit in its vector whose
   owning role (current lineage) has a current completion (R17-7), the unit output computed from this capture must
   equal that completion's recorded output, else `observation_target_mismatch`, existing message `Wrong output
   vector`, `details:{work:<producer work ID>}` (first mismatch in work order). A producer without a completion
   is not prechecked; its observed output enters the vector and completion is gated (R17-6).

Recapture (after the checks, outside any lock): `--root` again with the same spec. It *matches* when entry
membership, modes, sizes and digests are equal (E15 A: digest-only; change-and-revert and digest collisions are not
detected — recorded limit). A recapture refusal (`capture_changed`, `resource_limit`, a new symlink or special file)
counts as *differs* and is not a refusal of the command. Facet payload bytes come only from the recapture's
retained bytes, and only when the recapture matches.

## R17-5 Boundary gate `boundary_v1`

Computed natively in `planar.engine.execution.check::boundary_gate(plan, base, current)` from `diff_trees(base,
current)` (after `root.excluded`); Lua supplies policy only in the plan. Each change is classified, first match:
`frozen` if inside any `frozen` boundary of the spec; `own` if inside a boundary of the role's units; `producer` if
inside a boundary of a vector producer unit whose current output equals its current completion output; else
`outside`. The gate passes iff no change is `frozen` or `outside`. Renames are delete plus add; symlinks, special
files and aliases already refuse at capture; untracked and ignored files are included unless excluded. One evidence
row per unit of the role, `check` = `planar.boundary_v1` (`.` is outside the declaration ID grammar, so it never
collides), the same outcome for every unit, `vector_id` the own-unit target. The gate rows are part of the required
set: a failed gate withholds completion.

## R17-6 Check protocol and commits

Verb `check` (handler `shipped.check.v1`, response `check_v1`, capabilities `execution_commit_v1`,
`provenance_read_v1`, `check_run_v1`). Services, in this order (amends the "at most once" rule, AM-2):

| Step | Service | Notes |
| --- | --- | --- |
| 1 | `read_execution({})` | snapshot S0 |
| 2 | `read_provenance({provenance:SLOT})` | only with `--provenance` |
| 3 | `prepare_check({plan:P})` | native: open root/base/scratch slots, capture, delivery, precheck; returns `{observation, units, producers, contracts}` |
| 4 | `reserve_check({expected:E1})` | reserve commit; returns `{reservation, event}` |
| 5 | `run_checks({})` | runner over the plan, gate, recapture; returns `{evidence, recapture:"matched"\|"differs", observation, facets}` |
| 6 | `read_execution({})` then `commit_check({change:C})` | at most 4 pairs: the first plus 3 retries after `state_conflict` |

Lua (`src/stdlib/planar/checks.lua`) builds `P = check_plan_v1` from S0 and the token's role:
`{version:1,job,role,run,generation,units,checks:[{work,check,toolchain,command,argv,environment,timeout_ms,max_output_bytes,selection,inputs,coverage,vector}],contracts:[{id,work,path,bytes_digest}],facets:[{work,facet,paths}],owned,frozen,excluded,producers:[{work,paths,completion_output}]}`.
`checks` is the role's effective required checks per unit (W `effective_checks`), sorted by `(work, check)`; a check
required by two units runs once per unit. Native validates `P` against S0's recorded interpretation and registry
(the definitions must equal the accepted records; mismatch → `internal`, existing message).

**Reserve** (EX-3 commit). Precondition set, exact (enforced as EC D151): `request_unused(check id)`,
`active_revision == R`, `active_job == J`, `role_state(target) == (running, 0, run)`, `generation_current(target, G)`,
`run_active(run, G, secret)`, plus the native lock-time guard "no open reservation for the run" (AM-1, the R16 AR-2
form; not an `expected` member). Writes `reservations`, `reservation_state`
(`open`), `check_reserved`; no request row, no transitions. Mapping under the lock: `run_active` failed → `fenced`
`{run_state}`; `role_state` failed with locked drifted 1 → `drifted`; the guard failed →
`check_in_progress`; anything else → `state_conflict`.

**Complete.** Lua `checks.evaluate(snapshot, plan, run_result)` returns `C = execution_change_v1` with
`operation:"check"`, `facts:{reservation,evidence,observation,completion?,blockers_cleared}` (`blockers_cleared`
is R18's `[id]` list, DR18-8; `[]` until R18), `vector`, `transitions`, `expected`,
and `withheld` (sorted subset of `check_failed`, `boundary`, `input_changed`, `drifted`, `start_prerequisite`,
`complete_prerequisite`, `retry_exhausted`). `completion` is present iff `withheld` is empty, i.e. all of:

1. every registry evidence row is `pass` and every gate row is `pass`;
2. the recapture matched;
3. the role is `running` with `drifted` 0 and the run is current at this snapshot;
4. no start prerequisite of the role is absent (EC EX-4 rule 3; includes `blocks:"start"` questions);
5. every complete-phase dependency of its units holds: `available` contract/facet as EC EX-4; `done` = the
   producer role has an applicable completion (R17-7); `accepted` = R18-6 derived acceptance
   (`review.acceptance(snapshot)`), which evaluates false until R18 lands (DR17-7); no unanswered `blocks:"complete"` question.

Completion-mode set: the reserve set without the guard, plus `reservation_open(reservation)`, plus
`fact_version(job, role) == v` for every role of the active job. Evidence-only set: `request_unused(check id)`,
`reservation_open(reservation)`, `run_active(run, G, secret)`. Native validates (step 5 of EC EX-3) that
every evidence fact equals the native `run_checks` result byte for byte, that `completion` appears only when
conditions 1 to 3 hold natively, and that publications and unit outputs equal the native observation; any mismatch
is `internal`.

Effects (one commit): `evidence` rows, `reservation_state` → `completed`, `check_recorded`, the check request row;
in completion mode additionally `completions`, `publications`, `run_state` → `completed` (N10), the target
transition `running → done`/`completed` (M37) at ordinal 1, then downstream transitions sorted by role (R17-7); in
both modes, `blocker_state` → `cleared` for each id in `blockers_cleared` (R18-5).

Retry: on `state_conflict` the handler calls `read_execution` and `checks.evaluate` again with the same native run
result, at most 3 times; after the third `state_conflict` it commits evidence only with `retry_exhausted` in
`withheld`. A `fenced` at any complete attempt (snapshot or lock) refuses `fenced` `{run_state}`, writes nothing and
leaves the reservation open (option (a)). Busy, disk-full or I/O at the complete commit refuse with their existing
codes and leave the reservation open; a same-ID retry then refuses `check_in_progress` until `check-recover`.

Early replay (refusal step 3 below): a request row for the ID replays as EC EX-2; otherwise a reservation with that
`request_id` and an equal body that is open → `check_in_progress`, unequal body → `request_conflict`.

## R17-7 Applicability, reconcile and prerequisite rederivation (`checks.lua`, `reconcile.lua`)

Pure Lua over the snapshot (no I/O). Definitions, per role lineage (EC EX-5):

- **Observed output** of a unit: the unit output in the `check_observation_v1` of the latest `check_recorded` event
  of the owning role's lineage whose observation lists the unit as an own unit; absent if none.
- **Current completion** of a role: its latest `completions` row (by `seq`) in the current lineage.
- **Published output** of a unit: the unit output recorded by the owning role's current completion. **Current
  publication** of a facet: the publication of that facet in the producer role's current completion; this is the
  "current" of EC P2-3 `start_facets`.
- A completion K of role X is **applicable** iff (i) `K.work_fp` equals X's current `work_exec_v1` digest, (ii) for
  every own unit, K's output equals the observed output, and (iii) for every producer target in the evidence vectors
  of K's reservation, the target output equals that producer unit's published output.
- Precedence rule 4 (done) holds iff some completion of X's lineage is applicable (latest first), which makes M40
  and M41 reachable.
- Exported Lua functions (`checks.lua`; R18 calls them): `current_completion(snapshot, role)` (current completion or
  nil), `applicable_completion(snapshot, role)` (latest applicable completion of the lineage or nil) and
  `unit_output(completion, work)` (the `xxh3_128:` unit output the completion recorded, or nil).
- R16 recovery `evidence` section, non-empty body: line `Check evidence (recorded, latest per work and check):`, then
  one line per item sorted by (work, check): `- {work} {check}: {outcome}` with ` ({reason})` appended unless
  `outcome` is `pass` (the R18-1 check-summary line form).

Reconcile after a completion (carried by `check_recorded`; reasons per the EC role table): consumers whose W moves
through `start_facets` → M23, M24, M25 or M40 (`work_changed` / `drift_cleared`); consumers whose prerequisite
becomes available or unavailable → M17 to M22; consumers whose applicability changes only through (iii) → M38 or M41
(`evidence_inapplicable`), never `work_changed`. Format3 load reconciliation (R15) uses the same functions over the
now-populated snapshot. Explain: `status --role` `role_detail.explain.output_vector` becomes the sorted array of
`{work,recorded,current}` for targets failing (ii) or (iii) of the role's current completion.

Snapshot extension (`read_execution` result, `execution_snapshot_v1` keys added; all decimal IDs as strings,
digests as `xxh3_128:` text; bounded by `execution_snapshot_bytes`):

- `publications`: `[{publication,completion,job,role,work,facet,semantic}]`, current publications only, sorted by
  (work, facet).
- `completions`: `[{id,job,role,generation,run,dispatch,work_fp,seq,units:[{work,output}],vectors:[{work,targets}]}]`,
  every completion of every current lineage of the active job, sorted by id.
- `observed`: `[{work,output,seq}]`, the observed output per unit, sorted by work.
- `evidence`: `[{id,reservation,work,check,outcome,reason,matched,seq,vector}]`, latest row per (work, check) in each
  current lineage, sorted by (work, check). R16 `read_recovery_records` returns the same item shape over the
  role's lineage as `recovery_records.evidence` (R16-5 `evidence`).
- `reservations`: `[{id,run,request_id,opened_seq}]`, open reservations only.

## R17-8 `check-recover` (R17-7 of the design)

Verb `check-recover` (handler `shipped.check_recover.v1`, response `check_v1`, capability `execution_commit_v1`).
Options: `--ledger!`, `--run-id!` (canonical decimal), `--reservation!` (canonical decimal), `--confirm-stopped!`
(required scalar, only value `true`, else `usage`; EC OP1), `--request-id!`, `--actor!`. Parameter map
`{confirm_stopped:"true",reservation,run_id}`. Services: `read_execution({})`; `recover_reservation({run_id,
reservation})` (native-only commit; Lua builds no change). It is exempt from EC EX-6 binding, so an upgrade cannot
block recovery (DR17-8).

Refusals after replay: reservation id unknown, or not of `--run-id` → `reference`, message
`Unknown check reservation`, field `reservation`; reservation not `open` → `state_conflict` (deterministic, a
recorded D149-style limit). The run may be `active` or in any terminal state; no active job is required.
Preconditions under the lock: `request_unused(check-recover id)`, `request_unused(reserved check id)`,
`reservation_open(reservation)`. Effects: `reservation_state` → `interrupted`, `check_interrupted`, the
check-recover request row, and the reserved check ID's request row (outcome `interrupted`, referencing its
`check_reserved`), both with the same saved `check_v1` result. No evidence, transitions, `run_state` change or
fact_version bump; an interrupted check never makes a role done. A later retry of the reserved check ID replays the
interruption; a fresh check uses a new ID.

## R17-9 CLI, responses and refusal order

`check` options: `--ledger!`, `--run!` (token), `--root!`, `--base-root!`, `--scratch!` (directories, opaque native
slots), `--request-id!`, `--actor!`, `--provenance` (file). Parameter map
`{base_root,root,run_id,scratch}` plus `provenance_path` when given; `run_id` is the token's run id
(decimal string; the R16 DR16-1 rule), so no secret enters a body record (DR17-9). Paths are normalized absolute private strings.

Root help lines, placed directly after the R16 lines and before the RA01 lines / `planar --help`:

```text
planar check --ledger PATH --run TOKEN --root DIRECTORY --base-root DIRECTORY --scratch DIRECTORY --request-id ID --actor TEXT [--provenance FILE]
planar check-recover --ledger PATH --run-id ID --reservation ID --confirm-stopped true --request-id ID --actor TEXT
```

Response `check_v1` (E8 A: exit 0 for every recorded outcome; keys canonical-sorted):
`{version:1,kind:"check",ledger,request_id,job,role,run:{id,generation},reservation,outcome,withheld,evidence,observation,completion}`.
`outcome` ∈ `completed`, `recorded`, `interrupted`. `evidence` is
`[{evidence,work,check,outcome,reason,matched,exit_status,duration_ms,output_bytes,output_truncated,output}]` sorted
by (work, check) (`matched`, `exit_status` decimal string or `none`; `output` the record id or `none`); `[]` for
`interrupted`. `observation` is `{id,digest,units:[{work,output}]}` or `none`. `completion` is
`{id,work_fp,publications:[{work,facet,semantic,publication}]}` or `none`. The token never appears. Saved result ≤
1 MiB; the plan is bounded to 1024 evidence rows (`check_evidence_rows`) so it fits.

Refusal order for `check`: (1) `usage`; (2) `unsupported_ledger_version`; (3) early replay / `check_in_progress` /
`request_conflict`; (4) `interpretation_mismatch`; (5) `no_active_job`; (6) token (a grammar violation is already
`usage` at step 1): naming no run, or a run whose role is not of the active job →
`state_conflict` (EC A-13 precedent, DR17-10), wrong generation ordinal, wrong secret or run not current →
`fenced` `{run_state}`; (7) the run's role is a reviewer → `reference` `Unknown selected role`, field
`selection.role` (reviewers complete through R18 `verdict`, N11, never N10), then role `running` with drift →
`drifted`; (8) open reservation on the run → `check_in_progress`; (9) provenance refusals
(EC P2-4); (10) reserved environment name → `reference`; (11) host-path refusals of `--root`, `--base-root`,
`--scratch` (existing explicit-directory messages) and scratch overlap → `path_denied`; (12) capture refusals;
(13) contract delivery → `revision_conflict`; (14) vector precheck → `observation_target_mismatch`; (15) reserve
preconditions (mapping in R17-6); (16) complete-commit `fenced` and storage refusals (R17-6).

`check-recover` order: (1) `usage`; (2) `unsupported_ledger_version`; (3) early replay; (4) reservation refusals
(R17-8); (5) commit preconditions → `state_conflict`.

Post-commit fallbacks: EC EX-7 `check` and `check-recover` rows, unchanged. `check` has two commits; a failure
after the reserve commit is not a post-commit fallback: it refuses with its code and the reservation stays open.

Status and history additions (R17 surface others read):

- `status_v1` `roles[].run` gains `reservation` (`none` or the open reservation id) (AM-4).
- `status --request-id ID` where ID names an open reservation and no request row: `request` =
  `{request_id,operation:"check",outcome:"in_progress",reservation,run:{id,generation}}` with no `result` (AM-4).
- `history --event` `execution` expansions: `check_reserved` → `{reservation:{id,request_id,run,dispatch}}`;
  `check_recorded` → `{reservation:{id,request_id,run},evidence:[…as check_v1 without output bytes…],observation:{id,value},completion?:{id,dispatch,work_fp,publications}}`;
  `check_interrupted` → `{reservation:{id,request_id,run}}`. Output bytes are never expanded (records are reachable
  by id).

Orchestrator (`orchestrator --ledger`): `check` and `check-recover` labels become `available`; `orchestrator_state`
legal actions add to the R16-7 lists: worker `running` gains `check` (the only path to `done`; "always run `check`
before treating a role as done"); `running` with drift → `refresh` before `check`; a role whose run has an open reservation and
no live check process → `check-recover`, then a fresh `check`. Check events carry R15-8 provenance through the
`check_reserved` `provenance` reference (EC P2-4 attachment point pinned here).

Catalog: two new rows (production and authoring builds): after R16's 14/20 the counts are 16 production and 22
authoring; with-profile family list 16; external command capacity 50 to 48;
names `check`, `check-recover`, handler IDs `shipped.check.v1`, `shipped.check_recover.v1` leave the external
namespace. New capability `check_run_v1` (process spawning) is admitted only for `shipped.check.v1`.

## R17-10 Bounds and messages

Bounds (new limit names use `resource_limit`, `details:{limit,maximum}`, message `Execution resource limit
exceeded`, all checked before allocation):

| Limit | Maximum | Site |
| --- | --- | --- |
| `check_observation_bytes` / `_files` / `_depth` | 1073741824 / 100000 / 64 | each capture (EC EX-8) |
| `check_result_bytes` | 65536 | result file (recorded as `malformed_result`, not a refusal) |
| `check_retained_bytes` | 67108864 | retained output across the invocation; later checks get `retain_bytes` = remaining allowance (truncation, not failure) |
| `facet_file_bytes` / `facet_bytes` | 1048576 / 4194304 | publication payload (EC EX-8); exceeding refuses before reserve via the precheck capture |
| `check_evidence_rows` | 1024 | plan size, before reserve |

New messages (draft amendment AM-5 of the EC closed set; code, exit 3):

| Code | Message | Field |
| --- | --- | --- |
| `reference` | `Registry environment uses a reserved name` | `registry` |
| `reference` | `Unknown check reservation` | `reservation` |
| `path_denied` | `Scratch directory overlaps a checked root` | `scratch` |
| `io` (exit 4) | `Cannot create check scratch directory` | `scratch` |

All other sites reuse EC messages byte-exact: `check_in_progress` `A check is already in progress for this role`;
`drifted`; `fenced`; `state_conflict`; `observation_target_mismatch` `Wrong output vector` (now with
`details:{work}`); `revision_conflict` `Ledger revision does not match the expected revision`.

## R17-11 Module placement and test obligations

| Lane | Files | Owns |
| --- | --- | --- |
| R17-B | `src/engine/process/{runner,resolve}.{cppm,cpp,t.cpp}`, CMake registration | R17-2 |
| R17-C | `src/engine/capture/tree_digest.*`; `src/engine/execution/check.*`; `src/engine/ledger/execution.*` (`reserve_check`, `commit_check`, `recover_reservation`, snapshot extension); `src/engine/ledger/recovery.*` (`recovery_records.evidence`); `src/engine/history/execution_codec.*` (four codecs, `check_v1`); `src/stdlib/planar/checks.lua`, `reconcile.lua` | R17-1, R17-3 to R17-8 |
| R17-D | `src/cmd/planar/handlers/check/`, `handlers/check-recover/`, `commands.lua` rows, runtime ABI rows and capability, help bytes, `status`/`history`/`orchestrator` additions | R17-9, R17-10 |

Native ledger signatures (R17-C), following `planar.engine.ledger.execution`:

```cpp
std::expected<check_reservation, execution_error> reserve_check(const std::filesystem::path&, const execution_snapshot&, const check_reserve_change&);
std::expected<execution_commit_result, execution_error> commit_check(const std::filesystem::path&, const execution_snapshot&, const check_complete_change&);
std::expected<execution_commit_result, execution_error> recover_reservation(const std::filesystem::path&, const check_recover_request&);
```

`execution_error_code` gains `drifted` (R16 raises `drifted` from Lua only); `check_in_progress` is the enumerator R16
adds for its release and refresh guard, shared here (one enumerator).
`execution_expectation::kind_t::reservation_open` carries the reservation id in `number`; the reserve guard is a
native lock-time check shared with R16 release (AR-2).

Test obligations (Catch2, colocated, per-test temporary roots; fixtures are `/bin/sh` scripts written by the test):

- R17-2: exit 0/1/signal; `timeout_ms` 200 with a TERM-ignoring child killed after an injected 100 ms grace;
  grandchild in the group killed; `setsid` escapee not killed (limit test); output flood truncation and
  `output_bytes` saturation; stdout/stderr interleaving order for sequential writes; exact environment (child dumps
  `env`); cwd is the descriptor; stdin is empty; no leaked descriptors (child lists `/dev/fd`); resolution: absolute,
  PATH search, relative entries skipped, missing PATH, non-executable file.
- R17-3: each reason of R17-1 from a fixture (A17), including `zero_selected`, malformed result variants and
  `failed > 0` with exit 0; reserved-name refusal; scratch overlap refusal; scratch cleanup.
- R17-4: digest-only capture equals render identities on the same tree; limits; contract delivery mismatch and
  inline-contract refusal; vector precheck mismatch names the producer (A16).
- R17-5: added, deleted, modified, mode, rename, frozen-path and producer-path cases (A18); untracked and ignored
  files counted unless excluded.
- R17-6: pass completes (M37, N10, publications); each `withheld` reason records evidence without completion;
  the role stays `running` after a failed check; a file edited during the check gives `inconclusive` everywhere
  (A15), change-and-revert not detected (limit); a concurrent load between snapshot and complete triggers a
  re-evaluation; three forced `state_conflict`s give `retry_exhausted` (kills the "retry forever" mutant); a
  takeover during the run gives `fenced` with the reservation still open (A10, option (a)).
- R17-7: A12 (blocker cleared, done restored without rerun, M21/M41); A16 (producer re-completes with new output,
  consumer W unchanged, consumer `evidence_inapplicable`); A28 selective staleness (mtime touch: none; excluded
  path: none; rename inside one unit: only that unit); M40 W-return.
- R17-8: crash after reserve (reservation open; retry `check_in_progress`; `check-recover`; retry replays
  `interrupted`; fresh ID completes); unknown and closed reservation refusals.
- R17-9: `check_v1` replay bytes equal; `status` reservation fields; `history --event` expansions; help bytes.

## R17-12 Amendments

- **AM-1** (EC EX-3 "No other precondition exists"): the reserve commit adds the native lock-time guard "no open
  reservation for the run" (`check_in_progress`), the same guard R16 AR-2 adds to release; it is not an `expected`
  member and the vocabulary stays eight.
- **AM-2** (`command-contract.md:190-194`, as the design requires): family `check` may call `read_execution` and
  `commit_check` up to four times each, alternating, for the bounded retry; every other service at most once.
- **AM-3** (R16 draft): release already refuses `check_in_progress` (R16 AR-2); R17 adds the same lock-time guard
  to `refresh` (refusal step 7, after `drifted`), so W never moves under an open reservation; `checkpoint` keeps none.
- **AM-4** (EC P2-1 `status_v1`): the `reservation` field and the `in_progress` request lookup of R17-9.
- **AM-5** (EC EX-7 closed message set, XD4 for execution only): the four messages of R17-10, and `details:{work}`
  on `observation_target_mismatch` at the check site.
- **AM-6** (`declarations.md:338-339`, E7): registry environment replacement is extended by Planar's reserved
  `PLANAR_*` variables; registries may not define names with that prefix.

## R17-13 Draft decisions

| ID | Decision | Basis |
| --- | --- | --- |
| DR17-1 | Fenced completion: option (a), refuse `fenced`, write nothing, reservation open until `check-recover` | v5 `:59-60`, `:370`; no amendment needed |
| DR17-2 | `exit_status` = exit code, negated signal, or NULL | EC C5 permits any integer |
| DR17-3 | Gate rows retain the `boundary_diff_v1` only on failure | E15 A retains the diff set; bound storage |
| DR17-4 | Reserve the whole `PLANAR_` prefix | simpler than a name list; E7 |
| DR17-5 | Capture, delivery and precheck run before reserve, so their refusals leave no reservation | v5 completion step 2 order |
| DR17-6 | Contract delivery path is the contract's workspace file path in the root; inline contracts cannot be delivered; mismatch reuses `revision_conflict` | v5 `:497` "revision-mismatch refusal" |
| DR17-7 | `accepted` complete-phase quality evaluates false until R18 | R18-6 owns acceptance |
| DR17-8 | `check-recover` is native-only and exempt from EX-6 | E3 A intent: upgrades never block recovery |
| DR17-9 | Check request body stores the run id, never the token | R16 DR16-1; EC run-token exposure rules |
| DR17-10 | Token grammar violation → `usage`; unknown run → `state_conflict`; else `fenced` | R16 draft refusal order step 6; EC A-13 |
| DR17-11 | Observed output is recorded only for the checking role's own units; a `done` role cannot be claimed (R15-4), so A05 for a `done` role is detected at a consumer's vector precheck and at the role's next run; an out-of-run re-observation fact is an open item for the fixing rounds | R15-4; A05 |
| DR17-12 | One gate row per unit, `check` = `planar.boundary_v1` | evidence columns are per (work, check) |
| DR17-13 | Output-cap overflow kills the group like a timeout | bounded drain |
