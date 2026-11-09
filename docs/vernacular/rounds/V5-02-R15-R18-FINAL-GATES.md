# R15–R18 final platform gates

Status: all four complete current C14 host/Linux Debug/Release gates passed.
Publication and independent integrated acceptance remain pending. Earlier
C12/C13 failures and the qualified post-build artifact identity remain preserved.

Final C15 publication cleanup changes only three whitespace-only blank lines in
two Python test scripts. Exact old/new hashes, line preservation, AST attributes,
tokens/positions and whole/nested compiled-code identity are independently
verified; the affected two registered workflows pass in Debug and Release
(four executions, ten scenarios, zero skips). The C14 platform runs retain their
tested-original byte scope plus this [explicit C15 equivalence](V5-02-R15-R18-C15-code-review.md).
No new full-gate, Linux raw-byte equality or measurement execution is claimed for
C15; all other non-record candidate bytes retain their context identity.

The host toolchain changed during the review interruption. Both native build
directories were freshly configured against LLVM 23.1.5 before these gates.
Earlier stale-toolchain attempts and the gate-1 failures remain preserved as
excluded or superseded evidence; they are not counted as green runs.

| Configuration | Command | Result |
| --- | --- | --- |
| macOS Debug, C11 before C12 | `CMAKE_BUILD_PARALLEL_LEVEL=3 python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug` | Exit 0; 1,844/1,844 tests, zero skips; pinned formatting/Doxygen over 250 active files; repeat build did no work. |
| macOS Release, C11 before C12 | `CMAKE_BUILD_PARALLEL_LEVEL=3 python3 scripts/rewrite-check.py gate --build-dir build/rewrite-release` | Exit 0; 1,844/1,844 tests, zero skips; pinned formatting/Doxygen over 250 active files; repeat build did no work. |
| Linux aarch64 Debug, C12 | Configured `scripts/linux-gate.sh` run; `debug` preset | Configure/build/inventory/no-work exit 0; CTest exit 8, 1,842/1,844 passed, zero skips. Two descriptor-probe assertions failed. |
| Linux aarch64 Release, C12 | Same run; `release` preset | Configure/build/inventory/no-work exit 0; CTest exit 8, 1,842/1,844 passed, zero skips. Same two assertions failed. |
| macOS Release, C13 | Complete host gate helper | Exit 1 / CTest 8; 1,843/1,844 passed, zero skips. Static platform-guard inventory differs at the reviewed runner test. Lint/no-work stages were not reached. |
| macOS Debug, C13 | Host gate helper, interrupted by coordinator | 1,309 observed Passed rows; deliberately incomplete after the deterministic Release inventory finding. No full-gate credit. |
| macOS Debug, C14 current | Complete host gate helper with `CMAKE_BUILD_PARALLEL_LEVEL=3` | Exit 0; 1,844/1,844 passed, zero skips; formatting/Doxygen over 250 active files; no-work repeat. CTest 1,234.32 seconds. |
| macOS Release, C14 current | Same helper and parallelism | Exit 0; 1,844/1,844 passed, zero skips; formatting/Doxygen over 250 active files; no-work repeat. CTest 472.72 seconds. |
| Linux aarch64 Debug, C14 current | Official configured `scripts/linux-gate.sh` run | All configure/build/CTest/inventory/no-work exits 0; 1,844/1,844 passed, zero skips. CTest 1,044.19 seconds. |
| Linux aarch64 Release, C14 current | Same official run | All five exits 0; 1,844/1,844 passed, zero skips. CTest 450.76 seconds. |

Raw host evidence is under `tests/native/r15-r18-final-evidence/host-debug/`
and `host-release/`, using `gate-2.log` and its copied CTest records. Debug's
CTest wall time was 1,367.92 seconds; Release's was 479.19 seconds.

