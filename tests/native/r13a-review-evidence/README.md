# R13-A independent review evidence

Fresh independent review in the shared checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Only this directory and
`docs/vernacular/rounds/R13-A-review.md` were authored. Existing candidate,
protected product files and coder evidence were preserved. No Planar operational
tools, installed binaries, operator databases, commits, installs or publication.
The reviewer exclusively held `build/rewrite-debug` for direct serial operations.

`audit.py` verifies all twelve current candidate files against the frozen
repository-relative manifest and snapshot, all forty folder-relative coder
evidence entries, six baseline copies, twenty protected files and byte-identical
canonical encoder implementation/function. Before/after results are retained.

Independent commands, all successful unless specifically labeled below:

- `cmake --preset debug`: `configure.log`.
- `cmake --build build/rewrite-debug --target all planar_tests`: `build.log`.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(canonical|hash|record)' --output-on-failure`: `focused.log`, 19 passes and zero skips.
- `python3 tests/vectors/records/generate.py --check`: `record-vectors.log`, ten vectors.
- `python3 tests/vectors/generate.py --check`: `render-vectors.log`, 12 hand, 20 upstream and 41 domain/canonical vectors.
- `python3 tests/native/r13a-review-evidence/build-probe.py` and `./tests/native/r13a-review-evidence/adversarial`: `probe-build.log`, `probe.log`; 9,813 assertions, 2,000 generated trees, 5,000 malformed byte inputs, zero skips.
- `python3 tests/native/r13a-review-evidence/build-probe.py allocations` and `./tests/native/r13a-review-evidence/allocations`: `allocations-build.log`, `allocations.log`; nine allocation probes, zero skips.
- `python3 tests/native/r13a-review-evidence/final-checks.py`: raw `cli.log` (one passing acceptance case, existing 247 invocations), `lint.log` (40 files), `no-work.log` (exact no-work output), and command/exit summary `final-checks.log`.
- `python3 tests/native/r13a-review-evidence/initial-test-check.py`: `initial-test-build.log`, `initial-test.log`. This intentionally links the preserved original test source against the unchanged final candidate objects. Exactly the original two failures reproduce (7 cases, 5 pass, 2 fail; 800 assertions, 798 pass, 2 fail), demonstrating that the offset corrections did not require relaxed product behavior. Catch2's configured failure exit is 42; the wrapper succeeds only for that expected result and both original source locations.

Probe compilation uses the candidate's `compile_commands.json`, module maps,
BMIs and static product libraries; it does not generate substitutes or change
product/CMake source. Sources and compile/link commands are retained; derived
review-only binaries/objects were removed after execution. Native probes are
host-specific to the executed macOS Apple Silicon debug toolchain.

The allocation probe measures ordinary C++ allocation requests, not total
process memory or C allocations. Compact malformed length/count headers request
at most 112 bytes per allocation / 176 total for canonical decoding and zero for
record decoding. Exact 16 MiB file decoding requests 16,777,232 bytes; exact
16 MiB value decoding requests at most 16,777,232 bytes / 33,554,560 total; the
64 MiB request uses 67,108,800 total bytes. These observations agree with bounded
owned copies and the temporary canonical tree, without count-based reserve.

Reviewer harness corrections are preserved separately. `allocations-initial.cpp`
and `allocations-initial.log` show a threshold assumption of payload size plus
one byte; libc++ rounds the allocation to payload plus sixteen, so the final
probe permits 64 bytes of allocator rounding and retains the same payload-copy
and aggregate constraints. This was a probe expectation error, not a product
failure. `initial-test-build-first.log` retains the initial wrapper failure to
decode Catch2's arbitrary binary diagnostic as UTF-8; the final wrapper preserves
raw bytes. `initial-test-build-second.log` retains its corrected-output run with
a wrong expected Catch2 exit (2 instead of configured 42). Both runs reproduced
the intended two original failures; the final wrapper checks the actual exit,
case result and exact failing source locations.

Full integrated debug/release gates, Linux execution, contextual history/tree
validation, interning/collisions/rollback and provenance remain outside this
R13-A review. The 182-case registry was not run in full. The evidence manifest
covers every retained file here except itself; the review report is hashed
separately by the coordinator.
