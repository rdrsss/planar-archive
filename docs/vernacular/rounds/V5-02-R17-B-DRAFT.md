# V5-02-R17-B (DRAFT): native process runner

Status: draft-mode plan (operator decision 2026-11-08). Feature- and code-complete draft; compile and run the tests
this round adds. No full gate, no rewrite-check gate, no Linux/Docker run, no mutation probes, no review loop, no
evidence directory. Those belong to the fixing rounds (see "Left for fixing rounds").

Lane: worktree `/Users/mn/projects/github/rdrsss/planar-lane-b`, branch `draft/lane-b`, base `1338637c` (R15-B draft
base with the C01 R1 fix). Disjoint from lane A: this round touches only the new `src/engine/process/` directory, one
block appended to `src/engine/CMakeLists.txt`, one row in `docs/architecture.md` and this note.

## Authority and inputs

- Binding interface: `execution-contract-r17.md` R17-2 (runner API and rules 1 to 8) and the R17-2 row of R17-11 (test
  obligations), unreviewed draft contract. It was committed in `c7efd3ba`, which is **not** in this lane's base; coders
  read it from the main checkout, read-only:
  `/Users/mn/projects/github/rdrsss/planar/docs/vernacular/execution-contract-r17.md`. It is not copied into the lane.
- `execution-design.md` R17 sections 2 to 6 and the module-placement table (`src/engine/process/`: "posix_spawn,
  process group, cwd descriptor, replaced environment, timeout, output caps, macOS and Linux branches"; no Lua, no
  ledger). Q15 A (no OS sandbox, escaping descendants are a recorded limit) from `remaining-steps-design.md`.
- R15-B records (`V5-02-R15-B.md` L1 to L23, `V5-02-R15-B-review.md`): no limit or follow-up is assigned to R17-B;
  the R15-C/D/E items stay with those rounds. Nothing here depends on the format3 ledger.

## Scope

R17-2 only: `resolve_executable` and `run` with the exact types of the contract, macOS and Linux code paths (Linux
compiled conditionally, not run), and the R17-2 test obligations. Out of scope: R17-3 check orchestration
(`spawn_spec` construction, reserved `PLANAR_*` variables, scratch and result file, `classify`), capture, ledger,
Lua, CLI (lanes R17-C and R17-D).

## Module placement

| Module | Files | CMake target | Depends on |
| --- | --- | --- | --- |
| `planar.engine.process.resolve` | `src/engine/process/resolve.{cppm,cpp,t.cpp}` | `planar_process_resolve` | `std` + POSIX C API only |
| `planar.engine.process.runner` | `src/engine/process/runner.{cppm,cpp,t.cpp}` | `planar_process_runner` | `std` + POSIX C API only |