The earlier C11 full gates precede C12's sole Python test addition. The approved
[C12 plan review](V5-02-R15-R18-C12-plan-review.md) permits carrying unaffected
host coverage with exact unchanged source/build/binary/inventory identity and
a complete rerun of the affected registered entry in both configurations.
Earlier full gates are not represented as having executed later test bytes.
The final Linux gates must execute the final C12 source.

The final `replacement.execution_saved_response_c08_regressions` entry passed
in Debug (147.57 seconds) and Release (50.88 seconds), one selected test in each
configuration and zero skips. Each completed both embedded/profile workflows:
28 historical replays, 414 corruption scenarios and 1,242 reader assertions.
C12's sidecars bind unchanged 1,844-entry inventories, 1,628 build inputs per
configuration, 76 registered build binaries per configuration, and all 272
source files. The new test is the sole relevant input delta. The same archived
caller-refusal-suppression mutant passes the frozen previous complete matrix
and fails the new test; status and history expose it in both compositions.
Exact claim replay still refuses through its separate effective validation
path and receives no mutant-kill credit.

The C12 Linux raw outputs are retained under
`tests/native/r15-r18-final-evidence/linux-c12/`. Docker export exited 0,
both context comparisons were equal, and builder pruning exited 0. Debug CTest
took 1,162.90 seconds; Release took 482.87 seconds. The failing cases are
`R17-B runner leaks no descriptors to the child` (unexpected descriptor 3)
and `R17-B runner probe detects an inherited descriptor` (descriptors 3 and 9
where only 9 was expected). Both are assertion failures. C12's final registered
workflow passed in both configurations. See the
[independent request-changes disposition](V5-02-R15-R18-FINAL-ACCEPTANCE-C12.md)
and [C13 investigation](V5-02-R15-R18-C13.md).

C13's exact diagnosis, bounded source review, six-fault campaign and fresh
normal/raised measurements passed. Its complete Release gate then detected the
audited platform map's stale `runner.t.cpp` count (one versus the actual two
macOS guards). The additional guard is the required pre-spawn inheritable-subject
assertion. The in-flight Debug gate was stopped with SIGINT to its owned CTest
process; its reason and partial logs remain under
`tests/native/r15-r18-c13-evidence/host-debug/`. Both configurations' CTest
records were copied before any subsequent test invocation. See the
[C13 request-changes review](V5-02-R15-R18-FINAL-ACCEPTANCE-C13.md) and
[C14 scope](V5-02-R15-R18-C14.md).

Current C14 host logs, executed command vectors, exit statuses, exact Passed-row
accounting and copied CTest records are under
`tests/native/r15-r18-c14-evidence/host-debug/` and `host-release/`.
These complete gates execute the final C12 workflow, C13 fixture and C14 audit
correction together. Their builds regenerated 114 metadata-only artifact hashes;
the [explicit identity supplement](V5-02-R15-R18-C14-artifact-identity.md)
and independent audits qualify component carry without claiming raw equality.

The complete C14 Linux export is preserved under
`tests/native/r15-r18-final-evidence/linux-c14/`. Both actual test inventories
match the current host names, all ten exported step exits are zero, and both
repeat builds report no work. Host manifests before/after and the container
context manifest are identical. The runner and builder prune completed before
the source-context freeze was released. Independent Linux audit returned zero.
LLVM 23.1.6, CMake 4.4.5 and Linux aarch64 are recorded in the exported toolchain
and package evidence. No x86_64 Linux result is claimed.

The configured Linux runner builds `planar_tests` (which depends on the product
CLI), executes the complete CTest inventory, and repeats the build in each
configuration. Host checks provide pinned formatting and Doxygen coverage;
the Linux runner has no separate lint step. Every exported configure/build/
CTest/inventory/no-work exit and the actual inventory must be checked, because
the Docker export command itself preserves failed gate results without failing
the image layer. Coverage is Linux aarch64; x86_64 is not covered.

The real-project R19 exercise remains unselected. These platform and fixture
results do not substitute for that exercise. Installation, publication and merge
are not authorized.
