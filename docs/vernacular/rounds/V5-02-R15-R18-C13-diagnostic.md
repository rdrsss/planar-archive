# C13 exact Linux descriptor provenance

**Conditional fixture-repair precondition satisfied.** The unchanged runner
reproduces both failures through CTest because descriptor 3 is inherited from
CTest's temporary log file. Clean direct invocations pass. This is diagnostic
evidence, not a completed correctness gate or source approval.

The available ordinary agent substituted for the unavailable prescribed model.
Only this report, ignored diagnostic evidence and an external scratch recipe
were written. No candidate product, test, CMake, historical golden, installed
binary or operator database was changed or used. No builder prune was run.

## Pinned reproduction and identity

The scratch Dockerfile preserves the original recipe's exact Ubuntu digest,
LLVM apt pin `1:23.1.6~++20261108033755+c47d7823685a-1~exp1~20261108153854.96`,
CMake 4.4.5 SHA256, source-copy boundary, non-root user, toolchain footer and
evidence export. It replaces only the combined full-gate command with focused
configuration, a `planar_process_runner_tests` build at `-j3`, and a Python
diagnostic driver. The exact Dockerfile-specific ignore file was copied beside
the external recipe. Toolchain layers were reused from valid cache; no tool
substitution or retired-pin fallback occurred.

Docker export, configuration, focused build and diagnostic driver all exited 0.
The final build check exited 0 and reported `ninja: no work to do.` All 272
host source hashes match before/after and both copied-image source manifests.
Every invocation below uses the same binary, SHA256
`f0bf04531033738df1813d236719afd5e0572f73d5f5415dfba1b1e54873b940`;
the driver's final binary hash is unchanged.
Comparing copied context with the original full Linux gate finds only three
updated and three added documentation files. All 149 build-configuration files
and all source/vendor/test inputs are identical; the exact context comparison
is retained in `context-comparison.json`.

Executed core commands, with the existing binary and Debug build directory:

```sh
cmake --preset debug
cmake --build build/rewrite-debug --target planar_process_runner_tests -j3
ctest --test-dir build/rewrite-debug --no-tests=error --verbose -R '^replacement\.R17-B runner (leaks no descriptors to the child|probe detects an inherited descriptor)$'
ctest --test-dir build/rewrite-debug --no-tests=error --verbose -R '^replacement\.R17-B runner timeout of a plain sleep ends in SIGTERM$'
cmake --build build/rewrite-debug --target planar_process_runner_tests -j3
```

The Python driver also invokes both exact Catch2 filters with `close_fds=True`,
then repeats each using an explicitly inheritable `/dev/null` marker passed
through descriptor 3 or 23. Per-command argument vectors, inheritance settings,
logs and exits are retained. This focused target does not build or claim the
complete 1,844-test gate.

## Actual results

| Invocation | Matched cases / assertions | Result |
| --- | --- | --- |
| Clean direct negative probe | 1 / 7 | Exit 0; all pass. |
| Clean direct fd9 positive control | 1 / 3 | Exit 0; all pass. |
| CTest original two cases | 2 / 10 | CTest exit 8; each case has one assertion failure. Outputs are `3\n` and `3\n9\n`. |
| Direct marker 3, negative probe | 1 / 7 | Native exit 42; one expected assertion failure reports descriptor 3. |
| Direct marker 3, positive control | 1 / 3 | Native exit 42; one expected assertion failure reports descriptors 3 and 9. |
| Direct marker 23, negative probe | 1 / 7 | Native exit 42; one expected assertion failure reports descriptor 23. |
| Direct marker 23, positive control | 1 / 3 | Exit 0; its existing probe enumerates only 3 and 9, so this receives no marker-23 detection credit. |
| Passive-observer timeout case | 1 / 6 | CTest exit 0; all pass, first capture attempt complete. |

No selection was empty and no case was skipped. Native exit 42 is the actual
Catch2 assertion-failure result; the CTest wrapper returns 8. These deliberate
red diagnostics are preserved as failures, not counted as acceptance passes.

## External ownership proven through the process tree

During the existing 200 ms plain-sleep timeout case, passive procfs observation
captured this direct parent chain:

| Role | PID / parent PID | Recorded command |
| --- | --- | --- |
| CTest parent | 727 / 709 | `ctest` with the exact timeout-case filter. |
| Native test | 728 / 727 | `planar_process_runner_tests` with that exact Catch2 case. |
| Check child | 729 / 728 | `sleep 30`, spawned through the existing runner. |

All three hold descriptor 3 with the same target
`<container-source>/build/rewrite-debug/Testing/Temporary/LastTest.log.tmp959ed`,
inode 102684 and mount 143. Each fdinfo records flags `0400001`, with the
close-on-exec bit absent. The full raw target, command lines, fdinfo and other
descriptors are retained in `observer-timeout-1.proc.json` and the diagnostic
summary. This is a capture during the existing run, not claimed before-call
instrumentation or a capture from the earlier full-gate process.

The native process separately holds its borrowed test cwd on descriptor 4
(`02440000`, close-on-exec) and the runner's pipe read end on descriptor 6
(`02000000`, close-on-exec). Those original descriptor numbers are absent in
the check child. The child's standard outputs point to the same pipe as the
native read end, and its standard input points to `/dev/null`. The source opens
the original devnull descriptor with `O_CLOEXEC`, duplicates it to standard
input, then explicitly resets the original immediately after spawn; the live
parent snapshot correctly need not retain that original descriptor. This
source lifetime is distinguished from the observed external descriptor 3.

The same CTest-owned log target exists in the CTest parent before inheritance,
remains in the native test and then the check child, and is distinct from the
cwd, devnull and output pipe. Together with the clean direct runs, the exact
CTest reproduction and deliberate-marker controls, this establishes an ambient
harness descriptor rather than a Planar-owned CLOEXEC defect. R17-2 rule 2
promises close-on-exec for Planar-owned Linux descriptors; arbitrary external
descriptors retain their existing contract.

## Handoff

The conditional fixture-only branch may proceed under the approved C13 plan:
isolate pre-existing test-process descriptor flags before creating cwd and test
subjects, restore flags on all paths, retain the exact empty/fd9 assertions and
dynamic non-3 marker evidence, and preserve genuine owned-descriptor fault
sensitivity. No product close-all policy is justified by these observations.

The fresh coder/prober/reviewer still owe implementation correctness, the
specified Linux/macOS private faults, actual post-final-edit D15 measurements,
full host and final Linux gates and integrated acceptance. R19 remains separate.

Raw evidence and the reproducible driver are under
`tests/native/r15-r18-c13-evidence/diagnostic/`; `MANIFEST.json` binds all local
artifacts, source identities, recipe, command arguments, toolchain and this
report. The external scratch build remains available through Docker's build
cache for the following work. Lane 1 is released.