Both register through `planar_rewrite_module(... TESTS ...)` in `src/engine/CMakeLists.txt` (test executables stay
outside `all`; `planar_tests` builds them). Runner tests take `TEST_TIMEOUT 60`. The two modules do not import each
other: `run` takes an already-absolute `executable` (DB-2); R17-C composes them. No `planar.engine.value` edge either
(the contract says `std` only; the design table's "value only" is the looser bound).

Both live in `namespace planar::engine::process`. Every `.cppm` opens with `/// @file` and `/// @brief` (entry points,
error boundary, thread confinement); every exported declaration gets a Doxygen summary. Tests use plain `//`
headers, Catch2 names starting `R17-B resolve ` / `R17-B runner ` with tag `[process]`, and per-test temporary roots
(fixtures are `/bin/sh` scripts written by the test into its root, per R17-11; the design's Python-fixture line is
superseded by the later, more specific contract). Focused run:
`ctest --test-dir build/rewrite-debug -R 'replacement\.R17-B'` with a nonzero matched count.

## Packages (sequential)

### P1: resolve module and CMake registration

Files: `src/engine/process/resolve.cppm`, `resolve.cpp`, `resolve.t.cpp`; `src/engine/CMakeLists.txt` (append the
`planar_process_resolve` block and, in the same edit, a `planar_process_runner` block guarded so it can be enabled in
P2, or simply added in P2; coder's choice, recorded in the commit).

Implements rule 1 exactly: `command0` with `/` must be absolute, regular (`stat`, following symlinks; DB-5) and pass
`faccessat(AT_FDCWD, path, X_OK, AT_EACCESS)`; otherwise `missing`. Without `/`: take `PATH` from `environment` only
(first entry named exactly `PATH`; the vector is sorted and unique by contract), split on `:`, skip empty and
non-absolute entries, join `dir + "/" + command0`, first regular executable wins. Empty `command0`, or no `PATH`
variable → `missing`. Never consults the host environment, never calls `getenv`.

Tests: absolute hit; absolute non-executable regular file → missing; absolute directory → missing; relative-with-slash
(`./x`, `bin/x`) → missing; PATH search picks the first matching entry; empty and relative PATH entries skipped (a
relative entry that would match relative to cwd is not taken); non-executable earlier entry skipped for a later
executable one; missing `PATH` → missing; host `PATH` ignored (environment without `PATH` while the host has one);
empty `command0` → missing.

### P2: runner core (spawn, environment, cwd, stdin, merged output, exit and signal status)

Files: `src/engine/process/runner.cppm`, `runner.cpp`, `runner.t.cpp`; `src/engine/CMakeLists.txt` (runner block).

Exports exactly the R17-2 types (`process_error_code`, `process_error`, `spawn_spec`, `termination`, `run_result`,
`run`). Implements rules 2, 3, 7 and 8 and spec validation (DB-1): `pipe` with `O_CLOEXEC` (`pipe2` on Linux;
`pipe` + `fcntl(F_SETFD, FD_CLOEXEC)` on macOS, done before any spawn, DB-7), `/dev/null` opened
`O_RDONLY|O_CLOEXEC`, `posix_spawnattr` with `POSIX_SPAWN_SETPGROUP` (pgid 0), `POSIX_SPAWN_SETSIGMASK` (empty),
`POSIX_SPAWN_SETSIGDEF` (`sigfillset` minus `SIGKILL`, `SIGSTOP`), macOS `POSIX_SPAWN_CLOEXEC_DEFAULT`; file actions
`addfchdir_np(cwd_fd)`, `adddup2(devnull, 0)`, `adddup2(write_end, 1)`, `adddup2(write_end, 2)`. `posix_spawn` (never
`posix_spawnp`) with argv and a `NAME=VALUE` envp built from `spec.environment` only. Spawn errno `ENOENT`, `EACCES`,
`ENOEXEC`, `ENOTDIR` → `termination::not_spawned` with `spawn_errno`; any other → `spawn_io`. Parent closes the write
end immediately, reads with `poll` + `read` (64 KiB buffer, `EINTR` retried), retains the first `retain_bytes`, then
`waitpid` the leader; `WIFEXITED` → `exited`/`exit_code`, `WIFSIGNALED` → `signaled`/`signal`. `duration_ms` is
`steady_clock` from just before `posix_spawn` to leader reap. After reap: `kill(-pgid, SIGKILL)` and drain to EOF or
`kill_grace_ms` (rule 6, needed here so a grandchild holding the pipe cannot hang P2's loop). Every descriptor is an
RAII owner closed on every path. No signal disposition change in Planar (DB-8). Platform code under
`#if defined(__APPLE__)` / `#elif defined(__linux__)` / `#else #error`.

Tests: exit 0 with output; exit 1; child kills itself with `SIGTERM` → `signaled`, `signal == SIGTERM`; stdout/stderr
interleaving for sequential writes (`printf a; printf b >&2; printf c`) gives `abc`; exact environment (child runs
`/usr/bin/env`, output equals the sorted spec pairs, nothing from the host, e.g. no `HOME` when not given); cwd is the
descriptor (open a temp dir, run `pwd -P`, compare with its realpath, while the test process cwd is elsewhere);
stdin is empty (`cat` exits 0 with no output, no hang); no leaked descriptors (open a test descriptor with
`O_CLOEXEC`, and on macOS also one without it; the child probes fds 3 to 63 with `(: <&N) 2>/dev/null && echo N`
in `/bin/sh` and reports none); retained prefix and `truncated` when `retain_bytes < output_bytes`; `retain_bytes == 0`
retains nothing but counts bytes; non-executable executable → `not_spawned` with `EACCES`; nonexistent executable →
`not_spawned` with `ENOENT`; invalid spec rejections of DB-1.

### P3: timeout, output cap, group kill, drain and the reference row

Files: `src/engine/process/runner.cpp`, `runner.t.cpp`; `docs/architecture.md` (one module row plus the Q15 A
sentence of rule 7, verbatim: "Checks run trusted registry commands with bounded time, output, cwd and environment
and no OS sandbox; escaping descendants are not tracked.").

Implements rules 4, 5 and 6 fully: deadline = spawn time + `timeout_ms`; the poll loop waits in slices of at most
10 ms and checks `waitpid(pid, WNOHANG)` each slice (DB-3). At the deadline `kill(-pgid, SIGTERM)`, then
`SIGKILL` to the group if the leader is not reaped within `kill_grace_ms`. When read bytes exceed
`max_output_bytes`: stop retaining, saturate `output_bytes` at `max_output_bytes + 1`, kill the group the same way
(DR17-13), result `output_limit`, and keep reading-and-discarding so the child is never blocked on a full pipe before
the kill lands. The first terminating event wins (DB-4). After leader reap, group `SIGKILL` and bounded drain.

Tests (all with `kill_grace_ms = 100`, small `timeout_ms`, no sleep longer than 5 s): `timeout_ms` 200 with a child
that traps and ignores `TERM` → `timed_out`, `signal == SIGKILL`, `duration_ms` within [200, 200 + 100 + slack]; a
plain `sleep 30` with `timeout_ms` 200 → `timed_out`, `signal == SIGTERM`; grandchild in the group killed (the leader
backgrounds `sleep 30` writing its pid to the temp root and exits; after `run` returns, `kill(pid, 0)` reports `ESRCH`
after a bounded re-check for reaping by init); `setsid` escapee not killed (limit test: the escapee holds the pipe
open; `run` returns within about `kill_grace_ms` of leader exit and the escapee is still alive; the test kills it in
cleanup); output flood (`yes`-style loop) with `max_output_bytes` 4096 → `output_limit`, `output_bytes == 4097`,
retained bytes equal the first `retain_bytes` of the stream, `truncated`; output exactly `max_output_bytes` → `exited`,
not truncated when `retain_bytes == max_output_bytes`.

## Draft decisions

| ID | Decision | Basis |
| --- | --- | --- |
| DB-1 | `run` validates the spec before any descriptor is opened. Bound violations (`timeout_ms` outside 1..3600000, `max_output_bytes` outside 1..16777216, `retain_bytes > max_output_bytes`, `kill_grace_ms > 60000`) → `resource_limit`, `sys_errno` 0. Structural violations (`executable` not absolute, empty `argv`, `cwd_fd < 0`, environment not strictly sorted by name or a name empty or containing `=`, any string containing NUL) → `spawn_io` with `sys_errno = EINVAL`. | Contract gives ranges and the error enum but no mapping; `resource_limit` is the only bound-shaped code |
| DB-2 | `run` does not call `resolve_executable`; the modules are independent targets and R17-C composes them. | Contract: `executable` is "absolute path from resolve_executable" |
| DB-3 | Leader liveness by `waitpid(WNOHANG)` between `poll` slices of at most 10 ms on both platforms; no `SIGCHLD` handler, `kqueue` or `pidfd` (no global mutable state, one code path). | CLAUDE.md "avoid global mutable state"; simplest portable form |
| DB-4 | If a timeout or output-cap kill is sent and the leader then exits normally (a `TERM` handler that exits), `how` stays `timed_out`/`output_limit`, `signal` is 0 and `exit_code` holds the exit code. The first of deadline and cap overflow decides `how`. | Rule 4/5 fix the result; R17-3 `classify` ranks timeout and output limit above exit status |
| DB-5 | Resolution follows symlinks (`stat`, `faccessat` without `AT_SYMLINK_NOFOLLOW`) and returns the joined path, not a `realpath`. | Contract says "regular file with X_OK"; toolchains are commonly symlinks |
| DB-6 | `not_spawned` results carry empty output, `output_bytes` 0 and `duration_ms` 0. | Nothing ran |
| DB-7 | On macOS the pipe is `pipe` then `FD_CLOEXEC` per end; the race with a concurrent spawn elsewhere is covered by `POSIX_SPAWN_CLOEXEC_DEFAULT`. Linux uses `pipe2(O_CLOEXEC)`. | Rule 2 |
| DB-8 | Rule 8 is satisfied structurally: Planar never writes to the child, so `run` changes no signal disposition (a process-wide `SIGPIPE` change would be global state). | Rule 8 text, CLAUDE.md |
| DB-9 | Kills use `kill(-pgid, …)` with `pgid == pid`; `ESRCH` from a group kill is not an error; any other `kill` or `waitpid` failure → `wait_io` after a best-effort group `SIGKILL` and reap. A `poll`/`read` failure other than `EINTR` → `pipe_io` with the same cleanup. | Rule 6; no zombie or orphan left on error paths |
| DB-10 | Test fixtures are `/bin/sh` scripts written by the test (R17-11), not Python under `tests/native/` (design EX-10). | Later, more specific contract text |
| DB-11 | Past the rule 6 drain deadline the runner still takes bytes already buffered in the pipe (zero-timeout reads, at most 64 reads of 64 KiB) and stops at the first idle read, so `kill_grace_ms == 0` does not lose output the leader wrote before exiting while an escapee flooding the pipe cannot extend the drain without bound. | Rule 6 bound plus "first retained bytes" of the output |
| DB-12 | On macOS the contract's `posix_spawn_file_actions_addfchdir_np` is called under a local `-Wdeprecated-declarations` suppression: the macOS 26 SDK deprecates it in favour of `posix_spawn_file_actions_addfchdir`, which older supported releases lack. | Contract names the `_np` form; `-Werror` build |
| DB-13 | The descriptor-leak probe is `[ -e /dev/fd/N ] && echo N` in `/bin/sh`, not `(: <&N)`: macOS `/bin/sh` is bash, which reports its own saved descriptor 10 for the redirection form. A positive-control test opens fd 9 in the shell and expects the probe to report it. | P2 test obligation; observed on macOS 26 |
| DB-14 | P2 counts `output_bytes` saturating at `max_output_bytes + 1` and retains `retain_bytes`, but sends no timeout or output-cap kill (rules 4 and 5 are P3). The main loop already runs in 10 ms poll slices with `waitpid(WNOHANG)` so P3 adds the deadline and cap checks in place. | Package split |
| DB-15 | If no deadline came first and the bytes read exceed `max_output_bytes` only during the rule 6 drain (the leader wrote past the cap and was reaped before the main loop read it), the result is still `output_limit`, with the leader's own exit status per DB-4. | Rule 5 is defined on bytes read, not on read timing; keeps the outcome independent of scheduling |
| DB-16 | A group `kill` that fails with `EPERM` is ignored like `ESRCH`. macOS returns `EPERM` for a group whose only members are zombies (a leader that exited but is not yet reaped), observed in P3 tests as a spurious `wait_io`. | Rule 6 best-effort group kill; DB-9 |
| DB-17 | A `PATH` entry that already ends in `/` is joined without a second slash (`/a/` + `tool` gives `/a/tool`), not the literal `dir + "/" + command0` (`/a//tool`). Both name the same file; the returned spelling differs and R17-C records it in `check_definition_v1` `command`, so a fixing round must confirm or revert to the literal join. | Cleaner recorded spelling; contract silent on trailing separators |
| DB-18 | A `command0` containing a NUL byte, or any `PATH` candidate containing one, resolves to `missing`. | Contract silent; a NUL cannot name a file through the POSIX API |

Delegated: none of these needed the operator; all are taken under the draft-mode instruction to choose the option most
consistent with the contract and record it.


## Results (draft finisher, 2026-11-08)

Implemented: P1 (`planar.engine.process.resolve`, R17-2 rule 1), P2 (`planar.engine.process.runner` core: spec
validation, `posix_spawn` with process group, exact environment, descriptor cwd, `/dev/null` stdin, merged output,
exit/signal status) and P3 (timeout with group `SIGTERM` then `SIGKILL` after the grace, output cap with
`output_limit`, post-reap group `SIGKILL` and bounded drain), the two `planar_rewrite_module` blocks in
`src/engine/CMakeLists.txt` and the `docs/architecture.md` row carrying the rule 7 / Q15 A sentence verbatim.

Commands (macOS, lane worktree, Debug only):

- `cmake --preset debug` then `cmake --build build/rewrite-debug --target planar_tests`: exit 0.
- `ctest --test-dir build/rewrite-debug -R 'R17-B'`: 43 matched, 43 passed, 0 failed, 0 skipped
  (15 `R17-B resolve`, 28 `R17-B runner`). These are the only tests of the touched modules; both modules are new.

Failing tests: none.

Gaps (in addition to "Left for fixing rounds"):

- The resolve tests change the process cwd and host `PATH` (`setenv`) inside individual cases; safe under ctest (one
  process per case) but they mutate process state when the whole test binary runs directly.
- Non-executable cases were not exercised as root (`faccessat` `X_OK` still requires an execute bit, so they should
  hold).
- The `__linux__` branch compiles only conditionally and was neither compiled nor run on Linux.
- Timing assertions (timeout, grace, drain) passed on an unloaded run; their slack is not yet validated under load.

## Left for fixing rounds

- Full gate (`make test-all`), rewrite-check gate, Doxygen/format gates, no-change build check, Linux Docker run of the
  `__linux__` branch (glibc ≥ 2.29 `addfchdir_np`, `pipe2`, signal delivery to groups).
- Mutation probes over rules 1 to 8 (permissive mutants: skip group kill, inherit host environment, skip
  `SETSIGDEF`, ignore `retain_bytes`, unbounded drain).
- Time-based flakiness under machine load (timing assertions use generous slack in draft; the fixing round tightens
  or restructures them), stress of zombie/orphan cleanup on every error path, `EINTR` injection.
- Race and fault coverage (Q3 A variant binaries, `SIGKILL` of `planar` while a child runs) belongs to R17-C/D and the
  integrated gate, not here.
- Review of the draft contract itself (R17-2 is unreviewed); any amendment lands in the fixing rounds.
